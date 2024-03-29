//
//  parser.h
//  Parse
//
//  Created by Andrew Hunter on 13/05/2011.
//  
//  Copyright (c) 2011-2012 Andrew Hunter
//  
//  Permission is hereby granted, free of charge, to any person obtaining a copy 
//  of this software and associated documentation files (the \"Software\"), to 
//  deal in the Software without restriction, including without limitation the 
//  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or 
//  sell copies of the Software, and to permit persons to whom the Software is 
//  furnished to do so, subject to the following conditions:
//  
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//  
//  THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
//  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS 
//  IN THE SOFTWARE.
//

#ifndef _LR_PARSER_H
#define _LR_PARSER_H

#include <vector>
#include <stack>
#include <iostream>

#include "TameParse/Dfa/lexeme.h"
#include "TameParse/Dfa/basic_lexer.h"
#include "TameParse/Lr/lalr_builder.h"
#include "TameParse/Lr/parser_tables.h"
#include "TameParse/Lr/parser_stack.h"

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
    /// \brief Parser trace class that performs no actions
    ///
    /// You can implement a class with the same signature as this one (or derive from it to ensure future compatibility)
    /// in order to get information about the actions that the parser is performing.
    ///
    class no_parser_trace {
    public:
        /// \brief Lexeme type
        typedef dfa::lexeme lexeme;
        typedef dfa::lexeme_container lexeme_container;

    public:
        inline void ignore(const lexeme_container& lookahead)               { }
        inline void shift(const lexeme_container& lookahead, int newState)  { }
        inline void reduce(int nonterminalId, int ruleId, int length)       { }
        inline void goto_state(int newState)                                { }
        inline void checked_guard(int initialState, int result)             { }
        inline void reject(const lexeme_container& lookahead)               { }
    };
    
    ///
    /// \brief Parser trace class that writes trace data to wcerr
    ///
    /// Declare with level set to values greater than 0 to increase the amount of data that's presented
    ///
    template<int level> class debug_parser_trace : public no_parser_trace {
    public:
        inline void ignore(const lexeme_container& lookahead) {
            if (level > 1) {
                std::wcerr << L"IGNORE: " << lookahead->content<wchar_t>() << L" (" << lookahead->matched() << L")" << std::endl;
            }
        }
        
        inline void shift(const lexeme_container& lookahead, int newState) {
            std::wcerr << L"SHIFT TO " << newState << L": " << lookahead->content<wchar_t>() << L" (" << lookahead->matched() << L")" << std::endl;
        }
        
        inline void reduce(int nonterminalId, int ruleId, int length) {
            std::wcerr << L"REDUCE " << nonterminalId << L"(" << length << L" symbols)" << std::endl;
        }
        
        inline void goto_state(int newState) {
            std::wcerr << L"GOTO " << newState << std::endl;
        }
        
        inline void checked_guard(int initialState, int result) {
            if (level > 0) {
                std::wcerr << L"CHECK GUARD RESULT: " << result << L" (from state " << initialState << L")\n";
            }
        }

        inline void reject(const lexeme_container& lookahead) {
            std::wcerr << L"REJECT: " << lookahead->content<wchar_t>() << L" (" << lookahead->matched() << L")" << std::endl;
            std::wcerr << L"At line: " << lookahead->pos().line() << std::endl;
        }
    };

    ///
    /// \brief Generic parser implementation.
    ///
    template<typename item_type, typename parser_actions, typename parser_trace = no_parser_trace> class parser {
    private:
        /// \brief The parser tables
        const parser_tables* m_ParserTables;
        
        /// \brief True if this object owns the tables (and should destroy them)
        bool m_OwnsTables;
        
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
        explicit parser(const parser_tables& tables) 
        : m_ParserTables(new parser_tables(tables))
        , m_OwnsTables(true) { }
        
        /// \brief Creates a parser with a reference to the tables it should use.
        ///
        /// Set destroyTables to true if the parser should delete the tables when it is destructed.
        parser(const parser_tables* tables, bool destroyTables)
        : m_ParserTables(tables)
        , m_OwnsTables(destroyTables) {
        }

        /// \brief Creates a parser from the result of the specified builder class
        parser(const lalr_builder& builder, const weak_symbols* weakSymbols) 
        : m_ParserTables(new parser_tables(builder, weakSymbols))
        , m_OwnsTables(true) { }
        
        /// \brief Copy constructor
        parser(const parser& copyFrom) {
            // Only copy the tables if they are actually owned by the parser we're copying from
            // (For hard-coded parsers, the tables often aren't, so there's no need to copy)
            if (copyFrom.m_OwnsTables) {
                m_OwnsTables    = true;
                m_ParserTables  = new parser_tables(*copyFrom.m_ParserTables);
            } else {
                m_OwnsTables    = false;
                m_ParserTables  = copyFrom.m_ParserTables;
            }
        }

        /// \brief Destructor
        virtual ~parser() { 
            if (m_OwnsTables) {
                delete m_ParserTables;
            }
        }
        
    private:
        ///
        /// \brief Class representing a session with the parser
        ///
        /// A session consists of several states, and one stream of symbols from the lexer. Multiple parser
        /// states are used when implementing backtracking or GLR parsers. There is only one stream of
        /// symbols, however, which must be shared between all the states. This object manages that stream,
        /// ensuring that the symbols remain in memory when they're needed, and are removed once there are
        /// no more states referring to them.
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
            
            /// \brief The parser trace class
            parser_trace m_Trace;
            
        private:
            /// \brief States can't be assigned
            state& operator=(const state& noAssignment) { }
            
        private:
            /// \brief Constructs a new state, used by the parser
            state(const parser_tables* tables, int initialState, session* session);            
            
        public:
            /// \brief Creates a new parser state by copying an old one. Parser states can be run independently.
            state(const state& copyFrom);            
            
        public:
            /// \brief Destructor
            ~state();
            
        private:
            /// \brief Trims the lookahead in the sessions (removes any symbols that won't be visited again)
            inline void trim_lookahead();
            
        public:
            ///
            /// \brief Moves on a single symbol (ie, throws away the current lookahead)
            ///
            /// It is an error to call this without calling lookahead() at least once since the last call.
            ///
            inline void next();
            
            ///
            /// \brief Retrieves the current lookahead character
            ///
            inline const lexeme_container& look(int offset = 0);
            
        private:
            
            friend class standard_actions;
            
            ///
            /// \brief The actions performed when acting as a standard parser
            ///
            class standard_actions {
            private:
                /// \brief The trace for these actions
                parser_trace m_Trace;
                
            public:
                /// \brief Ignore action
                inline void ignore(state* state, const action* act, const lexeme_container& lookahead) {
                    // Just tell the trace
                    m_Trace.ignore(lookahead);
                }
                
                /// \brief Shift action
                inline void shift(state* state, const action* act, const lexeme_container& lookahead) {
                    // Push the next state, and the result of the shift action in the actions class
                    state->m_Stack.push(act->nextState, state->m_Session->m_Actions->shift(lookahead));
                    
                    // Tell the trace
                    m_Trace.shift(lookahead, act->nextState);
                }
                
                /// \brief Reduce action
                inline void reduce(state* state, const action* act, const parser_tables::reduce_rule& rule) {
                    // Tell the trace that this is happening
                    m_Trace.reduce(rule.identifier, rule.ruleId, rule.length);
                    
                    // Pop items from the stack, and create an item for them by calling the actions
                    reduce_list items;
                    for (int x=0; x < rule.length; ++x) {
                        items.push_back(state->m_Stack->item);
                        state->m_Stack.pop();
                    }
                    
                    // Fetch the state that's now on top of the stack
                    int gotoState = state->m_Stack->state;
                    
                    // Work out the lookahead position
                    const lexeme_container& la              = state->look();
                    const dfa::position*    lookaheadPos    = NULL;
                    
                    if (state->look().item()) {
                        // Still following symbols
                        lookaheadPos = &la->pos();
                    } else {
                        // At end: use an invalid position
                        // (Alternatively: modify the actions so it's possible to retrieve the current position)
                        static const dfa::position eofPos(-1, -1, -1);
                        lookaheadPos = &eofPos;
                    }
                    
                    // Get the goto action for this nonterminal
                    for (parser_tables::action_iterator gotoAct = state->m_Tables->find_nonterminal(gotoState, rule.identifier);
                         gotoAct != state->m_Tables->last_nonterminal_action(gotoState); 
                         ++gotoAct) {
                        if (gotoAct->type == lr_action::act_goto) {
                            // Found the goto action, perform the reduction
                            // (Note that this will perform the goto action for the next nonterminal if the nonterminal isn't in this state. This can only happen if the parser is in an invalid state)
                            state->m_Stack.push(gotoAct->nextState, state->m_Session->m_Actions->reduce(rule.identifier, rule.ruleId, items, *lookaheadPos));
                            
                            // Tell the trace about this
                            m_Trace.goto_state(gotoAct->nextState);
                            break;
                        }
                    }                    
                }
                
                /// \brief Sets the current state of the parser
                inline void set_state(state* state, int newState) {
                    state->m_Stack->state = newState;
                    
                    m_Trace.goto_state(newState);
                }
                
                /// \brief Returns the current lookahead symbol
                inline const lexeme_container& look(state* state) {
                    return state->look();
                }
                
                /// \brief Moves to the next lookahead symbol
                inline void next(state* state) {
                    state->next();
                }
                
                /// \brief The current state of the guard lookahead parser
                inline int current_state(state* state) const { return state->m_Stack->state; }
                
                /// \brief Returns -1 or the guard symbol matched by the lookahead with the specified initial guard state
                inline int check_guard(state* state, int initialState) {
                    int result = state->check_guard(initialState, 0);
                    
                    m_Trace.checked_guard(initialState, result);
                    
                    return result;
                }
                
                /// \brief The specified symbol has been rejected
                inline void reject(const lexeme_container& rejected) {
                    m_Trace.reject(rejected);
                }
                
                /// \brief Returns true if the specified terminal symbol can be reduced
                inline bool can_reduce(int terminal, parser_tables::action_iterator act, state* state) {
                    // Accepting or shifting actions always return true immediately
                    if (act->type == lr_action::act_shift || act->type == lr_action::act_shiftstrong || act->type == lr_action::act_accept) {
                        return true;
                    }

                    // Fake reduce using the action
                    std::stack<int> fakeStack;
                    int             stackPos = 0;

                    state->fake_reduce(act, stackPos, fakeStack, state->m_Stack);

                    // Do a can_reduce on what remains
                    return state->template can_reduce<terminal_fetcher>(terminal, stackPos, fakeStack, state->m_Stack);
                }
                
                /// \brief Returns true if the specified terminal symbol can be reduced
                inline bool can_reduce_nonterminal(int terminal, parser_tables::action_iterator act, state* state) {
                    // Accepting or shifting actions always return true immediately
                    if (act->type == lr_action::act_shift || act->type == lr_action::act_shiftstrong || act->type == lr_action::act_accept) {
                        return true;
                    }

                    // Fake reduce using the action
                    std::stack<int> fakeStack;
                    int             stackPos = 0;

                    state->fake_reduce(act, stackPos, fakeStack, state->m_Stack);

                    // Do a can_reduce on what remains
                    return state->template can_reduce<nonterminal_fetcher>(terminal, stackPos, fakeStack, state->m_Stack);
                }
            };
            
            friend class guard_actions;
            
            ///
            /// \brief A set of parser actions that are used to evaluate guards against the lookahead
            ///
            class guard_actions {
            private:
                /// \brief The offset for this action
                int             m_Offset;
                
                /// \brief The current stack for the guard symbol
                std::stack<int> m_Stack;
                
            public:
                /// \brief Creates 
                guard_actions(int initialState, int initialOffset)
                : m_Offset(initialOffset) {
                    m_Stack.push(initialState);
                }
                
            public:
                /// \brief Returns the current lookahead offset
                inline int offset() const { return m_Offset; }
                
                // \brief Moves on to the next symbol
                inline void next() { ++m_Offset; }
                
                /// \brief The current state of the guard lookahead parser
                inline int current_state(state* state) const { return m_Stack.top(); }
                
            public:
                /// \brief Ignore action
                inline void ignore(state* state, const action* act, const lexeme_container& lookahead) {
                    // Just tell the trace
                }

                /// \brief Shift action
                inline void shift(state* state, const action* act, const lexeme_container& lookahead) {
                    // Push the next state, and the result of the shift action in the actions class
                    m_Stack.push(act->nextState);
                }
                
                /// \brief Reduce action
                inline void reduce(state* state, const action* act, const parser_tables::reduce_rule& rule) {
                    // Pop items from the stack, and create an item for them by calling the actions
                    for (int x=0; x < rule.length; ++x) {
                        m_Stack.pop();
                    }
                    
                    // Fetch the state that's now on top of the stack
                    int gotoState = m_Stack.top();
                    
                    // Get the goto action for this nonterminal
                    for (parser_tables::action_iterator gotoAct = state->m_Tables->find_nonterminal(gotoState, rule.identifier);
                         gotoAct != state->m_Tables->last_nonterminal_action(gotoState); 
                         ++gotoAct) {
                        if (gotoAct->type == lr_action::act_goto) {
                            // Found the goto action, perform the reduction
                            // (Note that this will perform the goto action for the next nonterminal if the nonterminal isn't in this state. This can only happen if the parser is in an invalid state)
                            m_Stack.push(gotoAct->nextState);
                            break;
                        }
                    }                    
                }
                
                /// \brief Sets the current state of the parser
                inline void set_state(state* state, int newState) {
                    m_Stack.top() = newState;
                }
                
                /// \brief Returns -1 or the guard symbol matched by the lookahead with the specified initial guard state
                inline int check_guard(state* state, int initialState) {
                    return state->check_guard(initialState, m_Offset);
                }
                
                /// \brief The specified symbol has been rejected
                inline void reject(const lexeme_container& rejected) {
                }
                
                /// \brief Returns true if the specified terminal symbol can be reduced
                bool can_reduce(int terminal, parser_tables::action_iterator act, state* state) {
                    // Accepting or shifting actions always return true immediately
                    if (act->type == lr_action::act_shift || act->type == lr_action::act_shiftstrong || act->type == lr_action::act_accept) {
                        return true;
                    }

                    // Fake reduce using the action
                    std::stack<int> fakeStack   = m_Stack;
                    int             stackPos    = 0;

                    state->fake_reduce(act, stackPos, fakeStack, state->m_Stack);

                    // Do a can_reduce on the result
                    return state->template can_reduce<terminal_fetcher>(terminal, stackPos, fakeStack, state->m_Stack);
                }
                
                /// \brief Returns true if the specified terminal symbol can be reduced
                bool can_reduce_nonterminal(int nonterminal, parser_tables::action_iterator act, state* state) {
                    // Accepting or shifting actions always return true immediately
                    if (act->type == lr_action::act_shift || act->type == lr_action::act_shiftstrong || act->type == lr_action::act_accept) {
                        return true;
                    }

                    // Fake reduce using the action
                    std::stack<int> fakeStack   = m_Stack;
                    int             stackPos    = 0;

                    state->fake_reduce(act, stackPos, fakeStack, state->m_Stack);

                    // Do a can_reduce on the result
                    return state->template can_reduce<nonterminal_fetcher>(nonterminal, stackPos, fakeStack, state->m_Stack);
                }
            };

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
            template<class actions> inline bool perform_generic(const lexeme_container& lookahead, const action* act, actions& actDelegate);

            ///
            /// \brief Checks the lookahead against the guard condition which starts at the specified initial state
            ///
            /// This runs the parser forward from the specified state. If a 'end of guard' symbol is encountered and
            /// can produce an accepting state, then this will return the ID of the guard symbol that was accepted.
            /// If no accepting state is reached, this will return a negative value (generally -1)
            ///
            int check_guard(int initialState, int initialOffset);
            
        public:
            ///
            /// \brief Performs the specified action
            ///
            /// No check is made to see if the action is valid: it is just performed. Returns true if the lookahead
            /// should be updated to be the next symbol.
            ///
            inline bool perform(const lexeme_container& lookahead, const action* act) {
                // Call perform_generic with the standard actions
                standard_actions standard;
                return perform_generic(lookahead, act, standard);
            }
            
        private:
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
            
            /// \brief Fakes up a reduce action during can_reduce testing. act must be a reduce action
            inline void fake_reduce(parser_tables::action_iterator act, int& stackPos, std::stack<int>& pushed, const stack& underlyingStack);
            
            /// \brief Returns true if a reduction of the specified lexeme will result in it being shifted
            template<class symbol_fetcher> bool can_reduce(int symbol, int stackPos, std::stack<int> pushed, const stack& underlyingStack);
            
        public:
            /// \brief Returns true if a reduction of the specified lexeme will result in it being shifted
            ///
            /// In states with guards in their lookahead, or states with reduce/reduce conflicts that would
            /// be resolved by a LR(1) parser, this will disambiguate the grammar (making it possible to choose
            /// only the action that allows the parser to continue)
            inline bool can_reduce(const lexeme_container& lexeme) {
                return can_reduce<terminal_fetcher>(lexeme->matched(), 0, std::stack<int>());
            }

            /// \brief Returns true if a reduction of the specified terminal symbol will result in it being shifted
            ///
            /// In states with guards in their lookahead, or states with reduce/reduce conflicts that would
            /// be resolved by a LR(1) parser, this will disambiguate the grammar (making it possible to choose
            /// only the action that allows the parser to continue)
            inline bool can_reduce(int terminalId) {
                return can_reduce<terminal_fetcher>(terminalId, 0, std::stack<int>(), m_Stack);
            }

            /// \brief Returns true if a reduction of the lookahead will result in it being shifted
            inline bool can_reduce() {
                return can_reduce(look());
            }
            
        private:
            /// \brief As for can_reduce, but with a fake nonterminal lookahead value
            inline bool can_reduce_nonterminal(int nt) {
                return can_reduce<nonterminal_fetcher>(nt, 0, std::stack<int>(), m_Stack);
            }
            
        public:
            typedef parser_result::result result;

        private:
            /// \brief Performs a single parsing action, and returns the result
            ///
            /// This version takes several parameters: the current lookahead token, the ID of the symbol and whether or not it's
            /// a terminal symbol, and the range of actions that might apply to this particular symbol.
            template<class actions> inline result process_generic(actions& actDelegate, const lexeme_container& la, 
                                                                  int symbol, bool isTerminal,
                                                                  parser_tables::action_iterator& act, 
                                                                  parser_tables::action_iterator& end);

            /// \brief Performs a single parsing action, and returns the result
            template<class actions> inline result process_generic(actions& actDelegate);
            
            /// \brief Updates the state according to the actions required by a guard symbol
            ///
            /// This will return true if the guard symbol could be successfully processed, or false if it could not.
            /// Specifically, if a guard symbol generates a reduce action which does not result in it eventually being shifted,
            /// this will return false so that the parser can try the other actions associated with the specified symbol.
            ///
            /// Practical experience indicates that guards are often used in situations that are not quite LALR(1); checking
            /// whether or not reductions will be successful makes them easier to use as they will not cause spurious reductions
            /// in situations where it's not appropriate.
            template<class actions> bool process_guard(actions& actDelegate, const lexeme_container& la, int guardSymbol);
            
        public:
            /// \brief Performs a single parsing action, and returns the result
            inline result process() {
                standard_actions actions;
                return process_generic(actions);
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
        
        /// \brief Retrieves the tables for this parser
        inline const parser_tables& get_tables() const { return *m_ParserTables; }
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
        inline int reduce(int nonterminal, int rule, const parser<int, simple_parser_actions>::reduce_list& reduce, const dfa::position& lookaheadPosition) {
            return 0;
        }
    };
    
    /// \brief Simple parser, can be used to test if a language is accepted by a parser (but not much else)
    typedef parser<int, simple_parser_actions> simple_parser;
}

#include "parser_state.h"

#endif
