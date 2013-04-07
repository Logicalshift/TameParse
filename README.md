# TameParse

## What is it?

TameParse is a bottom-up parser generator that can deal with some context-sensitive
languages - particularly programming languages. The algorithm used is an extension 
of the venerable LALR algorithm, alongside a DFA based lexer.

TameParse has a few other features designed to make building practical parsers
easier:

 * 'Weak' lexical symbols
 * EBNF grammar syntax
 * Language inheritance (base one parser on another)
 * Output is an AST
 * Multiple start symbols
 * C++ API supporting dynamic parser generation
 * Language testing
 * Unicode support
 * Case-insensitivity support

## Context sensitivity

A context-sensitive parser will accept a narrower range of inputs than a context-free
parser. This is often desirable, as it eliminates the need to post-process the output
of a parser to produce an accurate representation of the structure of an input.

TameParse's current implementation adds context-sensitivity by making it possible to
match the lookahead against a context-free language. This is done by adding a 'guard'
to the grammar for a rule. The symbols following the guard are only matched if the
lookahead at that point matches the symbols inside the guard. Guards can be used to
resolve 

A guard has the syntax [=> Rule]. For example, this rule:

    <A> = <B>

could be rewritten with a guard:

    <A> = [=> <B>] <B>

This tells the parser to check that the lookahead at this point can match rule B and
to preferentially take that path if any other (unguarded) rule is present, resolving
any ambiguity. In this form, the parser is effectively working as a backtracking
parser with disambiguation.

There's no need for the guard rule to match what follows, though. The rule could be
written like this instead:

    <A> = [=> <C>] <B>

This tells the parser to match &lt;B> only if the lookahead also matches &lt;C>, 
effectively creating a conditional rule. This makes it possible for the tool to accept
certain context-sensitive languages. (See the ContextSensitive.tp example for a 
demonstration of how to write guards to accept an indexed language)

This particular type of rule is especially useful with programming languages, which
often disambiguate the meaning of an ambiguous piece of code based on what follows
or preceeds it.

# Status

The plan is to support many target languages for the generated parsers but the current
version really only has C++ support.

The code to display details about a conflict has a slight tendency to run for very 
long periods of time and display way too much information.

# License

Copyright (c) 2011-2013 Andrew Hunter

Permission is hereby granted, free of charge, to any person obtaining a copy 
of this software and associated documentation files (the "Software"), to 
deal in the Software without restriction, including without limitation the 
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or 
sell copies of the Software, and to permit persons to whom the Software is 
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS 
IN THE SOFTWARE.

# Contact

andrew (at) logicalshift.co.uk
