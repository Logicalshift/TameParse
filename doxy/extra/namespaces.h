//
//  namespaces.h
//  TameParse
//
//  Created by Andrew Hunter on 26/02/2012.
//  Copyright (c) 2012 Andrew Hunter. All rights reserved.
//

// Documentation for the namespaces declared by TameParse

///
/// \brief Classes relating to the creation of NDFA/DFAs and lexers
///
/// The classes in this namespace provide a general framework for working with
/// NDFAs and building DFAs. In the context of TameParse, these are used to 
/// build the lexer.
///
/// Ancilliary classes deal with the problems associated with grouping symbol
/// sets.
///
namespace dfa {

}

///
/// \brief Classes that provide data structures required to describe a context-free grammar
///
/// The contextfree::grammar class represents a full context-free grammar. The
/// various subclasses of contextfree::item are used to build up productions,
/// stored in contextfree::rule, for the nonterminals within a grammar.
///
namespace contextfree {

}

///
/// \brief Classes that deal with LR parsers
///
/// The lr::lalr_builder class is used to convert a context-free grammar into
/// a LALR parser. The lr::parser template can be used to build various
/// different kinds of parser using the results (it's most usefully used
/// as the parser class by the C++ ouput language)
///
namespace lr {

}

///
/// \brief Data-storage classes representing the TameParse input language
///
namespace yy_language {

}

///
/// \brief Classes used to implement the tameparse command-line tool
///
namespace compiler {

}

///
/// \brief Support classes
///
namespace util {

}
