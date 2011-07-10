//
//  parser_state.h
//  Parse
//
//  Created by Andrew Hunter on 10/07/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef _LR_PARSER_STATE_H
#define _LR_PARSER_STATE_H

#include "Lr/parser.h"

namespace lr {
    ///
    /// \brief Constructs a new state, used by the parser
    ///
    template<typename item_type, typename parser_actions> parser<item_type, parser_actions>::state::state(const parser_tables& tables, int initialState, session* session) 
    : m_Tables(&tables)
    , m_Session(session)
    , m_LookaheadPos(0) {
        // Push the initial state
        m_Stack->state          = initialState;
        m_NextState             = m_Session->m_FirstState;
        m_LastState             = NULL;
        m_Session->m_FirstState = this;
        
        if (m_NextState) m_NextState->m_LastState = this;
    }
    
    ///
    /// \brief Creates a new parser state by copying an old one. Parser states can be run independently.
    ///
    template<typename item_type, typename parser_actions> parser<item_type, parser_actions>::state::state(const state& copyFrom)
    : m_Tables(copyFrom.m_Tables)
    , m_Session(copyFrom.m_Session)
    , m_Stack(copyFrom.m_Stack)
    , m_LookaheadPos(copyFrom.m_LookaheadPos) {
        m_NextState             = m_Session->m_FirstState;
        m_LastState             = NULL;
        m_Session->m_FirstState = this;
        
        if (m_NextState) m_NextState->m_LastState = this;
    }
    
    ///
    /// \brief Destructor
    ///
    template<typename item_type, typename parser_actions> parser<item_type, parser_actions>::state::~state() {
        // Remove this state from the session
        if (m_LastState) {
            m_LastState->m_NextState = m_NextState;
        } else {
            m_Session->m_FirstState = m_NextState;
        }
        if (m_NextState) {
            m_NextState->m_LastState = m_LastState;
        }
        
        // Destroy the session if it is now empty of states
        if (!m_Session->m_FirstState) {
            delete m_Session;
        }
    }

    ///
    /// \brief Trims the lookahead in the sessions (removes any symbols that won't be visited again)
    ///
    template<typename I, typename A> inline void parser<I, A>::state::trim_lookahead() {
        // Find the minimum lookahead position in all of the states
        int minPos = m_LookaheadPos;
        for (state* whichState = m_Session->m_FirstState; whichState != NULL; whichState = whichState->m_NextState) {
            if (whichState->m_LookaheadPos < minPos) minPos = whichState->m_LookaheadPos;
                }
        
        // Give up if there's no work to do
        if (minPos == 0) return;
        
        // Remove the symbols from the session
        m_Session->m_Lookahead.erase(m_Session->m_Lookahead.begin(), m_Session->m_Lookahead.begin() + minPos);
        
        // Update the state lookahead positions
        for (state* whichState = m_Session->m_FirstState; whichState != NULL; whichState = whichState->m_NextState) {
            whichState->m_LookaheadPos -= minPos;
        }
    }

    ///
    /// \brief Moves on a single symbol (ie, throws away the current lookahead)
    ///
    /// It is an error to call this without calling lookahead() at least once since the last call.
    ///
    template<typename I, typename A> inline void parser<I, A>::state::next() {
        m_LookaheadPos++;
        trim_lookahead();
    }
    
    ///
    /// \brief Retrieves the current lookahead character
    ///
    template<typename I, typename A> inline const typename parser<I, A>::lexeme_container& parser<I, A>::state::look(int offset) {
        // Static lexeme container representing the end of the file
        static lexeme_container endOfFile((lexeme*)NULL);
        
        // Read a new symbol if necessary
        int pos = m_LookaheadPos + offset;
        
        while (pos >= m_Session->m_Lookahead.size()) {
            if (!m_Session->m_EndOfFile) {
                // Read the next symbol using the parser actions
                dfa::lexeme_container nextLexeme(m_Session->m_Actions->read(), true);
                
                // Flag up an end of file condition
                if (nextLexeme.item() == NULL) {
                    m_Session->m_EndOfFile = true;
                    return endOfFile;
                }
                
                // Store in the lookahead
                m_Session->m_Lookahead.push_back(nextLexeme);
            } else {
                // EOF
                return endOfFile;
            }
        }
        
        // Return the current symbol
        return m_Session->m_Lookahead[pos];
    }

    ///
    /// \brief Performs the specified action
    ///
    /// No check is made to see if the action is valid: it is just performed. Returns true if the lookahead
    /// should be updated to be the next symbol.
    ///
    /// This is a generic routine that can have a class supplied to change how the result of the action is
    /// generated. This is to support guard actions (where we are only interested in storing the state) as
    /// well as standard actions (where we want to call the actions object to actually perform the action)
    ///
    template<class I, class A> /* C++ you suck */ template<class actions> inline bool parser<I, A>::state::perform_generic(const lexeme_container& lookahead, const action* act, actions& actDelegate) {
        switch (act->m_Type) {
            case lr_action::act_ignore:
                // Discard the current lookahead
                return true;
                
            case lr_action::act_shift:
                // Push the lookahead onto the stack
                actDelegate.shift(this, act, lookahead);
                return true;
                
            case lr_action::act_divert:
                // Push the new state on to the stack
                actDelegate.shift(this, act, lookahead);
                
                // Leave the lookahead as-is
                return false;
                
            case lr_action::act_reduce:
            case lr_action::act_weakreduce:
            {
                // For reduce actions, the 'm_NextState' field actually refers to the rule that's being reduced
                const parser_tables::reduce_rule& rule = m_Tables->rule(act->m_NextState);
                
                // Pop items from the stack, and create an item for them by calling the actions
                actDelegate.reduce(this, act, rule);
                
                // Done. If no goto was performed, we just chuck everything away associated with this rule
                return false;
            }
                
            case lr_action::act_goto:
                // In general, this won't happen
                actDelegate.set_state(this, act->m_NextState);
                return false;
                
            case lr_action::act_guard:
            {
                // Check the guard symbol
                int guardSym = check_guard(act->m_NextState, 0);
                
                if (guardSym >= 0) {
                    // The guard symbol was matched
                    abort();
                }
                
                // The guard symbol was not matched
                return true;
            }
                
            default:
                return false;
        }
    }

    
    ///
    /// \brief Checks the lookahead against the guard condition which starts at the specified initial state
    ///
    /// This runs the parser forward from the specified state. If a 'end of guard' symbol is encountered and
    /// can produce an accepting state, then this will return the ID of the guard symbol that was accepted.
    /// If no accepting state is reached, this will return a negative value (generally -1)
    ///
    template<class I, class A> int parser<I,A>::state::check_guard(int initialState, int initialOffset) {
        // Create the guard actions object
        guard_actions guardActions(initialState, initialOffset);
        
        // Set to true once the EOG symbol can be reduced
        bool canReduceEog = false;
        
        // Perform parser actions to decide if the guard is accepted or not
        for (;;) {
            // Fetch the lookahead
            const lexeme_container& la = look(guardActions.offset());
            
            // Get the current state
            int state = guardActions.current_state();
            
            // Get the action for this lookahead
            int sym;
            bool isTerminal;
            parser_tables::action_iterator act;
            parser_tables::action_iterator end;
            
            if (la.item() != NULL) {
                // The item is a terminal
                sym         = la->matched();
                act         = m_Tables->find_terminal(state, sym);
                end         = m_Tables->last_terminal_action(state);
                isTerminal  = true;
            } else {
                // The item is the end-of-input symbol (which counts as a nonterminal)
                sym         = m_Tables->end_of_input();
                act         = m_Tables->find_nonterminal(state, sym);
                end         = m_Tables->last_nonterminal_action(state);
                isTerminal  = false;
            }
            
            // Reduce the EOG symbol as soon as possible
            if (m_Tables->has_end_of_guard(state)) {
                // Check if we can reduce the EOG symbol. No need to check twice.
                if (!canReduceEog) {
                    canReduceEog = can_reduce_nonterminal(m_Tables->end_of_guard());
                }
                
                // Switch to the EOG action if it exists
                sym         = m_Tables->end_of_guard();
                act         = m_Tables->find_nonterminal(state, sym);
                end         = m_Tables->last_nonterminal_action(state);
                isTerminal  = false;
            }
            
            // Work out which action to perform
            bool ok = false;
            for (; act != end; act++) {
                // Stop searching if the symbol is invalid
                if (act->m_SymbolId != sym) break;
                
                // If this is a weak reduce action, then check if the action is successful
                if (act->m_Type == lr_action::act_weakreduce) {
                    if (la.item() != NULL) {
                        // Standard symbol: use the usual form of can_reduce
                        if (!can_reduce(la)) {
                            continue;
                        }
                    } else {
                        // Reached the end of input: check can_reduce for the EOI symbol
                        if (!can_reduce_nonterminal(m_Tables->end_of_input())) {
                            continue;
                        }
                    }
                }
                
                // Perform this action
                if (act->m_Type == lr_action::act_accept) {
                    // Get the accepting rule
                    const parser_tables::reduce_rule& rule = m_Tables->rule(act->m_NextState);
                    
                    // Return the nonterminal ID for this rule, which should be the ID of the guard that was 
                    // matched
                    return rule.m_Identifier;
                }
                
                if (perform_generic(la, act, guardActions)) {
                    // Move on to the next lookahead value if needed
                    guardActions.next();
                }
                
                // Performed the action
                ok = true;
                break;
            }
            
            if (!ok) {
                // We reject if we reach here
                return -1;
            }
        }
        
        return -1;
    }
    
    /// \brief Fakes up a reduce action during can_reduce testing. act must be a reduce action
    template<class I, class A> inline void parser<I, A>::state::fake_reduce(parser_tables::action_iterator act, int& stackPos, std::stack<int>& pushed) {
        // Get the reduce rule
        const parser_tables::reduce_rule& rule = m_Tables->rule(act->m_NextState);
        
        // Pop items from the stack
        for (int x=0; x<rule.m_Length; x++) {
            if (!pushed.empty()) {
                // If we've pushed a fake state, then remove it from the stack
                pushed.pop();
            } else {
                // Update the 'real' stack position
                stackPos--;
            }
        }
        
        // Work out the current state
        int state;
        if (!pushed.empty()) {
            state = pushed.top();
        } else {
            state = m_Stack[stackPos].state;
        }
        
        // Work out the goto action
        parser_tables::action_iterator gotoAct = m_Tables->find_nonterminal(state, rule.m_Identifier);
        for (; gotoAct != m_Tables->last_nonterminal_action(state); gotoAct++) {
            if (gotoAct->m_Type == lr_action::act_goto) {
                // Push this goto
                pushed.push(gotoAct->m_NextState);
                break;
            }
        }
    }
    
    /// \brief Returns true if a reduction of the specified lexeme will result in it being shifted
    template<class I, class A> template<class symbol_fetcher> bool parser<I, A>::state::can_reduce(int symbol, int stackPos, std::stack<int> pushed) {
        // Get the new state
        int state;
        if (!pushed.empty()) {
            // (This will always happen unless there's a bug in the parser tables)
            state = pushed.top();
        } else {
            state = m_Stack[stackPos].state;
        }
        
        // Get the initial action for the terminal
        parser_tables::action_iterator act = symbol_fetcher::find_symbol(m_Tables, state, symbol);
        
        // Find the first reduce action for this item
        while (act != symbol_fetcher::last_symbol_action(m_Tables, state)) {
            // Fail if there are no actions for this terminal
            if (act->m_SymbolId != symbol) return false;
            
            switch (act->m_Type) {
                case lr_action::act_shift:
                case lr_action::act_accept:
                    // This terminal will result in a shift: this is successful
                    return true;
                    
                case lr_action::act_divert:
                    // Push the new state to the stack
                    pushed.push(act->m_NextState);
                    break;
                    
                    // TODO: guards (how to deal with these, there's limited lookahead here, and it's possible that they'll allow a reduction to continue). Hrm, maybe we can succeed if either of the possible paths lead to a successful reduction, though we need to know the rule ID to do that...
                    
                case lr_action::act_weakreduce:
                {
                    // To deal with weak reduce actions, we need to fake up the reduction and try again
                    // Use a separate stack so we can carry on after the action
                    int             weakPos = stackPos;
                    std::stack<int> weakStack(pushed);
                    
                    // If we can reduce via this item, then the result is true
                    fake_reduce(act, weakPos, weakStack);
                    if (can_reduce<symbol_fetcher>(symbol, weakPos, weakStack)) return true;
                    
                    // If not, keep looking for a stronger action
                    act++;
                    break;
                }
                    
                case lr_action::act_reduce:
                {
                    // Update our 'fake state' to be whatever will happen due to this reduce
                    fake_reduce(act, stackPos, pushed);
                    
                    // Get the new state
                    int state;
                    if (!pushed.empty()) {
                        // (This will always happen unless there's a bug in the parser tables)
                        state = pushed.top();
                    } else {
                        state = m_Stack[stackPos].state;
                    }
                    
                    // Get the initial action for the terminal
                    act = symbol_fetcher::find_symbol(m_Tables, state, symbol);
                    
                    // Carry on looking with the new state
                    break;
                }
                    
                default:
                    // Other actions fail
                    return false;
            }
        }
        
        // Result is false if we run out of actions
        return false;
    }

    /// \brief Performs a single parsing action, and returns the result
    ///
    /// This version takes several parameters: the current lookahead token, the ID of the symbol and whether or not it's
    /// a terminal symbol, and the range of actions that might apply to this particular symbol.
    template<class I, class A> template<class actions> inline parser_result::result parser<I,A>::state::process_generic(
                                                          actions& actDelegate, 
                                                          const lexeme_container& la, 
                                                          int symbol, bool isTerminal,
                                                          parser_tables::action_iterator& act, 
                                                          parser_tables::action_iterator& end) {
        // Work out which action to perform
        for (; act != end; act++) {
            // Stop searching if the symbol is invalid
            if (act->m_SymbolId != symbol) break;
            
            // If this is a weak reduce action, then check if the action is successful
            if (act->m_Type == lr_action::act_weakreduce) {
                if (isTerminal) {
                    if (!can_reduce(symbol)) {
                        continue;
                    }
                } else {
                    if (!can_reduce_nonterminal(symbol)) {
                        continue;
                    }
                }
            }
            
            // Perform this action
            if (act->m_Type == lr_action::act_accept) return parser_result::accept;
            
            if (perform_generic(la, act, actDelegate)) {
                // Move on to the next lookahead value if needed
                actDelegate.next(this);
            }
            return parser_result::more;
        }
        
        // We reject if we reach here
        return parser_result::reject;
    }
    
    /// \brief Performs a single parsing action, and returns the result
    template<class I, class A> template<class actions> inline parser_result::result parser<I,A>::state::process_generic(actions& actDelegate) {
        // Fetch the lookahead
        const lexeme_container& la = actDelegate.look(this);
        
        // Get the state
        int state = m_Stack->state;
        std::wcerr << state << L" " << (la.item()?la->content<wchar_t>():L"$") << L" (" << (la.item()?la->matched():-1) << L")" << std::endl;
        
        // Get the action for this lookahead
        int                             sym;
        parser_tables::action_iterator  act;
        parser_tables::action_iterator  end;
        bool                            isTerminal;
        
        if (la.item() != NULL) {
            // The item is a terminal
            sym         = la->matched();
            isTerminal  = true;
            act         = m_Tables->find_terminal(state, sym);
            end         = m_Tables->last_terminal_action(state);
        } else {
            // The item is the end-of-input symbol (which counts as a nonterminal)
            sym         = m_Tables->end_of_input();
            isTerminal  = false;
            act         = m_Tables->find_nonterminal(state, sym);
            end         = m_Tables->last_nonterminal_action(state);
        }
        
        return process_generic(actDelegate, la, sym, isTerminal, act, end);
    }
}

#endif
