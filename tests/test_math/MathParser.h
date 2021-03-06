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

#ifndef SYNTAKSRC_TESTS_TEST_MATH_MATHPARSER_H
#define SYNTAKSRC_TESTS_TEST_MATH_MATHPARSER_H

#include "Parser.h"

#if 1
#define PRINT(arg__) \
    { qDebug().noquote().nospace() << arg__; }
#else
#define PRINT(unused__) { }
#endif

class MathParser
{
public:
    MathParser() { init(); }

    struct Node
    {
        Node(const ParsedToken& t) : t(t) { }
        Node(int v) : value(v) { }
        ParsedToken t;
        int value;
    };

    Parser parser;
    QList<ParsedToken> emits;
    QList<Node> stack;
    QMap<QString, int> variables;

    void init()
    {
        Tokens lex;

        lex << Token("plus", "+")
            << Token("minus", "-")
            << Token("mul", "*")
            << Token("div", "/")
            << Token("bopen", "(")
            << Token("bclose", ")")
            << Token("semicolon", ";")
            << Token("dot", ".")
            << Token("equals", "=")
            << Token("print", "print")
            << Token("letter", QRegExp("[a-z,A-Z]"))
            << Token("digit", QRegExp("[0-9]"))
               ;

        Rules rules;
        rules.addTokens(lex);
        rules.createOr( "op1",          "plus" , "minus");
        rules.createOr( "op2",          "mul" , "div");
        rules.createAnd("op1_term",     "op1" , "term");
        rules.createAnd("op2_factor",   "op2" , "factor");
        rules.createAnd("expr",         "term" , "[op1_term]*");
        rules.createAnd("term",         "factor" , "[op2_factor]*");
        rules.createOr( "factor",       "int_expr");
        rules.createAnd("quoted_expr",  "bopen" , "expr" , "bclose");
        rules.createAnd("uint",         "digit" , "[digit]*");
        rules.createOr ("uint_expr",    "uint", "ident", "quoted_expr");
        rules.createAnd("int_expr",     "[op1]", "uint_expr");

        rules.createAnd("program",      "s_statement", "[s_statement]*");
        rules.createAnd("s_statement",  "statement", "semicolon");
        rules.createOr ("statement",    "assignment" , "print_call");
        rules.createAnd("assignment",   "ident", "equals", "expr");
        rules.createAnd("print_call",   "print", "bopen", "expr", "bclose");
        rules.createAnd("ident",        "letter" , "[alnum]*");
        rules.createAnd("signed_ident", "[op1]" , "ident");
        rules.createOr ("alnum",        "letter" , "digit");


        rules.check();

#define DO_STACK

        rules.connect("assignment", 0, [=](const ParsedToken& t)
        {
            emits << t;
            stack << t;
        });
        rules.connect("assignment", [=](const ParsedToken& t)
        {
            emits << t;
#ifdef DO_STACK
            int v = takeLastInt();
            auto p = stack.takeLast();
            variables.insert(p.t.text(), v);
            stack << Node(v);
#endif
        });
        // int in factor
        rules.connect("factor", 0, [=](const ParsedToken& t)
        {
            emits << t;
#ifdef DO_STACK
            if (!t.text().startsWith("("))
                stack << t;
#endif
        });
        rules.connect("op1_term", [=](const ParsedToken& t)
        {
            emits << t;
#ifdef DO_STACK
            int p2 = takeLastInt(), p1 = takeLastInt();
            if (t.text().startsWith("+"))
                stack << Node(p1 + p2);
            else
                stack << Node(p1 - p2);
#endif
        });
        rules.connect("op2_factor", [=](const ParsedToken& t)
        {
            emits << t;
#ifdef DO_STACK
            int p2 = takeLastInt(), p1 = takeLastInt();
            if (t.text().startsWith("*"))
                stack << Node(p1 * p2);
            else
                stack << Node(p1 / p2);
#endif
        });

        //PRINT(rules.toDefinitionString());

        parser.setLexxer(lex);
        parser.setRules(rules);

    }

    void parse(const QString& text)
    {
        emits.clear();
        stack.clear();
        variables.clear();

        parser.parse(text);
    }

    void print()
    {
        PRINT("\n" << parser.text());
        PRINT("Nodes visited: " << parser.numNodesVisited());
        PRINT("-- all emits --");
        for (auto& s : emits)
            PRINT(s.toString());
#ifdef DO_STACK
        PRINT("-- stack --");
        for (auto& s : stack)
            PRINT( (s.t.isValid() ? s.t.toString()
                                  : QString::number(s.value)) );
        PRINT("-- vars --");
        for (auto i = variables.begin(); i!=variables.end(); ++i)
            PRINT( QString("'%1' : %2").arg(i.key()).arg(i.value()) );
#endif
    }

    int takeLastInt()
    {
        auto n = stack.takeLast();
        if (!n.t.isValid())
            return n.value;
        if (n.t.rule()->name() == "int_expr")
        {
            QString text = n.t.text();
            bool neg = text.startsWith("-");
            text.remove("-").remove("+");
            int v;
            if (variables.contains(text))
                v = variables[text];
            else
                v = text.toInt();
            return neg ? -v : v;
        }
        PARSE_ERROR("expected int_expr in stack, got "
                    << n.t.rule()->name());
        return 0;
    }
};

#endif // SYNTAKSRC_TESTS_TEST_MATH_MATHPARSER_H

