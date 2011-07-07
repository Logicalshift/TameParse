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
#include <stack>

#include "Dfa/lexeme.h"
#include "Dfa/basic_lexer.h"
#include "Lr/lalr_builder.h"
#include "Lr/parser_tables.h"
#include "Lr/parser_stack.h"

namespace lr {
    
    /// \brief Possible result codes of a parsing action
    class parser_result {
    public:
        enum result {
            /// \brief There is more parsing to do
            more,
            
            /// \brief The next symbol could not be recognised by the parser
            reject,
            
            /// \brief The language was accepted (reached the end and was reduced to a single nonterminal)
            accept
        };
    };

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
        
        /// \brief The parser stack
        typedef parser_stack<item_type> stack;
        
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
            : m_Actions(actions)
            , m_EndOfFile(false)
            , m_FirstState(NULL) {
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
            stack m_Stack;
            
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
                m_Session->m_Lookahead.erase(m_Session->m_Lookahead.begin(), m_Session->m_Lookahead.begin() + minPos);
                
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
                return m_Session->m_Lookahead[m_LookaheadPos];
            }
            
            ///
            /// \brief Performs the specified action
            ///
            /// No check is made to see if the action is valid: it is just performed. Returns true if the lookahead
            /// should be updated to be the next symbol.
            ///
            inline bool perform(const lexeme_container& lookahead, const action* act) {
                switch (act->m_Type) {
                    case lr_action::act_ignore:
                        // Discard the current lookahead
                        return true;
                        
                    case lr_action::act_shift:
                        // Push the lookahead onto the stack
                        m_Stack.push(act->m_NextState, m_Session->m_Actions->shift(lookahead));
                        return true;
                        
                    case lr_action::act_divert:
                        // Push the new state on to the stack
                        m_Stack.push(act->m_NextState, m_Session->m_Actions->shift(lookahead));
                        
                        // Leave the lookahead as-is
                        return false;
                        
                    case lr_action::act_reduce:
                    case lr_action::act_weakreduce:
                    {
                        // For reduce actions, the 'm_NextState' field actually refers to the rule that's being reduced
                        const parser_tables::reduce_rule& rule = m_Tables->rule(act->m_NextState);
                        
                        // Pop items from the stack, and create an item for them by calling the actions
                        reduce_list items;
                        for (int x=0; x < rule.m_Length; x++) {
                            items.push_back(m_Stack->item);
                            m_Stack.pop();
                        }
                        
                        // Fetch the state that's now on top of the stack
                        int gotoState = m_Stack->state;
                        
                        // Get the goto action for this nonterminal
                        for (parser_tables::action_iterator gotoAct = m_Tables->find_nonterminal(gotoState, rule.m_Identifier);
                             gotoAct != m_Tables->last_nonterminal_action(gotoState); 
                             gotoAct++) {
                            if (gotoAct->m_Type == lr_action::act_goto) {
                                // Found the goto action, perform the reduction
                                // (Note that this will perform the goto action for the next nonterminal if the nonterminal isn't in this state. This can only happen if the parser is in an invalid state)
                                m_Stack.push(gotoAct->m_NextState, m_Session->m_Actions->reduce(rule.m_Identifier, act->m_NextState, items));
                                break;
                            }
                        }
                        
                        // Done. If no goto was performed, we just chuck everything away associated with this rule
                        return false;
                    }
                        
                    case lr_action::act_goto:
                        // In general, this won't happen
                        m_Stack->state = act->m_NextState;
                        return false;
                        
                    case lr_action::act_guard:
                        // TODO: actually deal with guards
                        return true;
                        
                    default:
                        return false;
                }
            }
            
        private:
            /// \brief Fakes up a reduce action during can_reduce testing. act must be a reduce action
            inline void fake_reduce(parser_tables::action_iterator act, int& stackPos, std::stack<int>& pushed) {
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
            
            ///
            /// \brief Private class that provides methods for reading data about terminal symbols.
            ///
            /// Currently used for fetching the actions by the can_reduce implementation. This is the most used version, as it
            /// works with actions based on terminals.
            ///
            /// (Note that there are some actions generated for certain non-terminal symbols: specifically the end-of-input
            /// symbol, guard symbols and the end-of-guard symbol, hence the need for multiple versions of this)
            ///
            class terminal_fetcher {
            public:
                /// \brief The parser tables action iterator type
                typedef parser_tables::action_iterator action_iterator;
                
                /// \brief Returns an iterator pointing to the first action referring to a terminal symbol in the specified state
                inline static action_iterator find_symbol(const parser_tables* tables, int state, int terminal) {
                    return tables->find_terminal(state, terminal);
                }
                
                /// \brief Returns an iterator pointing to the last action referring to a terminal symbol in the specified state
                inline static action_iterator last_symbol_action(const parser_tables* tables, int state) {
                    return tables->last_terminal_action(state);
                }
            };
            
            ///
            /// \brief Private class that provides methods for reading data about non-terminal symbols.
            ///
            /// Mainly used for actions for things like guard symbols and the end of input symbol.
            ///
            class nonterminal_fetcher {
            public:
                /// \brief The parser tables action iterator type
                typedef parser_tables::action_iterator action_iterator;
                
                /// \brief Returns an iterator pointing to the first action referring to a terminal symbol in the specified state
                inline static action_iterator find_symbol(const parser_tables* tables, int state, int terminal) {
                    return tables->find_nonterminal(state, terminal);
                }
                
                /// \brief Returns an iterator pointing to the last action referring to a terminal symbol in the specified state
                inline static action_iterator last_symbol_action(const parser_tables* tables, int state) {
                    return tables->last_nonterminal_action(state);
                }
            };
            
            /// \brief Returns true if a reduction of the specified lexeme will result in it being shifted
            template<class symbol_fetcher> bool can_reduce(int symbol, int stackPos, std::stack<int> pushed) {
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
            
        public:
            /// \brief Returns true if a reduction of the specified lexeme will result in it being shifted
            ///
            /// In states with guards in their lookahead, or states with reduce/reduce conflicts that would
            /// be resolved by a LR(1) parser, this will disambiguate the grammar (making it possible to choose
            /// only the action that allows the parser to continue)
            inline bool can_reduce(const lexeme_container& lexeme) {
                return can_reduce<terminal_fetcher>(lexeme->matched(), 0, std::stack<int>());
            }
            
            /// \brief Returns true if a reduction of the lookahead will result in it being shifted
            inline bool can_reduce() {
                return can_reduce(look());
            }
            
        private:
            /// \brief As for can_reduce, but with a fake nonterminal lookahead value
            inline bool can_reduce_nonterminal(int nt) {
                return can_reduce<nonterminal_fetcher>(nt, 0, std::stack<int>());
            }
            
        public:
            typedef parser_result::result result;
            
            /// \brief Performs a single parsing action, and returns the result
            inline result process() {
                // Fetch the lookahead
                const lexeme_container& la = look();
                
                // Get the state
                int state = m_Stack->state;
                
                // Get the action for this lookahead
                int sym;
                parser_tables::action_iterator act;
                parser_tables::action_iterator end;

                if (la.item() != NULL) {
                    // The item is a terminal
                    sym = la->matched();
                    act = m_Tables->find_terminal(state, sym);
                    end = m_Tables->last_terminal_action(state);
                } else {
                    // The item is the end-of-input symbol (which counts as a nonterminal)
                    sym = m_Tables->end_of_input();
                    act = m_Tables->find_nonterminal(state, sym);
                    end = m_Tables->last_nonterminal_action(state);
                }
                
                // Work out which action to perform
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
                            // Reach the end of input: check can_reduce for the EOI symbol
                            if (!can_reduce_nonterminal(m_Tables->end_of_input())) {
                                continue;
                            }
                        }
                    }
                    
                    // Perform this action
                    if (act->m_Type == lr_action::act_accept) return parser_result::accept;
                    
                    if (perform(la, act)) {
                        // Move on to the next lookahead value if needed
                        next();
                    }
                    return parser_result::more;
                }
                
                // We reject if we reach here
                return parser_result::reject;
            }
            
            /// \brief Parses the input file specified by the actions object, and returns true if it was accepted
            /// or false if it was rejected.
            inline bool parse() {
                for (;;) {
                    // Perform the next action
                    result next = process();
                    
                    // Keep going if there are more results
                    if (next == parser_result::more) continue;
                    
                    // Stop, and indicate if the result was successful
                    return next == parser_result::accept;
                }
            }
            
            /// \brief Returns the parser stack associated with this state
            inline const stack& get_stack() const {
                return m_Stack;
            }
            
            /// \brief Gets the parser item on top of the stack
            inline const item_type& get_item() const {
                return m_Stack->item;
            }
        };
        
    public:
        /// \brief Factory method that creates a new parser. The actions will be destroyed when the state is destroyed.
        inline state* create_parser(parser_actions* actions, int initialState = 0) const {
            session* newSession = new session(actions);
            return new state(m_ParserTables, initialState, newSession);
        }
    };
    
    ///
    /// \brief Basic parser actions class
    ///
    /// This is only useful for determining if a stream available from a lexer will be matched by the language.
    /// The item type should be 'int' for this object
    ///
    class simple_parser_actions {
    private:
        /// \brief The lexer associated with the object, destroyed when the object is destructed
        dfa::lexeme_stream* m_Lexer;
        
        simple_parser_actions(const simple_parser_actions& noCopying) { }
        simple_parser_actions& operator=(const simple_parser_actions& noCopying) { return *this; }
        
    public:
        /// \brief Creates a new actions object that will read from the specified stream.
        ///
        /// The stream will be deleted when this object is deleted
        simple_parser_actions(dfa::lexeme_stream* lexer)
        : m_Lexer(lexer) {
        }
        
        /// \brief Destroys an existing actions object
        ~simple_parser_actions() { delete m_Lexer; }
        
        /// \brief Reads the next symbol from the stream
        inline dfa::lexeme* read() {
            dfa::lexeme* result = NULL;
            (*m_Lexer) >> result;
            return result;
        }
        
        /// \brief Returns the item resulting from a shift action
        inline int shift(const dfa::lexeme_container& lexeme) {
            return 0;
        }
        
        /// \brief Returns the item resulting from a reduce action
        inline int reduce(int nonterminal, int rule, const parser<int, simple_parser_actions>::reduce_list& reduce) {
            return 0;
        }
    };
    
    /// \brief Simple parser, can be used to test if a language is accepted by a parser (but not much else)
    typedef parser<int, simple_parser_actions> simple_parser;
}

#endif