//
//  parser.h
//  Parse
//
//  Created by Andrew Hunter on 13/05/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef _LR_PARSER_H
#define _LR_PARSER_H

#include <vector>

#include "Dfa/lexeme.h"
#include "Lr/lalr_builder.h"
#include "Lr/parser_tables.h"
#include "Lr/parser_stack.h"

namespace lr {
    ///
    /// \brief Generic parser implementation.
    ///
    template<typename item_type, typename parser_actions> class parser {
    private:
        /// \brief The parser tables
        parser_tables m_ParserTables;
        
    public:
        /// \brief Lexeme type
        typedef dfa::lexeme lexeme;
        typedef dfa::lexeme_container lexeme_container;
        
        /// \brief Parser action
        typedef parser_tables::action action;
        
        /// \brief List of items passed to a reduce action
        typedef std::vector<item_type> reduce_list;
        
        /// \brief Forward declaration of the state class
        class state;
        
        /// \brief Creates a parser by copying the tables
        parser(const parser_tables& tables) 
        : m_ParserTables(tables) { }

        /// \brief Creates a parser from the result of the specified builder class
        parser(const lalr_builder& builder) 
        : m_ParserTables(builder) { }

        /// \brief Destructor
        virtual ~parser() { }
        
    private:
        ///
        /// \brief Class representing a session with the parser
        ///
        /// A session consists of several states, and one stream of symbols from the lexer. Multiple parser
        /// states are used when implementing backtracking or GLR parsers. There is only one stream of
        /// symbols, however, which must be shared between all the states
        ///
        class session {
        public:
            friend class state;
            
            /// \brief Session lookahead
            typedef std::vector<dfa::lexeme_container> lookahead_list;
            
        private:
            /// \brief The symbols that are in the parser lookahead
            lookahead_list m_Lookahead;
            
            /// \brief Set to true if we've reached the end of the file
            bool m_EndOfFile;
            
            /// \brief The first active state in the parser
            state* m_FirstState;
            
            /// \brief The parser actions for this session
            parser_actions* m_Actions;

        public:
            session(parser_actions* actions)
            : m_Actions(actions) {
            }
            
            ~session() {
                // We consider that the parser owns its own actions, so we destroy them here
                delete m_Actions;
            }
        };
        
    public:
        friend class state;
        
        ///
        /// \brief Class representing a parser state
        ///
        class state {
        private:
            friend class parser;
            friend class session;
            
            /// \brief The parser tables for this state
            const parser_tables* m_Tables;
            
            /// \brief The parser stack in this state
            parser_stack<item_type> m_Stack;
            
            /// \brief The session that this is a part of
            session* m_Session;
            
            /// \brief The position in the lookahead of this state
            int m_LookaheadPos;
            
            /// \brief The next state in the state list
            state* m_NextState;
            
            /// \brief The last state in the state list
            state* m_LastState;
            
        private:
            /// \brief States can't be assigned
            state& operator=(const state& noAssignment) { }
            
        private:
            /// \brief Constructs a new state, used by the parser
            state(const parser_tables& tables, int initialState, session* session) 
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
            
        public:
            /// \brief Creates a new parser state by copying an old one. Parser states can be run independently.
            state(const state& copyFrom)
            : m_Tables(copyFrom.m_Tables)
            , m_Session(copyFrom.m_Session)
            , m_Stack(copyFrom.m_Stack)
            , m_LookaheadPos(copyFrom.m_LookaheadPos) {
                m_NextState             = m_Session->m_FirstState;
                m_LastState             = NULL;
                m_Session->m_FirstState = this;
                
                if (m_NextState) m_NextState->m_LastState = this;
            }
            
        public:
            /// \brief Destructor
            ~state() {
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
            
        private:
            /// \brief Trims the lookahead in the sessions (removes any symbols that won't be visited again)
            inline void trim_lookahead() {
                // Find the minimum lookahead position in all of the states
                int minPos = m_LookaheadPos;
                for (state* whichState = m_Session->m_FirstState; whichState != NULL; whichState = whichState->m_NextState) {
                    if (whichState->m_LookaheadPos < minPos) minPos = whichState->m_LookaheadPos;
                }
                
                // Give up if there's no work to do
                if (minPos == 0) return;
                
                // Remove the symbols from the session
                m_Session->m_Lookahead.erase(0, minPos);
                
                // Update the state lookahead positions
                for (state* whichState = m_Session->m_FirstState; whichState != NULL; whichState = whichState->m_NextState) {
                    whichState->m_LookaheadPos -= minPos;
                }
            }
            
        public:
            ///
            /// \brief Moves on a single symbol (ie, throws away the current lookahead)
            ///
            /// It is an error to call this without calling lookahead() at least once since the last call.
            ///
            inline void next() {
                m_LookaheadPos++;
                trim_lookahead();
            }
            
            ///
            /// \brief Retrieves the current lookahead character
            ///
            inline const lexeme_container& look() {
                // Static lexeme container representing the end of the file
                static lexeme_container endOfFile((lexeme*)NULL);
                
                // Read a new symbol if necessary
                while (m_LookaheadPos >= m_Session->m_Lookahead.size()) {
                    if (!m_Session->m_EndOfFile) {
                        // Read the next symbol using the parser actions
                        dfa::lexeme_container nextLexeme(m_Session->m_Actions->read());

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
                return m_Session->m_Lookahead[m_LookaheadPos];
            }
            
            ///
            /// \brief Performs the specified action
            ///
            /// No check is made to see if the action is valid: it is just performed.
            ///
            inline void act(const lexeme_container& lookahead, const action* act) {
                switch (act->m_Type) {
                    case lr_action::act_ignore:
                        // Discard the current lookahead
                        look();
                        next();
                        break;
                        
                    case lr_action::act_shift:
                        // Push the lookahead onto the stack
                        m_Stack.push(act->m_NextState, m_Session->m_Actions->shift(look()));
                        break;
                        
                    case lr_action::act_reduce:
                    case lr_action::act_weakreduce:
                    {
                        // For reduce actions, the 'm_NextState' field actually refers to the rule that's being reduced
                        const parser_tables::reduce_rule& rule = m_Tables->rule(act->m_NextState);
                        
                        // Pop items from the stack, and create an item for 
                        reduce_list items;
                        for (int x=0; x < rule.m_Length; x++) {
                            items.push_back(m_Stack->item);
                            m_Stack.pop();
                        }
                        
                        // Fetch the state that's now on top of the stack
                        int gotoState = m_Stack->state;
                        
                        // Get the goto action for this nonterminal
                        for (parser_tables::action_iterator gotoAct = m_Tables->find_nonterminal(gotoState, rule.m_Identifier);
                             gotoAct != m_Tables->last_nonterminal_action(); 
                             gotoAct++) {
                            if (gotoAct->m_Type == lr_action::act_goto) {
                                // Found the goto action, perform the reduction
                                // (Note that this will perform the goto action for the next nonterminal if the nonterminal isn't in this state. This can only happen if the parser is in an invalid state)
                                m_Stack.push(gotoAct->m_NextState, m_Session->m_Actions->reduce(rule.m_Identifier, act->m_NextState, items));
                                break;
                            }
                        }
                        
                        // Done. If no goto was performed, we just chuck everything away associated with this rule
                        break;
                    }
                        
                    case lr_action::act_goto:
                        // In general, this won't happen
                        m_Stack->state = act->m_NextState;
                        break;
                }
            }
        };
        
    public:
        /// \brief Factory method that creates a new parser. The actions will be destroyed when the state is destroyed.
        inline state* create_parser(parser_actions* actions, int initialState = 0) {
            session* newSession = new session(actions);
            return new state(m_ParserTables, initialState, newSession);
        }
    };
}

#endif