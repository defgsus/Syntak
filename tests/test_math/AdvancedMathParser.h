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
#include "error.h"

#if 1
#define PRINT(arg__) \
    { qDebug().noquote().nospace() << arg__; }
#else
#define PRINT(unused__) { }
#endif

using namespace Syntak;

class AdvancedMathParser
{
public:
    AdvancedMathParser() : rootNode(nullptr) { init(); }
    ~AdvancedMathParser() { delete rootNode; }

    struct Node
    {
        Node(ParsedNode* n) : node(n), value(0) { }
        Node(int v) : node(nullptr), value(v) { }
        ParsedNode* node;
        int value;
    };

    Parser parser;
    ParsedNode* rootNode;
    QList<ParsedNode*> emits;
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
            << Token("ident", QRegExp("[A-Za-z][0-9A-Za-z]*"))
               ;

//#define DO_SIGNED

        Rules rules;
        rules.addTokens(lex);
        rules.createOr( "op1",          "plus" , "minus");
        rules.createOr( "op2",          "mul" , "div");
        rules.createAnd("op1_term",     "op1" , "term");
        rules.createAnd("op2_factor",   "op2" , "factor");
        rules.createAnd("expr",         "term" , "[op1_term]*");
        rules.createAnd("term",         "factor" , "[op2_factor]*");
        rules.createAnd("quoted_expr",  "bopen" , "expr" , "bclose");
#ifdef DO_SIGNED
        rules.createOr( "factor",       "int_val", "int_quoted_expr");
        rules.createAnd("uint",         "digit" , "[digit]*");
        rules.createOr ("uint_val",     "uint", "ident");
        rules.createAnd("int_val",      "[op1]", "uint_val");
        rules.createAnd("int_quoted_expr",
                                        "[op1]", "quoted_expr");
#else
        rules.createOr( "factor",       "int_val", "quoted_expr");
        rules.createAnd("int_val",      "digit" , "[digit]*");
#endif
        rules.createAnd("program",      "s_statement", "[s_statement]*");
        rules.createAnd("s_statement",  "statement", "semicolon");
        rules.createOr ("statement",    "assignment" , "print_call");
        rules.createAnd("assignment",   "ident", "equals", "expr");
        rules.createAnd("print_call",   "print", "bopen", "expr", "bclose");
        //rules.createAnd("ident",        "letter" , "[alnum]*");
        //rules.createAnd("signed_ident", "[op1]" , "ident");
        //rules.createOr ("alnum",        "letter" , "digit");


        rules.check();

#define DO_STACK

        // ident name
        rules.connect("assignment", 0, [=](ParsedNode* n)
        {
            emits << n;
            stack << n;
        });
        rules.connect("assignment", [=](ParsedNode* n)
        {
            emits << n;
#ifdef DO_STACK
            int v = takeLastInt();
            auto p = stack.takeLast();
            variables.insert(p.node->text(), v);
            stack << Node(v);
#endif
        });
        // int_val in factor
        rules.connect("factor", 0, [=](ParsedNode* n)
        {
            emits << n;
            stack << n;
        });
#ifdef DO_SIGNED
        // int_quoted_expr in factor
        rules.connect("factor", 1, [=](const ParsedNode* n)
        {
            emits << n;
            stack << n;
        });
#endif
        rules.connect("op1_term", [=](ParsedNode* n)
        {
            emits << n;
#ifdef DO_STACK
            int p2 = takeLastInt(), p1 = takeLastInt();
            if (n->text().startsWith("+"))
                stack << Node(p1 + p2);
            else
                stack << Node(p1 - p2);
#endif
        });
        rules.connect("op2_factor", [=](ParsedNode* n)
        {
            emits << n;
#ifdef DO_STACK
            int p2 = takeLastInt(), p1 = takeLastInt();
            if (n->text().startsWith("*"))
                stack << Node(p1 * p2);
            else
                stack << Node(p2 != 0 ? (p1 / p2) : 0);
#endif
        });
        //PRINT(rules.toDefinitionString());

        parser.setTokens(lex);
        parser.setRules(rules);

    }

    void parse(const QString& text)
    {
        emits.clear();
        stack.clear();
        variables.clear();

        delete rootNode;
        rootNode = parser.parse(text);
        //PRINT("\n" << rootNode->toBracketString(true));
        auto reduced = rootNode->reducedTree();
        PRINT("\n" << reduced->toBracketString());
        delete reduced;
        //printNodes();
    }

    void print()
    {
        PRINT("\n" << parser.text());
        PRINT("Nodes visited: " << parser.numNodesVisited());
        PRINT("-- all emits --");
        for (auto& s : emits)
            PRINT(s->toString());
#ifdef DO_STACK
        PRINT("-- stack --");
        for (auto& s : stack)
            PRINT( (s.node ? s.node->toString()
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
        const Node& n = stack.takeLast();
        if (!n.node)
            return n.value;

        if (n.node->name() == "int_val")
        {
            QString text = n.node->text();
            bool neg = text.startsWith("-");
            text.remove("-").remove("+");
            int v;
            if (variables.contains(text))
                v = variables[text];
            else
            {
                bool ok;
                v = text.toInt(&ok);
                if (!ok)
                {
                    print();
                    SYNTAK_ERROR("Failed to convert '" << text
                                << "' to int");
                }
            }
            return neg ? -v : v;
        }
        if (n.node->name() == "int_quoted_expr")
        {
            QString text = n.node->text();
            bool neg = text.startsWith("-");
            text.remove("-").remove("+").remove("(").remove(")");
            int v;
            if (variables.contains(text))
                v = variables[text];
            else
            {
                bool ok;
                v = text.toInt(&ok);
                if (!ok)
                {
                    print();
                    SYNTAK_ERROR("Failed to convert '" << text
                                << "' to int");
                }
            }
            return neg ? -v : v;
        }

        print();
        SYNTAK_ERROR("expected int in stack, got "
                    << n.node->name());
        return 0;
    }
};

#endif // SYNTAKSRC_TESTS_TEST_MATH_MATHPARSER_H

