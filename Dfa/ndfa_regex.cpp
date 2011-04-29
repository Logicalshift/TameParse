//
//  ndfa_regex.cpp
//  Parse
//
//  Created by Andrew Hunter on 27/04/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "ndfa_regex.h"
#include "symbol_set.h"

using namespace std;
using namespace dfa;

/// \brief Constructs an empty NDFA
ndfa_regex::ndfa_regex() {
}

/// \brief Conversion constructor
ndfa_regex::ndfa_regex(const ndfa& copyFrom)
: ndfa(copyFrom) { 
}

/// \brief Copy constructor
ndfa_regex::ndfa_regex(const ndfa_regex& copyFrom)
: ndfa(copyFrom) {
}

/// \brief Converts a string to a symbol_string
symbol_string ndfa_regex::convert(string source) {
    // Create the result
    symbol_string result;
    result.reserve(source.size());
    
    // Fill it in, treating the characters as unsigned
    for (string::iterator it=source.begin(); it != source.end(); it++) {
        result += (int)(unsigned char)*it;
    }
    
    // This is the result
    return result;
}

/// \brief Converts a wstring to a symbol_string
symbol_string ndfa_regex::convert(wstring source) {
    // Create the result
    symbol_string result;
    result.reserve(source.size());
    
    // Fill it in, treating the characters as unsigned
    for (wstring::iterator it=source.begin(); it != source.end(); it++) {
        result += (int)(unsigned wchar_t)*it;
    }
    
    // This is the result
    return result;    
}

/// \brief Converts a null-terminated string to a symbol_string
symbol_string ndfa_regex::convert(char* source) {
    // Create the result
    symbol_string result;
    if (!source) return result;
    
    // Fill it in, treating the characters as unsigned
    for (char* pos = source; *pos != 0; pos++) {
        result += (int)(unsigned char)*pos;
    }
    
    // This is the result
    return result;        
}

/// \brief Converts a null-terminated to a symbol_string
symbol_string ndfa_regex::convert(wchar_t* source) {
    // Create the result
    symbol_string result;
    if (!source) return result;
    
    // Fill it in, treating the characters as unsigned
    for (wchar_t* pos = source; *pos != 0; pos++) {
        result += (int)(unsigned wchar_t)*pos;
    }
    
    // This is the result
    return result;        
}

/// \brief Compiles a regular expression starting at the specified state, returning the final state
int ndfa_regex::add_regex(int initialState, const symbol_string& regex) {
    // Can't really do anythign if the initial state is invalid
    if (initialState < 0 || initialState >= count_states()) return initialState;
    
    // Create a constructor in the initial state
    constructor cons = get_cons();
    cons.goto_state(get_state(initialState));
    
    // Create an epsilon transform to an initial state for this regex
    cons >> epsilon();
    
    // Compile until we reach the end of the regex
    symbol_string::const_iterator pos = regex.begin();
    symbol_string::const_iterator end = regex.end();
    
    while (pos != end) {
        // Compile the next character
        compile(pos, end, cons);
        
        // Move on
        pos++;
    }
    
    // Compile the end of the expression
    compile(end, end, cons);
    
    // Return the final state
    return ((state)cons).identifier();
}

///
/// \brief Compiles a single symbol from a regular expression
///
/// Subclasses can override this to extend the grammar accepted by the regular expression.
/// This class should update the supplied iterator and NDFA constructor object with the position of the next item.
///
void ndfa_regex::compile(symbol_string::const_iterator& pos, const symbol_string::const_iterator& end, constructor& cons) {
    // Don't process the final character, but rejoin if there's an or section in effect
    if (pos == end) {
        cons.rejoin();
        return;
    }
    
    // Default regex compiler
    switch (*pos) {
        case '(':           // Group start
            // Start a new bracketed section by pushing the current state
            cons.push();
            break;
            
        case ')':           // Group end
            // Discard the entry on top of the stack
            cons.pop();
            break;
            
        case '|':           // Or
            // Or with the most recent state
            cons.begin_or();
            break;
            
        case '*':           // 0 or more of previous
        {
            // Get the previous and current state
            const state& previous = cons.previous_state();
            const state& current  = cons.current_state();
            
            // Can skip from previous to current (so this subexpression can be skipped entirely)
            cons.goto_state(previous);
            cons >> current >> epsilon();
            
            // Can skip from current to previous (so this expression will repeat)
            cons.goto_state(current);
            cons >> previous >> epsilon();
            
            // Final state is the current state (and preserve the 'previous' state)
            cons.goto_state(current, previous);
            break;
        }

        case '+':           // 1 or more of previous
        {
            // Get the previous and current state
            const state& previous = cons.previous_state();
            const state& current  = cons.current_state();
            
            // Can skip from current to previous (so this expression will repeat)
            cons.goto_state(current);
            cons >> previous >> epsilon();
            
            // Final state is the current state (and preserve the 'previous' state)
            cons.goto_state(current, previous);
            break;
        }
            
        case '?':           // 0 or 1 of previous
        {
            // Get the previous and current state
            const state& previous = cons.previous_state();
            const state& current  = cons.current_state();
            
            // Can skip from previous to current (so this subexpression can be skipped entirely)
            cons.goto_state(previous);
            cons >> current >> epsilon();
            
            // Final state is the current state (and preserve the 'previous' state)
            cons.goto_state(current, previous);
            break;
        }

        case '.':
            // Any symbol
            cons >> symbol_set::symbol_range(0, 0x7fffffff);
            // cons.rejoin();
            break;
            
        default:
            // Most characters just go through unchanged
            cons >> *pos;
            // cons.rejoin();
            break;
    }
}
