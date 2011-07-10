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
    /// Got to love these nice concise C++ declarations. Forward declarations and templates are fun. Ish.
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
}

#endif
