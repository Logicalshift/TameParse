//
//  lexer_compiler.cpp
//  Parse
//
//  Created by Andrew Hunter on 21/08/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

// TODO: it'd be nice to have a way of designing this so it's not dependent on the language_compiler stage
//       (we don't do this at the moment to keep things reasonably simple, there's a lot of dependencies that means that DI
//       wouldn't really fix the problem, and would just create a new 'giant constructor of doom' problem)

#include <sstream>
#include "Compiler/lexer_compiler.h"

using namespace std;
using namespace dfa;
using namespace contextfree;
using namespace lr;
using namespace compiler;

/// \brief Creates a new lexer compiler
///
/// The compiler will not 'own' the objects passed in to this constructor; however, they must have a lifespan
/// that is at least as long as the compiler itself (it's safe to call the destructor but no other call if they
/// have been destroyed)
lexer_compiler::lexer_compiler(console_container& console, const std::wstring& filename, language_compiler* languageCompiler)
: compilation_stage(console, filename)
, m_Language(languageCompiler)
, m_Dfa(NULL)
, m_Lexer(NULL) {
}

/// \brief Destroys the lexer compiler
lexer_compiler::~lexer_compiler() {
    // Destroy the DFA if it exists
    if (m_Dfa) {
        delete m_Dfa;
    }
    
    if (m_Lexer) {
        delete m_Lexer;
    }
}

/// \brief Compiles the lexer
void lexer_compiler::compile() {
    // Grab the input
    ndfa*                   ndfa            = m_Language->ndfa();
    terminal_dictionary*    terminals       = m_Language->terminals();
    set<int>*               weakSymbolIds   = m_Language->weak_symbols();
    
    // Sanity check
    if (!ndfa || !terminals || !weakSymbolIds) {
        cons().report_error(error(error::sev_bug, filename(), L"BUG_LEXER_BAD_PARAMETERS", L"Missing input for the lexer stage", position(-1, -1, -1)));
        return;
    }
    
    // Output a staging message
    cons().verbose_stream() << L"  = Constructing final lexer" << endl;
    
    // Compile the NDFA to a NDFA without overlapping symbol sets
    dfa::ndfa* stage1 = ndfa->to_ndfa_with_unique_symbols();
    
    if (!stage1) {
        cons().report_error(error(error::sev_bug, filename(), L"BUG_DFA_FAILED_TO_CONVERT", L"Failed to create an NDFA with unique symbols", position(-1, -1, -1)));
        return;
    }
    
    // Write some information about the first stage
    cons().verbose_stream() << L"    Initial number of character sets:       " << ndfa->symbols().count_sets() << endl;
    cons().verbose_stream() << L"    Final number of character sets:         " << stage1->symbols().count_sets() << endl;
    
    // Compile the NDFA to a DFA
    dfa::ndfa* stage2 = stage1->to_dfa();
    delete stage1;
    
    if (!stage2) {
        cons().report_error(error(error::sev_bug, filename(), L"BUG_DFA_FAILED_TO_COMPILE", L"Failed to compile DFA", position(-1, -1, -1)));
        return;
    }
    
    // TODO: need a way to find out where terminals were defined to display messages about them
    // TODO: identify any terminals that are always replaced by other terminals (warning)
    // TODO: also identify any terminals that clash with terminals at the same level (warning)
    
    // Build up the weak symbols set if there are any
    if (weakSymbolIds->size() > 0) {
        // Build up the weak symbol set as a series of items
        item_set weakSymSet;
        
        // Count how many symbols there were initially
        int initialSymCount = terminals->count_symbols();
        
        // Iterate through the symbol IDs
        for (set<int>::iterator weakSymId = weakSymbolIds->begin(); weakSymId != weakSymbolIds->end(); weakSymId++) {
            weakSymSet.insert(item_container(new terminal(*weakSymId), true));
        }
        
        // Add these symbols to the weak symbols object
        m_WeakSymbols.add_symbols(*stage2, weakSymSet, *terminals);
        
        // Display how many new terminal symbols were added
        int finalSymCount = terminals->count_symbols();
        
        cons().verbose_stream() << L"    Number of extra weak symbols:           " << finalSymCount - initialSymCount << endl;
    }
    
    // Compact the resulting DFA
    cons().verbose_stream() << L"    Number of states in the lexer DFA:      " << stage2->count_states() << endl;
    dfa::ndfa* dfa = stage2->to_compact_dfa();
    delete stage2;
    
    // Write some information about the DFA we just produced
    cons().verbose_stream() << L"    Number of states in the compacted DFA:  " << dfa->count_states() << endl;
    m_Dfa = dfa;
    
    // Build the final lexer
    m_Lexer = new lexer(*dfa);
    
    // Write some parting words
    // (Well, this is really kibibytes but I can't take blibblebytes seriously as a unit of measurement)
    cons().verbose_stream() << L"    Approximate size of final lexer:        " << (m_Lexer->size() + 512) / 1024 << L" kilobytes" << endl;
}
