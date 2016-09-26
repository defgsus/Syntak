/***************************************************************************

Copyright (C) 2016  stefan.berke @ modular-audio-graphics.com

This source is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either
version 3.0 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this software; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

****************************************************************************/

#ifndef SYNTAKSRC_TESTS_TEST_MATH_SIMPLE_MATHPARSER_H
#define SYNTAKSRC_TESTS_TEST_MATH_SIMPLE_MATHPARSER_H

#include "Parser.h"

#ifndef PRINT
    #if 1
        #define PRINT(arg__) \
            { qDebug().noquote().nospace() << arg__; }
    #elif
        #define PRINT(unused__) { }
    #endif
#endif

class SimpleMathParser
{
public:
    SimpleMathParser() : rootNode(0) { init(); }
    ~SimpleMathParser() { delete rootNode; }

    struct Node
    {
        Node(const ParsedToken& t) : t(t) { }
        Node(int v) : value(v) { }
        ParsedToken t;
        int value;
    };

    Parser parser;
    ParsedNode* rootNode;
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
        rules.createAnd("quoted_expr",  "bopen" , "expr" , "bclose");
        rules.createOr( "factor",       "num", "quoted_expr");
        rules.createAnd("num",          "digit" , "[digit]*");

        rules.createAnd("program",      "expr");

        rules.check();

#define DO_STACK

        // num in factor
        rules.connect("factor", 0, [=](const ParsedToken& t)
        {
            emits << t;
            stack << t;
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
                stack << Node(p2 != 0 ? (p1 / p2) : 0);
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

        delete rootNode;
        rootNode = parser.parse(text);
        //PRINT(rootNode->toBracketString());
        //printNodes();
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

    void printNodes() { printNodes(rootNode); }
    void printNodes(ParsedNode* node)
    {
        PRINT(node->toString() << "\t\""
              << parser.text().mid(node->pos().pos(),
                                   node->length()) << "\"");
        for (auto c : node->children())
            printNodes(c);
    }

    int takeLastInt()
    {
        auto n = stack.takeLast();
        if (!n.t.isValid())
            return n.value;
        if (n.t.rule()->name() == "num")
        {
            return n.t.text().toInt();
        }
        print();
        PARSE_ERROR("expected num in stack, got "
                    << n.t.rule()->name());
        return 0;
    }
};

#endif // SYNTAKSRC_TESTS_TEST_MATH_SIMPLE_MATHPARSER_H

