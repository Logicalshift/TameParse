//
//  ndfa.cpp
//  Parse
//
//  Created by Andrew Hunter on 26/04/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <stack>

#include "TameParse/Dfa/ndfa.h"
#include "TameParse/Dfa/transition.h"
#include "TameParse/Dfa/remapped_symbol_map.h"

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

/// \brief Copies the values from the specified NDFA into this one (replacing any values it might already contain)
ndfa& ndfa::operator=(const ndfa& assignFrom) {
    // Nothing to do if we're trying to assign to ourselves
    if (&assignFrom == this) return *this;

    // Destroy any accept actions that might be in this NDFA
    for (accept_action_for_state::iterator acceptState = m_Accept->begin(); acceptState != m_Accept->end(); acceptState++) {
        for (accept_action_list::iterator actionList = acceptState->second.begin(); actionList != acceptState->second.end(); actionList++) {
            delete *actionList;
        }
    }

    m_Accept->clear();
    
    // Destroy any states
    for (state_list::iterator stateIt = m_States->begin(); stateIt != m_States->end(); stateIt++) {
        delete *stateIt;
    }

    m_States->clear();

    // Copy the states from the target NDFA
    for (int stateId=0; stateId < assignFrom.count_states(); stateId++) {
        m_States->push_back(new state(assignFrom.get_state(stateId)));
    }

    // Copy the symbol map for this NDFA
    *m_Symbols = assignFrom.symbols();

    // Copy the accepting states for this NDFA
    for (accept_action_for_state::const_iterator acceptAct = assignFrom.m_Accept->begin(); acceptAct != assignFrom.m_Accept->end(); ++acceptAct) {
        // Begin creating a new action list
        accept_action_list& newActions = (*m_Accept)[acceptAct->first];

        // Iterate through the list of actions for this state
        for (accept_action_list::const_iterator copyAction = acceptAct->second.begin(); copyAction != acceptAct->second.end(); copyAction++) {
            // Ignore NULL actions
            if (!*copyAction) continue;

            // Clone this action
            newActions.push_back((*copyAction)->clone());
        }
    }

    // Copy the remaining parameters
    m_IsDeterministic   = assignFrom.m_IsDeterministic;
    m_CurrentState      = assignFrom.m_CurrentState;
    
    // Done
    return *this;
}
