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

#include <cstddef>

#include "MathParser.h"
#include "Parser.h"
#include "error.h"

namespace Syntak {

namespace {

    template <typename>
    struct TypeTraits { };

    template <>
    struct TypeTraits<uint8_t>
    {
        static uint8_t toValue(const QString& t) { return t.toUInt(); }
        static QRegExp numberRegex() { return QRegExp("[0-9][0-9]*"); }
    };
    template <>
    struct TypeTraits<uint16_t>
    {
        static uint16_t toValue(const QString& t) { return t.toUInt(); }
        static QRegExp numberRegex() { return QRegExp("[0-9][0-9]*"); }
    };
    template <>
    struct TypeTraits<uint32_t>
    {
        static uint32_t toValue(const QString& t) { return t.toUInt(); }
        static QRegExp numberRegex() { return QRegExp("[0-9][0-9]*"); }
    };
    template <>
    struct TypeTraits<uint64_t>
    {
        static uint64_t toValue(const QString& t) { return t.toInt(); }
        static QRegExp numberRegex() { return QRegExp("[0-9][0-9]*"); }
    };

    template <>
    struct TypeTraits<int8_t>
    {
        static int8_t toValue(const QString& t) { return t.toInt(); }
        static QRegExp numberRegex() { return QRegExp("[0-9][0-9]*"); }
    };
    template <>
    struct TypeTraits<int16_t>
    {
        static int16_t toValue(const QString& t) { return t.toInt(); }
        static QRegExp numberRegex() { return QRegExp("[0-9][0-9]*"); }
    };
    template <>
    struct TypeTraits<int32_t>
    {
        static int32_t toValue(const QString& t) { return t.toInt(); }
        static QRegExp numberRegex() { return QRegExp("[0-9][0-9]*"); }
    };
    template <>
    struct TypeTraits<int64_t>
    {
        static int64_t toValue(const QString& t) { return t.toLongLong(); }
        static QRegExp numberRegex() { return QRegExp("[0-9][0-9]*"); }
    };

    template <>
    struct TypeTraits<float>
    {
        static float toValue(const QString& t) { return t.toFloat(); }
        static QRegExp numberRegex() { return QRegExp("[0-9][0-9]*"); }
    };
    template <>
    struct TypeTraits<double>
    {
        static double toValue(const QString& t) { return t.toDouble(); }
        static QRegExp numberRegex() { return QRegExp("[0-9][0-9]*"); }
    };


} // namespace





template <typename F>
struct MathParser<F>::Private
{
    Private(MathParser* p)
        : p     (p)
    {

    }

    struct Node
    {
        Node(const ParsedNode* n) : node(n), value(0) { }
        Node(F v) : node(nullptr), value(v) { }
        const ParsedNode* node;
        F value;
    };

    void init();
    F takeLastValue();

    MathParser* p;
    Parser parser;

    QList<Node> stack;
};


template <typename F>
MathParser<F>::MathParser()
    : p_        (new Private(this))
{
    p_->init();
}

template <typename F>
MathParser<F>::~MathParser()
{
    delete p_;
}

template <typename F>
const Parser& MathParser<F>::parser() const { return p_->parser; }


template <typename F>
void MathParser<F>::Private::init()
{
    Tokens lex;

    lex << Token("plus", "+")
        << Token("minus", "-")
        << Token("mul", "*")
        << Token("div", "/")
        << Token("bopen", "(")
        << Token("bclose", ")")
        << Token("num", TypeTraits<F>::numberRegex())
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

    rules.createAnd("program",      "expr");

    rules.check();

    // num in factor
    rules.connect("factor", 0, [=](ParsedNode* n)
    {
        stack << n;
    });

    rules.connect("op1_term", [=](ParsedNode* n)
    {
        int p2 = takeLastValue(), p1 = takeLastValue();
        if (n->text().startsWith("+"))
            stack << Node(p1 + p2);
        else
            stack << Node(p1 - p2);
    });
    rules.connect("op2_factor", [=](ParsedNode* n)
    {
        int p2 = takeLastValue(), p1 = takeLastValue();
        if (n->text().startsWith("*"))
            stack << Node(p1 * p2);
        else
            stack << Node(p2 != 0 ? (p1 / p2) : 0);
    });

    parser.setTokens(lex);
    parser.setRules(rules);
}

template <typename F>
F MathParser<F>::Private::takeLastValue()
{
    const Node& n = stack.takeLast();
    if (!n.node)
        return n.value;
    if (n.node->name() == "num")
    {
        return TypeTraits<F>::toValue( n.node->text() );
    }
    SYNTAK_ERROR("expected num in stack, got "
                << n.node->name() );
    return 0;
}


template <typename F>
F MathParser<F>::evaluate(const QString& expression)
{
    p_->stack.clear();
    auto node = p_->parser.parse(expression);
    if (p_->stack.size() != 1)
        SYNTAK_ERROR("Something went wrong with the math stack");
    F val = p_->takeLastValue();
    delete node;
    return val;
}




#define SYNTAK__INSTANTIATE(T__) \
    template class MathParser<T__>;

SYNTAK__INSTANTIATE(int8_t)
SYNTAK__INSTANTIATE(int16_t)
SYNTAK__INSTANTIATE(int32_t)
SYNTAK__INSTANTIATE(int64_t)
SYNTAK__INSTANTIATE(uint8_t)
SYNTAK__INSTANTIATE(uint16_t)
SYNTAK__INSTANTIATE(uint32_t)
SYNTAK__INSTANTIATE(uint64_t)
SYNTAK__INSTANTIATE(float)
SYNTAK__INSTANTIATE(double)

} // namespace Syntak
