//
//  ndfa.cpp
//  Parse
//
//  Created by Andrew Hunter on 26/04/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <stack>

#include "ndfa.h"
#include "transition.h"
#include "remapped_symbol_map.h"

using namespace std;
using namespace dfa;

/// \brief Empty action list
const ndfa::accept_action_list ndfa::s_NoActions;

/// \brief A placeholder 'no state' representation
const state ndfa::s_NoState(-1);

/// \brief Copies an existing NDFA
ndfa::ndfa(const ndfa& copyFrom)
: m_States(new state_list())
, m_Symbols(new symbol_map(*copyFrom.m_Symbols))
, m_Accept(new accept_action_for_state())
, m_CurrentState(0)
, m_IsDeterministic(copyFrom.m_IsDeterministic) {
    // Copy the states
    for (state_list::const_iterator it = copyFrom.m_States->begin(); it != copyFrom.m_States->end(); it++) {
        m_States->push_back(new state(**it));
    }
    
    // Copy the accept actions
    for (accept_action_for_state::const_iterator it = copyFrom.m_Accept->begin(); it != copyFrom.m_Accept->end(); it++) {
        accept_action_list& action = (*m_Accept)[it->first];
        for (accept_action_list::const_iterator actionIt = it->second.begin(); actionIt != it->second.end(); actionIt++) {
            action.push_back((*actionIt)->clone());
        }
    }
}

// \brief Creates a new NDFA
ndfa::ndfa() 
: m_CurrentState(0)
, m_States(new state_list())
, m_Accept(new accept_action_for_state())
, m_Symbols(new symbol_map())
, m_IsDeterministic(false) {
    m_States->push_back(new state(0));
}

/// \brief Creates a new NDFA with the specified data
ndfa::ndfa(state_list* states, symbol_map* symbols, accept_action_for_state* accept)
: m_CurrentState(0)
, m_States(states)
, m_Symbols(symbols)
, m_Accept(accept)
, m_IsDeterministic(false) {
}

// \brief Destructor
ndfa::~ndfa() {
    // Destroy any accept actions that might be in this NDFA
    for (accept_action_for_state::iterator acceptState = m_Accept->begin(); acceptState != m_Accept->end(); acceptState++) {
        for (accept_action_list::iterator actionList = acceptState->second.begin(); actionList != acceptState->second.end(); actionList++) {
            delete *actionList;
        }
    }
    
    // Destroy any states
    for (state_list::iterator stateIt = m_States->begin(); stateIt != m_States->end(); stateIt++) {
        delete *stateIt;
    }
    
    // Destroy the symbol map
    delete m_Symbols;
    delete m_States;
    delete m_Accept;
}

/// \brief While building an NDFA: moves to the specified state
int ndfa::goto_state(int newState) {
    // Set the current state
    m_CurrentState = newState;
    
    // Set the current state to 0 if an out of range state is specified
    if (m_CurrentState < 0 || m_CurrentState >= m_States->size()) {
        m_CurrentState = 0;
    }
    
    // The current state is the return value
    return m_CurrentState;
}

/// \brief While building an NDFA: moves to the start state
int ndfa::goto_start() {
    m_CurrentState = 0;
    return m_CurrentState;
}

/// \brief Adds a new state (with no transitions), and returns the ID for that state
int ndfa::add_state() {
    int     newStateId  = (int)m_States->size();
    state*  newState    = new state(newStateId);
    
    m_States->push_back(newState);
    
    return newStateId;
}

/// \brief Adds a transition from the current to a new state, moves to the state, and returns the ID of the state
int ndfa::add_transition(const symbol_set& transition) {
    // Add a new state
    int newStateId = add_state();
    
    // Add a transition to this state
    add_transition(transition, newStateId);
    
    // Return the ID of the new state
    return newStateId;
}

/// \brief Adds a transition from the current state to the specified state, and moves to that state
void ndfa::add_transition(const symbol_set& symbols, int newState) {
    /// \brief Add the transition for this state
    add_transition(m_CurrentState, symbols, newState);
    
    // Update the current state
    m_CurrentState = newState;
}

/// \brief Adds a transition from the old state to the new state
void ndfa::add_transition(int oldState, const symbol_set& symbols, int newState) {
    // Nothing to do if the state ID is invalid
    if (oldState < 0 || oldState >= m_States->size()) return;
    if (newState < 0 || newState >= m_States->size()) return;
    
    // Get the ID for this symbol set
    int symbolId = m_Symbols->identifier_for_symbols(symbols);
    
    // Add as a transition to the current state
    (*m_States)[oldState]->add(transition(symbolId, newState));
}

/// \brief Marks the current state as an accepting state (for the specified symbol)
///
/// If eager is set, then the state is marked as an 'eager' accepting state - ie, the resulting DFA will not consider states 'following' this one
/// when it is evaluated.
void ndfa::accept(int symbol, bool eager) {
    accept(accept_action(symbol, eager));
}

/// \brief Marks the current state as an accepting state (for the specified symbol)
///
/// If eager is set, then the state is marked as an 'eager' accepting state - ie, the resulting DFA will not consider states 'following' this one
/// when it is evaluated.
void ndfa::accept(const accept_action& action) {
    accept(m_CurrentState, action);
}


/// \brief Marks the current state as an accepting state, returning the specified symbol
///
/// If eager is set, then the state is marked as an 'eager' accepting state - ie, the resulting DFA will not consider states 'following' this one
/// when it is evaluated.
void ndfa::accept(int state, const accept_action& action) {
    // Clone the action and store it
    (*m_Accept)[state].push_back(action.clone());
}

/// \brief Marks the specified state as non-accepting
void ndfa::clear_accept(int state) {
    // Remove all of the items from this state
    (*m_Accept)[state].clear();
}

/// \brief Returns the surrogate pair for a UTF-32 value
///
/// See section 3.9 of the Unicode standard.
static inline pair<int, int> surrogate_pair(int ucs32) {
    // The 'lower' range (second code point)
    int lower = 0xdc00 | (ucs32&0x3ff);

    // The 'higher' range (first code point)
    int higher1 = (ucs32 >> 10) & 0x3f;
    int higher2 = (ucs32 >> 16) - 1;
    int higher = 0xd800 | higher1 | (higher2<<6);

    // Return the pair for this character
    return pair<int, int>(higher, lower);
}

/// \brief Adds a transition for a range of surrogate symbols for the specified state
static void add_surrogate_transition(const range<int>& surrogateRange, int currentState, int targetState, ndfa* nfa) {
    // If the range is out of the range of valid surrogate characters then clip it
    if (surrogateRange.lower() >= 0x110000) return;
    if (surrogateRange.upper() > 0x110000) {
        add_surrogate_transition(range<int>(surrogateRange.lower(), 0x110000), currentState, targetState, nfa);
        return;
    }

    // Work out the range as surrogate pairs
    pair<int, int> surrogateLower   = surrogate_pair(surrogateRange.lower());
    pair<int, int> surrogateHigher  = surrogate_pair(surrogateRange.upper()-1);

    // Action depends on whether or not there are 1, 2 or more 'upper' characters
    if (surrogateLower.first == surrogateHigher.first) {
        // Transit to a state if we match the 'upper' code point
        int tmpState = nfa->add_state();
        nfa->add_transition(currentState, range<int>(surrogateLower.first, surrogateLower.first+1), tmpState);

        // Transit to the final state if we match any of the lower symbols
        nfa->add_transition(tmpState, range<int>(surrogateLower.second, surrogateHigher.second+1), targetState);
    } else {
        // Transit to a new state for the lower set of symbols
        int tmpState1 = nfa->add_state();
        nfa->add_transition(currentState, range<int>(surrogateLower.first, surrogateLower.first+1), tmpState1);

        // Transit to the final state for all the 'lower' symbols
        nfa->add_transition(tmpState1, range<int>(surrogateLower.second, 0xdc00), targetState);

        // ... do the same for the 'upper' set of symbols
        int tmpState2 = nfa->add_state();
        nfa->add_transition(currentState, range<int>(surrogateHigher.first, surrogateHigher.first+1), tmpState2);

        // Transit to the final state for all the 'lower' symbols
        nfa->add_transition(tmpState2, range<int>(0xd800, surrogateHigher.second+1), targetState);

        // If there's a middle range, then add transitions for that as well
        if (surrogateHigher.first-1 > surrogateLower.first) {
            // Transit for all of the remaining 'higher' symbols
            int tmpState3 = nfa->add_state();
            nfa->add_transition(currentState, range<int>(surrogateLower.first+1, surrogateHigher.first), tmpState3);

            // Accept for any 'lower' symbol
            nfa->add_transition(tmpState3, range<int>(0xdc00, 0xe000), targetState);
        }
    }
}

/// \brief Moves to a new state when the specified range of symbols are encountered
ndfa::builder& ndfa::builder::operator>>(const symbol_set& symbols) {
    // Get the state that should be moved to by this transition
    int nextState = m_NextState;
    m_NextState = -1;
    
    // Use a new state if no state has been explicitly set
    if (nextState == -1) {
        nextState = m_Ndfa->add_state();
    }

    // If generate surrogates is turned on, and the symbol set ends outside the surrogate range
    if (m_GenerateSurrogates) {
        // Search to see if there are any surrogate ranges
        symbol_set surrogates;
        symbol_set nonSurrogates;

        // Look through the symbol set
        for (symbol_set::iterator syms = symbols.begin(); syms != symbols.end(); syms++) {
            // Ignore empty ranges
            if (syms->lower() >= syms->upper()) continue;

            if (syms->upper() > 0x10000) {
                // Surrogate range
                if (syms->lower() <= 0xffff) {
                    // Split range
                    nonSurrogates   |= range<int>(syms->lower(), 0x10000);
                    surrogates      |= range<int>(0x10000, syms->upper());
                } else {
                    // Just a surrogate range
                    surrogates |= *syms;
                }
            } else {
                // Not a surrogate range
                nonSurrogates |= *syms;
            }
        }

        // See if there were any surrogate ranges
        if (!surrogates.empty()) {
            // Push before this surrogate
            push();

            if (!nonSurrogates.empty()) {
                // Just add a transition on the non-surrogate range
                m_Ndfa->add_transition(m_CurrentState, nonSurrogates, nextState);
            }

            // Add a surrogate transition for each surrogate range
            for (symbol_set::iterator syms = surrogates.begin(); syms != surrogates.end(); syms++) {
                add_surrogate_transition(*syms, m_CurrentState, nextState, m_Ndfa);
            }

            // Update the current state
            m_PreviousState = m_CurrentState;
            m_CurrentState  = nextState;

            // Pop afterwards
            pop();

            // Done
            return *this;
        }
    }
    
    // Add the transition for these symbols
    m_Ndfa->add_transition(m_CurrentState, symbols, nextState);
    m_PreviousState = m_CurrentState;
    m_CurrentState  = nextState;
    
    // Return the resulting object
    return *this;
}

/// \brief Pops the state on top of the stack and discards it
///
/// Returns false if the stack is empty.
/// The 'previous state' becomes the state on top of the stack.
/// If an 'or' is being constructed, this will move to the final state of the 'or' in the same way that rejoin does.
bool ndfa::builder::pop() { 
    if (!m_Stack.empty()) {
        // If there's a final state, then move there from the current state
        if (m_Stack.top().second >= 0) {
            int finalState = m_Stack.top().second;
            *this >> m_Ndfa->get_state(finalState) >> epsilon();
        }
        
        // Set the previous state to the 'initial' state on top of the stack
        if (m_Stack.top().first >= 0) {
            m_PreviousState = m_Stack.top().first;
        }
        
        // Pop the state
        m_Stack.pop();
        return true;
    } else {
        return false;
    }
}

/// \brief Begins or continues an 'or' expression
void ndfa::builder::begin_or() {
    // If the stack is empty then push a fake entry
    if (m_Stack.empty()) {
        m_Stack.push(stack_entry(0, -1));
    }
    
    // The initial state is the state on top of the stack
    int initialState = m_Stack.top().first;
    
    // Remember the previous state (in case we change it)
    int previousState = m_PreviousState;
    
    if (m_Stack.top().second < 0) {
        // Create a 'final state' for the various alternatives by adding an epsilon transition
        *this >> epsilon();
        
        // Set this on the stack
        m_Stack.top().second = m_CurrentState;
        
        // Preserve the previous state
        m_PreviousState = previousState;
    } else {
        // Create a transition from the current state to the final state
        m_Ndfa->add_transition(m_CurrentState, epsilon(), m_Stack.top().second);
    }
    
    // Move back to the 'initial state' (the point where the or expression starts)
    m_CurrentState = initialState;
}

/// \brief Moves to the state after the current 'or' expression, if there is one
void ndfa::builder::rejoin() {
    // Nothing to do if the top of the stack isn't building an 'or' expression
    if (m_Stack.empty() || m_Stack.top().second < 0) return;
    
    // Move to the final state
    int finalState = m_Stack.top().second;
    *this >> m_Ndfa->get_state(finalState) >> epsilon();
    
    // Unset the final state
    m_Stack.top().second = -1;
    
    // Pop the top stack entry if it was added by an or operation
    if (m_Stack.top().first == -1) {
        m_Stack.pop();
    }
}

/// \brief Creates a new NDFA that is equivalent to this one, except there will be no overlapping symbol sets
///
/// Note that if further transitions are added to the new NDFA, it may lose the unique symbol sets
ndfa* ndfa::to_ndfa_with_unique_symbols() const {
    // Remap the symbols so that there are no duplicates
    remapped_symbol_map* symbols = remapped_symbol_map::deduplicate(*m_Symbols);
    
    // Rebuild the transition table with the new symbols
    state_list*                 states  = new state_list();
    accept_action_for_state*    accept  = new accept_action_for_state();

    // Copy the accept actions
    for (accept_action_for_state::const_iterator it = m_Accept->begin(); it != m_Accept->end(); it++) {
        accept_action_list& action = (*accept)[it->first];
        for (accept_action_list::const_iterator actionIt = it->second.begin(); actionIt != it->second.end(); actionIt++) {
            action.push_back((*actionIt)->clone());
        }
    }

    // Recreate the states
    for (state_list::const_iterator stateIt = m_States->begin(); stateIt != m_States->end(); stateIt++) {
        // Create a new state
        states->push_back(new state((int)states->size()));
        state& newState = **(states->rbegin());
        
        // Create transitions for this state
        for (state::iterator transit = (*stateIt)->begin(); transit != (*stateIt)->end(); transit++) {
            // Get the new symbols for this transition
            int transitSet      = transit->symbol_set();
            int transitState    = transit->new_state();
            
            const remapped_symbol_map::new_symbol_set& newSyms = symbols->new_symbols(transitSet);
            
            for (remapped_symbol_map::new_symbol_set::const_iterator symIt = newSyms.begin(); symIt != newSyms.end(); symIt++) {
                newState.add(transition(*symIt, transitState));
            }
        }
    }
    
    // Create the new NDFA
    return new ndfa(states, symbols, accept);
}

/// \brief Internal method: computes the closure of the specified set of states (modifies the set to include 
/// all states reachable by epsilon transitions)
void ndfa::closure(set<int>& states) const {
    /// Set of states that need to be checked for epsilon transitions
    set<int> newStates = states;
    
    // Get the symbol ID of the epsilon set
    int epsSymbol = m_Symbols->identifier_for_symbols(epsilon());
    if (epsSymbol == -1) return;
    
    // Iterate until we've added no new states
    while (!newStates.empty()) {
        // Create a set of states we're going to add
        set<int> addedStates;
        
        for (set<int>::iterator stateIt = newStates.begin(); stateIt != newStates.end(); stateIt++) {
            // Find any epsilon transitions in this state
            const state& thisState = get_state(*stateIt);
            
            for (state::iterator transIt = thisState.begin(); transIt != thisState.end(); transIt++) {
                // Ignore non-epsilon transitions
                if (transIt->symbol_set() != epsSymbol) continue;
                
                // Ignore this state if it's already in the set
                int newState = transIt->new_state();
                if (states.find(newState) != states.end()) continue;
                
                // Add this state (and process it for further epsilons the next time through)
                states.insert(newState);
                addedStates.insert(newState);
            }
        }
        
        // The new states for the next iteration are the states we added in this iterator
        newStates = addedStates;
    }
}


/// \brief Creates a DFA from this NDFA
///
/// Note that if further transitions are added to the DFA, it may no longer be deterministic.
/// Use this call on the result of calling to_ndfa_with_unique_symbols.
ndfa* ndfa::to_dfa(const vector<int>& initialState) const {
    // Empty NDFA if no states were supplied
    if (initialState.size() == 0) { 
        return new ndfa();
    }
    
    // Some types used by this method
    typedef set<int>                        state_set;                                  // Set of states in this NDFA (maps onto a single state in the final NDFA)
    typedef pair<int, state*>               remaining_entry;                            // State that's waiting to be processed
    typedef map<int, state_set>             transition_for_symbol;                      // Maps a symbol_set to the states that would be reached in this NDFA
    
    // Create the structures for the new DFA. Symbols are preserved (and state 0 remains the same), but we regenerate everything else
    symbol_map*                 symbols     = new symbol_map(*m_Symbols);
    state_list*                 states      = new state_list();
    accept_action_for_state*    accept      = new accept_action_for_state();
    vector<state_set>           stateSets;
    
    // Get the epsilon set
    int epsilonSymbolSet = m_Symbols->identifier_for_symbols(epsilon());
    
    // Create a map saying which of our states are represented in each new state
    map<state_set, int> stateMap;
    
    // Create the stack of states to process
    stack<remaining_entry> remainingStates;

    // Create the set of initial states
    for (vector<int>::const_iterator initialIt = initialState.begin(); initialIt != initialState.end(); initialIt++) {
        // Create a set for this initial state
        state_set thisStateSet;
        thisStateSet.insert(*initialIt);
        closure(thisStateSet);
        
        // Put it in the state map
        int stateId = (int)states->size();
        
        // Generate a state for this ID
        states->push_back(new state(stateId));
        stateSets.push_back(thisStateSet);
        
        // Add to the map
        if (stateMap.find(thisStateSet) == stateMap.end()) {
            // (We create a new state if there's a duplicate initial state, but the first state we created becomes the 'canonical' one)
            stateMap[thisStateSet] = stateId;
        }
        
        // Add to the list of states to process
        remainingStates.push(remaining_entry(stateId, (*states)[stateId]));
    }
    
    // Keep processing states until we stop generating new ones
    while (!remainingStates.empty()) {
        // Get the next state to process
        remaining_entry next = remainingStates.top();
        remainingStates.pop();
        
        // Find the set of states reached by each symbol set for this transition
        transition_for_symbol statesForSymbol;
        
        // For each state making up this state...
        bool isEager = false;
        const state_set& nextSet = stateSets[next.first];
        
        for (state_set::const_iterator stateIt = nextSet.begin(); stateIt != nextSet.end(); stateIt++) {
            // Get this state
            state& thisState = *(*m_States)[*stateIt];
            
            // For each transition in this state, add to the appropriate set
            for (state::iterator transit = thisState.begin(); transit != thisState.end(); transit++) {
                // Ignore the epsilon set (covered by performing the closure)
                if (transit->symbol_set() == epsilonSymbolSet) continue;
                
                // Otherwise, add this transition
                statesForSymbol[transit->symbol_set()].insert(transit->new_state());
            }
            
            // Add the accepting actions for this state, if there are any
            accept_action_for_state::const_iterator acceptForState = m_Accept->find(*stateIt);
            if (acceptForState != m_Accept->end()) {
                for (accept_action_list::const_iterator acceptIt = acceptForState->second.begin(); acceptIt != acceptForState->second.end(); acceptIt++) {
                    // Add a clone of this action
                    (*accept)[next.second->identifier()].push_back((*acceptIt)->clone());
                    
                    // Mark if it's eager
                    if ((*acceptIt)->eager()) {
                        isEager = true;
                    }
                }
            }
        }
        
        // If this state is 'eager' (ie, accepts immediately), then there's no point in generating any transitions from it
        if (isEager) continue;
        
        // Generate the closures for epsilon transitions
        for (transition_for_symbol::iterator it = statesForSymbol.begin(); it != statesForSymbol.end(); it++) {
            closure(it->second);
        }
        
        // Generate new transitions for each symbol
        for (transition_for_symbol::const_iterator transit = statesForSymbol.begin(); transit != statesForSymbol.end(); transit++) {
            // Try to find state that this transition is targeting
            map<state_set, int>::const_iterator targetState = stateMap.find(transit->second);
            
            // Create a new state if there's no existing state
            if (targetState == stateMap.end()) {
                // Work on the new state ID
                int newStateId = (int) states->size();
                
                // Add to the state map
                targetState = stateMap.insert(pair<state_set, int>(transit->second, newStateId)).first;
                
                // Create the new state
                states->push_back(new state(newStateId));
                stateSets.push_back(transit->second);
                
                // Add the new state to the list that need processiing
                remainingStates.push(remaining_entry(newStateId, (*states)[newStateId]));
            }
            
            // Add this transition
            next.second->add(transition(transit->first, targetState->second));
        }
    }
    
    // Create the new NDFA from the result
    ndfa* result = new ndfa(states, symbols, accept);
    
    // Mark it as deterministic
    result->m_IsDeterministic = true;
    
    // Return it
    return result;
}

/// \brief Checks the symbol map to see if there are any overlapping ranges. Returns true if overlapping ranges exist.
///
/// This is a slow check to deal with cases where symbol ranges aren't sorting correctly. This should be true for
/// NDFAs returned by to_ndfa_with_unique_symbols() and to_dfa()
bool ndfa::verify_no_symbol_overlap() const {
    // Iterate through all of the symbols in the map
    return !m_Symbols->has_duplicates();
}

/// \brief Checks all of the states in this NDFA and returns true if there are no epsilon transitions and at most one
/// transition per symbol.
bool ndfa::verify_is_dfa() const {
    // Get the symbol ID representing 'epsilon'
    int epsSymbol = m_Symbols->identifier_for_symbols(epsilon());
    
    // Iterate through the states in this NDFA
    for (state_list::const_iterator nextState = m_States->begin(); nextState != m_States->end(); nextState++) {
        // Verify that there are no duplicate transitions in this state, and no epsilon transitions
        set<int> usedSymbols;
        
        // Count 'epsilon' as used right away, so it always generates a conflict
        usedSymbols.insert(epsSymbol);
        
        // Iterate through the transitions
        for (state::iterator nextTrans = (*nextState)->begin(); nextTrans != (*nextState)->end(); nextTrans++) {
            // Verify that this symbol isn't already used
            if (usedSymbols.find(nextTrans->symbol_set()) != usedSymbols.end()) {
                return false;
            }
            
            // Add this symbol set
            usedSymbols.insert(nextTrans->symbol_set());
        }
    }
    
    // Looks good
    return true;
}

/// \brief Class used to compare accepting actions
class order_actions {
public:
    /// Returns true if one accept action is less than another
    inline bool operator()(accept_action* a, accept_action* b) {
        if (a == b)     return false;
        if (a == NULL)  return true;
        if (b == NULL)  return false;
        
        return *a < *b;
    }
};

/// \brief Compacts a DFA, reducing the number of states
///
/// For DFAs with only a single initial state, this may have one extra state than is required. If firstAction is set
/// to true, then the resulting DFA will only have final states that contain the first action (rather than all possible
/// actions): this will generally result in a smaller DFA, at the cost of being able to distinguish states that are
/// ambiguous.
ndfa* ndfa::to_compact_dfa(const vector<int>& initialState, bool firstAction) const {
    // TODO: we can further compact the DFA by looking for symbol sets that always produce the same transition and merging them
    
    // Set of states from the original (this) DFA
    typedef set<int> state_set;
    
    // Vector of new states
    typedef vector<state_set> new_states;

    // Maps state IDs in the original (this) set to IDs in the 'new' set
    typedef map<int, int> state_map;
    
    // Set of actions
    typedef set<accept_action*, order_actions> action_set;

    // Variables used to represent our new state machine
    new_states  newStates;
    state_map   oldToNew;
    
    // Create the set of initial states. We assume that each state can only appear once in the initialState vector; the result
    // will be incorrect if they appear more than once
    for (vector<int>::const_iterator initial = initialState.begin(); initial != initialState.end(); initial++) {
        // Each initial state becomes an initial state in the new DFA
        newStates.push_back(state_set());
        state_set& thisState = newStates.back();
        
        // Create a new state containing just this item
        oldToNew[*initial] = (int) newStates.size()-1;
        thisState.insert(*initial);
    }
    
    // Bundle all of the non-accepting states up into a single state
    // (If all of the remaining states are accepting then we'll end up with a free bonus state here :-/)
    int nonAcceptingStates = (int) newStates.size();
    newStates.push_back(state_set());
    
    // Also remember which new state contains which set of accepting actions
    map<action_set, int> stateForActions;
    
    // Iterate through the states
    for (int stateId = 0; stateId < count_states(); stateId++) {
        // If this state is already mapped, then ignore it
        if (oldToNew.find(stateId) != oldToNew.end()) continue;
        
        // Try to fetch the accept actions for this state
        accept_action_for_state::const_iterator acceptActions = m_Accept->find(stateId);

        // If this state is not an accepting state then add it to the non-accepting set
        if (acceptActions == m_Accept->end() || acceptActions->second.empty()) {
            newStates[nonAcceptingStates].insert(stateId);
            oldToNew[stateId] = nonAcceptingStates;
        }
        
        // For accepting states, either create a new state or use an existing one
        else {
            // Build up a set of actions
            action_set actions;
            
            if (firstAction) {
                // Choose only the 'first' action (the one that compares 'highest')
                accept_action* smallestAction = acceptActions->second[0];
                for (accept_action_list::const_iterator action = acceptActions->second.begin(); 
                     action != acceptActions->second.end(); action++) {
                    if ((*smallestAction) < (**action)) {
                        smallestAction = *action;
                    }
                }
                
                // Add this action to the set
                actions.insert(smallestAction);
            } else {
                // Create a set of all of the accept actions for this state
                for (accept_action_list::const_iterator action = acceptActions->second.begin(); 
                     action != acceptActions->second.end(); action++) {
                    actions.insert(*action);
                }
            }
            
            // Get the state that's mapped to this set of actions
            map<action_set, int>::const_iterator existingState = stateForActions.find(actions);
            int targetState;
            
            if (existingState == stateForActions.end()) {
                // Create a new state
                targetState = (int) newStates.size()-1;
                newStates.push_back(state_set());
            } else {
                // Add to the existing state
                targetState = existingState->second;
            }
            
            // Add to the appropriate state
            newStates[targetState].insert(stateId);
            oldToNew[stateId] = targetState;
        }
    }
    
    // Now, iterate through the states until there are no changes to make
    bool changed = true;
    while (changed) {
        // No changes so far this pass
        changed = false;
        
        // Iterate through the set of 'new' states
        for (int newStateId = 0; newStateId < (int) newStates.size(); newStateId++) {
            // Split any symbol that has a transition to more than one different (new) state
            state_set thisState = newStates[newStateId];                        // TODO: use pointers to state sets instead
            if (thisState.size() <= 1) continue;
            
            // Use the first state in the set as the template; split off any state that doesn't match it
            map<int, int> targetStateForSymbol;
            
            state_set::iterator curState    = thisState.begin();
            const state&        firstState  = get_state(*curState);
            
            for (state::iterator transit = firstState.begin(); transit != firstState.end(); transit++) {
                targetStateForSymbol[transit->symbol_set()] = oldToNew[transit->new_state()];
            }
            
            // Iterate through the remaining states, and put any that have a different transition set into a new state
            int splitStateId    = -1;
            int numToMatch      = firstState.count_transitions();
            vector<int> toRemove;
            curState++;
            for (; curState != thisState.end(); curState++) {
                // Check through the items in this state
                const state&    originalState   = get_state(*curState);
                bool            different       = originalState.count_transitions() != numToMatch;
                
                if (!different) {
                    // Number of transitions are the same: need to check that they are actually the same set of transitions
                    for (state::iterator transit = originalState.begin(); transit != originalState.end(); transit++) {
                        // Check the target state
                        map<int, int>::iterator originalTransition = targetStateForSymbol.find(transit->symbol_set());
                        
                        // ... this state is different if the first state doesn't have this symbol set
                        if (originalTransition == targetStateForSymbol.end()) {
                            different = true;
                            break;
                        }
                        
                        // ... this state is different if the target state is different
                        if (originalTransition->second != oldToNew[transit->new_state()]) {
                            different = true;
                            break;
                        }
                    }
                }
                
                // Just leave the states that are the same alone
                if (!different) continue;
                
                // This state is different to the first state, so we split it off into a new set
                if (splitStateId == -1) {
                    // Assign a new state
                    changed         = true;
                    splitStateId    = (int) newStates.size()-1;
                    newStates.push_back(state_set());
                }
                
                // Move this state into the new state
                toRemove.push_back(*curState);                  // Have to remove the state later due to iterator semantics :-/
                newStates[splitStateId].insert(*curState);
                oldToNew[*curState] = splitStateId;
            }
            
            // Remove the states that are no longer in the current state
            for (vector<int>::iterator removeState = toRemove.begin(); removeState != toRemove.end(); removeState++) {
                // TODO: pointers!
                newStates[newStateId].erase(*removeState);
            }
        }
    }
    
    // Build the final state machine from the result

    // Build the transitions and accepting actions
    state_list*                 states      = new state_list();
    symbol_map*                 symbolMap   = new symbol_map(*m_Symbols);
    accept_action_for_state*    accept      = new accept_action_for_state();
    
    for (int newStateId = 0; newStateId < (int) newStates.size(); newStateId++) {
        // Ignore empty states
        if (newStates[newStateId].empty()) continue;
        
        state* newState = new state(newStateId);
        states->push_back(newState);
        
        // Add the transitions for this state: we only need a single template state as the mapped transitions for each symbol
        // will be the same
        int             templateStateId = *newStates[newStateId].begin();
        const state&    templateState   = get_state(templateStateId);
        
        for (state::iterator originalTransit = templateState.begin(); originalTransit != templateState.end(); originalTransit++) {
            int symbolSetId = originalTransit->symbol_set();
            int targetState = oldToNew[originalTransit->new_state()];
            
            newState->add(transition(symbolSetId, targetState));
        }
        
        // Copy the accept actions from the template state
        // TODO: if firstAction is set, then only copy the 'most important' action
        const accept_action_list&   actions         = actions_for_state(templateStateId);
        accept_action_list&         targetActions   = (*accept)[newStateId];
        for (accept_action_list::const_iterator act = actions.begin(); act != actions.end(); act++) {
            targetActions.push_back((*act)->clone());
        }
    }
    
    // Mark as deterministic
    ndfa* result = new ndfa(states, symbolMap, accept);
    result->m_IsDeterministic = m_IsDeterministic;
    
    // Return the minimized DFA
    return result;
}

/// \brief Creates a new NDFA (DFA if this is a DFA), merging any symbol sets that always produce the same action.
///
/// This will reduce the number of symbol sets in use by the DFA, which will reduce the size of the tables that 
/// are generated from it. This is particularly effective after calling to_compact_dfa() to eliminate any redundant
/// symbol sets.
ndfa* ndfa::to_ndfa_with_merged_symbols() const {
    // A set of all the unique symbols in this (N)DFA
    vector<set<int> >   uniqueSymbols;

    // Maps symbols in this DFA to symbols in our new DFA
    map<int, int>       symbolForSymbol;

    // Begin with a set of all symbols, as ID 0
    uniqueSymbols.push_back(set<int>());
    for (symbol_map::iterator symbolSet = m_Symbols->begin(); symbolSet != m_Symbols->end(); symbolSet++) {
        uniqueSymbols[0].insert(symbolSet->second);
        symbolForSymbol[symbolSet->second] = 0;
    }

    // Iterate through all of the states to find the symbol sets that are different from one another
    for (int stateId = 0; stateId < count_states(); stateId++) {
        // Fetch this state
        const state& thisState = get_state(stateId);

        // Maps new symbol sets to the state that they transfer to
        map<int, int> stateForSet;

        // Iterate through the transitions for this state
        for (state::iterator transit = thisState.begin(); transit != thisState.end(); transit++) {
            // Get the new symbol set for this transit
            int symbolSet = symbolForSymbol[transit->symbol_set()];

            // Look for the state that this symbol produces
            map<int, int>::iterator foundState = stateForSet.find(symbolSet);

            // Just remember this target state if it hasn't been encountered before
            if (foundState == stateForSet.end()) {
                stateForSet[symbolSet] = transit->new_state();
            } 

            // Otherwise, do nothing if this transition maps to the same state
            else if (foundState->second == transit->new_state()) {
                continue;
            }

            // This symbol set is different: create a new set consisting of all of the symbols that map to the same set and transit to this alternate state
            else {
                // Create a new symbol set
                uniqueSymbols.push_back(set<int>());
                int         newSetId    = (int)uniqueSymbols.size()-1;
                set<int>&   newSet      = uniqueSymbols.back();

                // Add any symbol that has the same set and target state
                for (state::iterator similarTransit = transit; similarTransit != thisState.end(); similarTransit++) {
                    // Check that this transit goes to the same state
                    if (similarTransit->new_state() != transit->new_state()) continue;

                    // It must also use the same symbol set as before
                    int originalSet = similarTransit->symbol_set();
                    int similarSet  = symbolForSymbol[originalSet];
                    if (similarSet != symbolSet) continue;

                    // This symbol should be remapped to the set we just created
                    uniqueSymbols[symbolSet].erase(originalSet);
                    newSet.insert(originalSet);
                    symbolForSymbol[originalSet] = newSetId;
                }
            }
        }
    }

    // Create the new symbol sets
    symbol_map* newSymbolMap = new symbol_map();

    // Iterate through the different symbol sets that we found
    for (int newSymbolId = 0; newSymbolId < (int) uniqueSymbols.size(); newSymbolId++) {
        // Get the old sets that are mapped to this new set
        set<int>& newSymbols = uniqueSymbols[newSymbolId];

        // Iterate through them to build up the final symbol set
        symbol_set newSymbolSet;
        for (set<int>::iterator oldSymbolId = newSymbols.begin(); oldSymbolId != newSymbols.end(); oldSymbolId++) {
            newSymbolSet |= (*m_Symbols)[*oldSymbolId];
        }

        // Add to the new map (relies on the symbol map counting from 0)
        newSymbolMap->identifier_for_symbols(newSymbolSet);
    }

    // Create the new set of states with the remapped symbol sets
    accept_action_for_state*    newActions  = new accept_action_for_state();
    state_list*                 newStates   = new state_list();
    for (int stateId = 0; stateId < count_states(); stateId++) {
        // Get the old state
        const state& oldState = get_state(stateId);

        // Create the new state
        state* newState = new state(stateId);
        newStates->push_back(newState);

        // Transform the transitions
        for (state::iterator transit=oldState.begin(); transit != oldState.end(); transit++) {
            newState->add(transition(symbolForSymbol[transit->symbol_set()], transit->new_state()));
        }

        // Copy the actions for this state
        const accept_action_list& oldActions    = actions_for_state(stateId);
        accept_action_list& newActionsForState  = (*newActions)[stateId];

        for (accept_action_list::const_iterator oldAct = oldActions.begin(); oldAct != oldActions.end(); oldAct++) {
            newActionsForState.push_back((*oldAct)->clone());
        }
    }

    // Return the new NDFA
    ndfa* result = new ndfa(newStates, newSymbolMap, newActions);
    result->m_IsDeterministic = m_IsDeterministic;
    return result;
}
