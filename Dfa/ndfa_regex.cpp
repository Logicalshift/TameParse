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
ndfa_regex::ndfa_regex()
: m_ConstructSurrogates(true) {
}

/// \brief Conversion constructor
ndfa_regex::ndfa_regex(const ndfa& copyFrom)
: ndfa(copyFrom)
, m_ConstructSurrogates(true) { 
}

/// \brief Copy constructor
ndfa_regex::ndfa_regex(const ndfa_regex& copyFrom)
: ndfa(copyFrom)
, m_ConstructSurrogates(copyFrom.m_ConstructSurrogates)
, m_ExpressionMap(copyFrom.m_ExpressionMap) {
}

/// \brief Converts a string to a symbol_string
symbol_string ndfa_regex::convert(const string& source) {
    // Create the result
    symbol_string result;
    result.reserve(source.size());
    
    // Fill it in, treating the characters as unsigned
    for (string::const_iterator it=source.begin(); it != source.end(); it++) {
        result += (int)(unsigned char)*it;
    }
    
    // This is the result
    return result;
}

/// \brief Converts a wstring to a symbol_string
symbol_string ndfa_regex::convert(const wstring& source) {
    // Create the result
    symbol_string result;
    result.reserve(source.size());
    
    // Fill it in, treating the characters as unsigned
    for (wstring::const_iterator it=source.begin(); it != source.end(); it++) {
        result += (int)(unsigned)(wchar_t)*it;
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
        result += (int)(unsigned)(wchar_t)*pos;
    }
    
    // This is the result
    return result;        
}

/// \brief Compiles an NDFA that matches a literal string starting at the specified state, returning the final state
int ndfa_regex::add_literal(int initialState, const symbol_string& literal) {
    // Can't really do anything if the initial state is invalid
    if (initialState < 0 || initialState >= count_states()) return initialState;
    
    // Create a constructor in the initial state
    builder cons = get_cons();
    cons.set_generate_surrogates(m_ConstructSurrogates);
    cons.goto_state(get_state(initialState));
    
    // Compile as a straight string
    for (symbol_string::const_iterator pos = literal.begin(); pos != literal.end(); pos++) {
        // Add the next string
        cons >> *pos;
    }
    
    // Return the final state
    return ((state)cons).identifier();
}

/// \brief Compiles a regular expression starting at the specified state, returning the final state
int ndfa_regex::add_regex(int initialState, const symbol_string& regex) {
    // Can't really do anything if the initial state is invalid
    if (initialState < 0 || initialState >= count_states()) return initialState;
    
    // Create a constructor in the initial state
    builder cons = get_cons();
    cons.set_generate_surrogates(m_ConstructSurrogates);
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
        if (pos != end) {
            pos++;
        }
    }
    
    // Compile the end of the expression
    compile(end, end, cons);
    
    // Return the final state
    return ((state)cons).identifier();
}

/// \brief Defines a new expression
///
/// When a regular expression contains {name}, it will be substituted for the supplied value. Subclasses can 
/// change this behaviour by overriding the compile_expression funciton.
void ndfa_regex::define_expression(const symbol_string& name, const symbol_string& value) {
    m_ExpressionMap[name] = value;
}

///
/// \brief Compiles a single symbol from a regular expression
///
/// Subclasses can override this to extend the grammar accepted by the regular expression.
/// This class should update the supplied iterator and NDFA constructor object with the position of the next item.
///
void ndfa_regex::compile(symbol_string::const_iterator& pos, const symbol_string::const_iterator& end, builder& cons) {
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
            break;

            
        case '[':
        {
            // Group of symbols
            pos++;
            if (pos == end) return;
            
            // If the group begins '^', then we take the negative of this group
            bool negate = false;
            if (*pos == '^') {
                negate = true;
                pos++;
                if (pos == end) return;
            }
            
            // Go through the symbols until we encounter a ']'
            symbol_set syms;
            
            while (pos != end && *pos != ']') {
                // Get the symbol at this position
                int initialSym = symbol_for_sequence(pos, end);
                if (pos == end) break;
                
                // Check the next symbol to see if we've got a range
                pos++;
                if (pos == end) break;
                
                if (*pos == '-') {
                    // Got a range of symbols
                    pos++;
                    if (pos == end) break;
                    
                    // Get the final symbol in the range
                    int finalSym = symbol_for_sequence(pos, end);
                    if (pos == end) break;
                    pos++;
                    
                    // Ranges are inclusive
                    syms |= range<int>(initialSym, finalSym+1);
                } else {
                    // Just a single symbol
                    syms |= initialSym;
                }
            }
            
            // Negate the set if necessary
            if (negate) {
                syms = ~syms;
            }
            
            // Append the transition
            cons >> syms;
            break;
        }

        case '\\':
            // Quoted symbol (same as default behaviour, explicit case is here to make it clear overriding this is incorrect)
        default:
            // Most characters just go through unchanged
            cons >> symbol_for_sequence(pos, end);
            break;
    }
}

/// \brief Returns a hex number with maxDigits from the character after the current position
static int hex(symbol_string::const_iterator& pos, const symbol_string::const_iterator& end, int maxDigits) {
    int result = 0;
    
    for (int x=0; x<maxDigits; x++) {
        // Get the next position
        symbol_string::const_iterator next = pos;
        next++;
        
        // Check that this is a hex character
        if (next == end) break;
        
        int chr = *next;
        if ((chr < '0' || chr > '9') && (chr < 'a' || chr > 'f') && (chr < 'A' || chr > 'F')) break;
        
        // Get the value of this character
        int value = 0;
        if (chr <= '9')         value = chr - '0';
        else if (chr <= 'F')    value = chr - 'A' + 10;
        else if (chr <= 'f')    value = chr - 'a' + 10;
        
        // Update the result
        result *= 16;
        result += value;
        
        // Update the position
        pos = next;
    }
    
    return result;
}

/// \brief Returns a octal number with maxDigits from the character after the current position
static int oct(symbol_string::const_iterator& pos, const symbol_string::const_iterator& end, int maxDigits) {
    int result = 0;
    
    for (int x=0; x<maxDigits; x++) {
        // Get the next position
        symbol_string::const_iterator next = pos;
        next++;
        
        // Check that this is a hex character
        if (next == end) break;
        
        int chr = *next;
        if ((chr < '0' || chr > '7')) break;
        
        // Get the value of this character
        int value = chr - '0';
        
        // Update the result
        result *= 8;
        result += value;
        
        // Update the position
        pos = next;
    }
    
    return result;
}

///
/// \brief Returns the symbol at the specified position. Updates pos to point to the last character that makes up this symbol
///
/// This translates quoted symbols like '\n' to their symbolic equivalent (0x0a in this case). Subclasses can override this to
/// add more quoting behaviour to the default.
///
int ndfa_regex::symbol_for_sequence(symbol_string::const_iterator& pos, const symbol_string::const_iterator& end) {
    // Shouldn't happen, but check anyway
    if (pos == end) return 0;
    
    // Quoted symbols begin with '\': just return the symbol if this character isn't present
    if (*pos != '\\') return *pos;
    
    // Move on to the character being quoted
    pos++;

    // Just use '\' if we fall off the end of the expression
    if (pos == end) return '\\';
    
    switch (*pos) {
            // Single-character escapes
        case 'a':   return 0x7;
        case 'e':   return 0x1b;
        case 'n':   return 0xa;
        case 'r':   return 0xd;
        case 'f':   return 0xc;
        case 't':   return 0x9;
            
            // Multi-character escapes
        case 'u':
            // Unicode character (0-4 hexadecimal digits)
            return hex(pos, end, 4);
            
        case 'x':
            // Latin-1 character (0-2 hexadecimal digits)
            return hex(pos, end, 2);
            
        case 'o':
            // Octal sequence (supporting unicode)
            return oct(pos, end, 6);
            
        case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
            // Octal sequence (latin-1)
            pos--;
            return oct(pos, end, 3);
            
        default:
            // Default behaviour is to pass the 'quoted' character through untouched (so you can do things like \.)
            return *pos;
    }
}

///
/// \brief Compiles the value of a {} expression
///
/// The '{}' operator in a regular expression generally indicates a subexpression macro. The default implementation
/// of this will first look in the expression map for a regular expression, and compile that at the current position
/// if it finds one. If it does not find an expression, it will look up the expression as a unicode character range.
///
/// Returns true if the expression was successfully compiled.
///
bool ndfa_regex::compile_expression(const symbol_string& expression, builder& cons) {
    // Look up the expression in the expression map
    map<symbol_string, symbol_string>::const_iterator found = m_ExpressionMap.find(expression);

    // If found, then compile this expression
    if (found != m_ExpressionMap.end()) {
        // Add this regular expression, starting at the current state
        int finalState = add_regex(cons.current_state().identifier(), expression);

        // Set the final state as the new 'current' state
        cons.goto_state(get_state(finalState));
        return true;
    }

    // TODO: look up unicode character sequences

    // Fail
    return false;
}
