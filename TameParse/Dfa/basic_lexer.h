//
//  basic_lexer.h
//  Parse
//
//  Created by Andrew Hunter on 28/04/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef _DFA_BASIC_LEXER_H
#define _DFA_BASIC_LEXER_H

#include <iostream>
#include <vector>

#include "TameParse/Dfa/symbol_set.h"
#include "TameParse/Dfa/ndfa_regex.h"
#include "TameParse/Dfa/state_machine.h"
#include "TameParse/Dfa/lexeme.h"
#include "TameParse/Dfa/position.h"

namespace dfa {
    ///
    /// \brief Abstract base class that represents a session with a lexer
    ///
    class lexeme_stream {
    public:
        /// \brief Destructor
        virtual ~lexeme_stream();
        
        /// \brief Fills in the contents of the specified pointer with the next lexeme (or NULL if the end of input has been reached)
        ///
        /// The caller needs to delete the resulting lexeme object
        virtual lexeme_stream& operator>>(lexeme*& result) = 0;
        
        /// \brief Sets the initial state to be used by the next run through of the state machine
        ///
        /// Might not do anything, the meaning of the 'initialState' is defined by the implementation of the lexer. However, the default initial 
        /// state is always 0.
        virtual void set_initial_state(int initialState);
    };
    
    ///
    /// \brief Abstract base class representing a source of symbols for a lexer
    ///
    class lexer_symbol_stream {
    public:
        /// \brief Destructor
        virtual ~lexer_symbol_stream();
        
        /// \brief Reads the next symbol from this stream
        ///
        /// The result should be symbol_set::end_of_input when the end of input is reached 
        virtual lexer_symbol_stream& operator>>(int& result) = 0;
    };
    
    ///
    /// \brief Abstract base class that runs a state machine to turn the contents of a stream into a series of lexemes
    ///
    class basic_lexer {
    protected:
        /// \brief A symbol stream that reads from a basic_istream with the specified traits
        template<typename stream, typename Char> class stream_stream : public lexer_symbol_stream {
        private:
            /// The stream that this refers to
            stream& m_Stream; 
            
        public:
            stream_stream(stream& str)
            : m_Stream(str) {
            }
            
            /// \brief Reads the next symbol from this stream
            virtual lexer_symbol_stream& operator>>(int& result) {
                Char next;

                m_Stream.get(next);
                if (!m_Stream.good()) {
                    result = symbol_set::end_of_input;
                } else {
                    result = (int)(unsigned)next;
                }
                return *this;
            }
        };
        
    public:
        /// \brief Destructor
        virtual ~basic_lexer();
        
        ///
        /// \brief Creates a new lexer to process the specified symbol stream
        ///
        /// The lexeme_stream should take ownership of the supplied lexer_symbol_stream and delete it once it has finished with it
        ///
        virtual lexeme_stream* create_stream(lexer_symbol_stream* stream) const = 0;
        
        /// \brief Estimated size in bytes of this lexer
        virtual size_t size() const = 0;
        
        /// \brief Creates a new lexer that will read from the specified stream (which must not be destroyed while the lexer is in use)
        template<typename char_type, typename traits> inline lexeme_stream* create_stream_from(std::basic_istream<char_type, traits>& input) const {
            return create_stream(new stream_stream<std::basic_istream<char_type, traits>, char_type>(input));
        }
        
        /// \brief Creates a new lexer from a custom type that supports the get and good operators for a particular character type
        template<typename char_type, typename custom_stream_alike> inline lexeme_stream* create_stream_from(custom_stream_alike& input) const {
            return create_stream(new stream_stream<custom_stream_alike, char_type>(input));
        }
    };
    
    ///
    /// \brief Class that describes a lexer built from a DFA. 
    /// 
    /// The Char template supplies the expected character type for the DFA (the lexer will still accept any symbol type, so it's safe to leave 
    /// it as a wchar_t in most cases)
    ///
    /// firstState indicates the state that the lexer starts in before it has received any input. newlineState indicates the state the lexer moves into
    /// if the last lexeme ends with a newline character.
    ///
    template<typename state_machine, int firstState = 0, int newlineState = 0, bool deleteTables = true, typename state_machine_ref = const state_machine&> class dfa_lexer_base : public basic_lexer {
    private:
        /// \brief The state machine for this lexer
        ///
        /// (More specific characters produce a state machine that runs faster and requires less memory)
        state_machine m_StateMachine;
        
        /// \brief Number of states in this lexer
        int m_MaxState;
        
        /// \brief Array containing a list of possible accept actions for accepting states
        const int* m_Accept;
        
        dfa_lexer_base& operator=(const dfa_lexer_base& copyFrom);
        dfa_lexer_base(const dfa_lexer_base& copyFrom);
        
    private:
        /// \brief Fills in an entry in the accept array
        template<typename iterator> inline void fill_accept(int& target, iterator begin, iterator end) {
            // Default is -1
            if (begin == end) {
                target = -1;
                return;
            }
            
            // Use the highest ranked action
            accept_action* highest = *begin;
            for (iterator it = begin; it != end; ++it) {
                if ((*highest) < **it) {
                    highest = *it;
                }
            }
            
            // Use the target symbol of the highest ranked action
            target = highest->symbol();
        }
        
    public:
        /// \brief Constructs a lexer from a DFA
        ///
        /// A DFA is an NDFA which has been transformed by to_ndfa_with_unique_symbols() and to_dfa(), in that order.
        dfa_lexer_base(const ndfa& dfa)
        : m_StateMachine(dfa)
        , m_MaxState(dfa.count_states()) {
            // Allocate space for the accepting states
            int* accept = new int[m_MaxState];
            m_Accept    = accept;
            
            // Create accepting action lists for each accepting state
            for (int stateId=0; stateId<m_MaxState; ++stateId) {
                fill_accept(accept[stateId], dfa.actions_for_state(stateId).begin(), dfa.actions_for_state(stateId).end());
            }
        }

        /// \brief Constructs a lexer from a state machine
        dfa_lexer_base(state_machine_ref stateMachine, int maxState, const int* accept)
        : m_StateMachine(stateMachine)
        , m_MaxState(maxState)
        , m_Accept(accept) {
        }

        /// \brief Destructor
        virtual ~dfa_lexer_base() {
            // Delete the accepting states
            if (deleteTables && m_Accept) {
                delete[] m_Accept;
            }
        }
        
    private:
        ///
        /// \brief A lexeme stream that reads from a DFA
        ///
        class dfa_stream : public lexeme_stream {
        private:
            /// \brief The state machine for this lexer
            state_machine_ref m_StateMachine;
            
            /// \brief Array of symbols that are accepted in each state
            const int* m_Accept;
            
            /// \brief The stream that this will read symbols from
            lexer_symbol_stream* m_Stream;
            
            /// \brief The position tracker
            position_tracker m_Position;
            
            /// \brief Type of the buffer
            typedef std::vector<int> buffer;
            
            /// \brief Buffer of characters waiting to be processed by this stream
            buffer m_Buffer;
            
            /// \brief The initial state to use before retrieving the next lexeme
            int m_InitialState;
            
        private:
            
        public:
            /// \brief Creates a new stream that works with the specified state machine, list of accepting actions and symbol stream
            dfa_stream(state_machine_ref sm, const int* acc, lexer_symbol_stream* str)
            : m_StateMachine(sm)
            , m_Accept(acc)
            , m_Stream(str)
            , m_InitialState(firstState) {
            }
            
            /// \brief Destructor
            virtual ~dfa_stream() {
                delete m_Stream;
            }
            
            /// \brief Sets the initial state to be used by the next run through of the state machine
            ///
            /// Might not do anything, the meaning of the 'initialState' is defined by the implementation of the lexer. However, the default initial 
            /// state is always 0.
            virtual void set_initial_state(int initialState) {
                m_InitialState = initialState;
            }

            /// \brief Fills in the contents of the specified pointer with the next lexeme (or NULL if the end of input has been reached)
            virtual lexeme_stream& operator>>(lexeme*& result) {
                // Create the initial lexer state
                int     state           = m_InitialState;
                int     pos             = 0;
                int     acceptSymbol    = -1;
                int     acceptPos       = -1;
                bool    atEof           = false;
                
                for (;;) {
                    // Add to the end of the buffer if it is empty
                    if (pos == (int) m_Buffer.size()) {
                        // Get the next symbol
                        int nextSym;
                        (*m_Stream) >> nextSym;
                        
                        // Stop once we reach the end of file marker
                        if (nextSym == symbol_set::end_of_input) {
                            atEof = true;
                            break;
                        }
                        
                        // Push this as the next symbol
                        m_Buffer.push_back(nextSym);
                    }
                    
                    // Get the current symbol
                    int curSym = m_Buffer[pos];
                    
                    // The position moves on here
                    ++pos;
                    
                    // Run the state machine (use the faster 'unsafe' mode, we check the state later ourselves)
                    state = m_StateMachine.run_unsafe(state, curSym);
                    
                    // Stop processing if the state machine rejects this character (this is why we can use the unsafe mode, at least assuming the state machine doesn't transition to a state that's too high)
                    if (state < 0) break;
                    
                    // If this is an accepting state, mark it as such
                    if (m_Accept[state] >= 0) {
                        acceptPos       = pos;
                        acceptSymbol    = m_Accept[state];
                    }
                }
                
                // If the buffer is empty, then the result is always NULL 
                if (m_Buffer.empty()) {
                    result = NULL;
                    return *this;
                }
                
                // If the accept position is -1 or 0, change it to 1 so we reject at least one character
                if (acceptPos <= 0) acceptPos = 1;
                
                // Create the lexeme for this item
                result = new lexeme(m_Buffer.begin(), m_Buffer.begin() + acceptPos, m_Position.current_position(), acceptSymbol, acceptPos);
                
                // Choose the new initial state
                m_InitialState = 0;
                if (newlineState != m_InitialState) {
                    // Use the newline state if the last character in the lexeme is a newline
                    int lastChar = m_Buffer[acceptPos-1];
                    if (lastChar == 0x0a || lastChar == 0x0b || lastChar == 0x0c || lastChar == 0x0d || lastChar == 0x85 || lastChar == 0x2028 || lastChar == 0x2029) {
                        m_InitialState = newlineState;
                    }
                }
                
                // Update the position to point after the accepted lexeme
                m_Position.update_position(m_Buffer.begin(), m_Buffer.begin() + acceptPos);
                
                // Delete the accepted symbols from the buffer
                m_Buffer.erase(m_Buffer.begin(), m_Buffer.begin() + acceptPos);
                
                // Done
                return *this;
            }
        };
        
    public:
        ///
        /// \brief Creates a new lexer to process the specified symbol stream
        ///
        /// The lexeme_stream should take ownership of the supplied lexer_symbol_stream and delete it once it has finished with it
        ///
        virtual lexeme_stream* create_stream(lexer_symbol_stream* stream) const {
            if (!stream) return NULL;
            return new dfa_stream(m_StateMachine, m_Accept, stream);
        }
        
        /// \brief Estimated size in bytes of this lexer
        virtual size_t size() const {
            return m_StateMachine.size();
        }
    };
    
    ///
    /// \brief A lexer that is built from a DFA
    ///
    /// Use this class rather than dfa_lexer_base for dynamically creating lexers. dfa_lexer_base can be used to create lexers
    /// with custom state machines, which can be useful for running DFAs using custom table types.
    ///
    template<typename char_type, typename state_machine_row = state_machine_flat_table, int firstState = 0, int newlineState = 0> class dfa_lexer : public dfa_lexer_base<state_machine<char_type, state_machine_row>, firstState, newlineState> {
    public:
        typedef dfa_lexer_base<state_machine<char_type, state_machine_row>, firstState, newlineState> base;
        
        /// \brief Constructs a lexer from a DFA
        ///
        /// A DFA is an NDFA which has been transformed by to_ndfa_with_unique_symbols() and to_dfa(), in that order.
        inline dfa_lexer(const ndfa& dfa) : base(dfa) {
        }
    };
}

#endif
