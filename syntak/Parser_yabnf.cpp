/***************************************************************************

MIT License

Copyright (c) 2016 stefan berke

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

****************************************************************************/

#include "Parser.h"

namespace Syntak {

/**
http://cui.unige.ch/db-research/Enseignement/analyseinfo/AboutBNF.html

syntax     ::=  { rule }
rule       ::=  identifier  "::="  expression
expression ::=  term { "|" term }
term       ::=  factor { factor }
factor     ::=  identifier |
                quoted_symbol |
                "("  expression  ")" |
                "["  expression  "]" |
                "{"  expression  "}"
identifier ::=  letter { letter | digit }
quoted_symbol ::= """ { any_character } """


*/



Parser Parser::createYabnfParser()
{
    Tokens tok;
    tok
        << Token("ident",     QRegExp("[A-Za-z][A-Za-z0-9]*"))
        << Token("alpha",     QRegExp("[A-Za-z]"))
        << Token("digit",     QRegExp("[0-9]"))
        << Token("squoted_sym",QRegExp("'[\x01-\x26,\x28-\x7f]*'"))
        << Token("dquoted_sym",QRegExp("\"[\x01-\x21,\x23-\x7f]*\""))
        << Token("colon",     ":")
        << Token("semicolon", ";")
        << Token("star",      "*")
        << Token("or",        "|")
        << Token("brace_o",   "(")
        << Token("brace_c",   ")")
        << Token("bracket_o", "[")
        << Token("bracket_c", "]")
        //<< Token("other_char",
        //         QRegExp("[0x20-0x2f,0x3a-0x40,0x5b-0x60,0x7b-0x7e"))
           ;

    Rules rules;
    rules.addTokens(tok);
    rules.createAnd("syntax",       "[rule]*");
    rules.createAnd("rule",         "ident", "colon", "expr", "semicolon");
    rules.createAnd("expr",         "term", "[or_term]*");
    rules.createAnd("or_term",      "or", "term");
    rules.createAnd("term",         "factor", "[factor]*");
    rules.createOr ("factor",       "optional_factor",
                                    "nonoptional_factor");
    rules.createAnd("optional_factor",
                                    "nonoptional_factor", "star");
    rules.createOr ("nonoptional_factor",
                                    "ident", "quoted_sym",
                                    "grouped_expr", "optional_expr");
    rules.createAnd("grouped_expr", "brace_o", "expr", "brace_c");
    rules.createAnd("optional_expr","bracket_o", "expr", "bracket_c");

    rules.createOr ("quoted_sym",   "dquoted_sym", "squoted_sym");

    Parser p;
    p.setTokens(tok);
    p.setRules(rules);

    return p;
}

} // namespace Syntak
