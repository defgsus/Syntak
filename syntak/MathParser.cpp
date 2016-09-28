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

#include <QMap>
#include <QList>

#include "MathParser.h"
#include "Parser.h"
#include "error.h"

#if 0
#   define SYNTAK_DEBUG(arg__) { qDebug().noquote().nospace() << arg__; }
#else
#   define SYNTAK_DEBUG(unused__) { }
#endif

namespace Syntak {

namespace {

    template <typename>
    struct TypeTraits { };

    template <>
    struct TypeTraits<uint8_t>
    {
        typedef uint8_t T;
        static bool isSigned() { return false; }
        static T toValue(const QString& t) { return t.toUInt(); }
        static QRegExp numberRegex() { return QRegExp("[0-9]+"); }
        static T modulo(T a, T b) { return a % b; }
    };
    template <>
    struct TypeTraits<uint16_t>
    {
        typedef uint16_t T;
        static bool isSigned() { return false; }
        static T toValue(const QString& t) { return t.toUInt(); }
        static QRegExp numberRegex() { return QRegExp("[0-9]+"); }
        static T modulo(T a, T b) { return a % b; }
    };
    template <>
    struct TypeTraits<uint32_t>
    {
        typedef uint32_t T;
        static bool isSigned() { return false; }
        static T toValue(const QString& t) { return t.toUInt(); }
        static QRegExp numberRegex() { return QRegExp("[0-9]+"); }
        static T modulo(T a, T b) { return a % b; }
    };
    template <>
    struct TypeTraits<uint64_t>
    {
        typedef uint64_t T;
        static bool isSigned() { return false; }
        static T toValue(const QString& t)
            { return t.toULongLong(); }
        static QRegExp numberRegex() { return QRegExp("[0-9]+"); }
        static T modulo(T a, T b) { return a % b; }
    };

    template <>
    struct TypeTraits<int8_t>
    {
        typedef int8_t T;
        static bool isSigned() { return true; }
        static T toValue(const QString& t) { return t.toInt(); }
        static QRegExp numberRegex() { return QRegExp("[0-9]+"); }
        static T modulo(T a, T b) { return a % b; }
    };
    template <>
    struct TypeTraits<int16_t>
    {
        typedef int16_t T;
        static bool isSigned() { return true; }
        static T toValue(const QString& t) { return t.toInt(); }
        static QRegExp numberRegex() { return QRegExp("[0-9]+"); }
        static T modulo(T a, T b) { return a % b; }
    };
    template <>
    struct TypeTraits<int32_t>
    {
        typedef int32_t T;
        static bool isSigned() { return true; }
        static T toValue(const QString& t) { return t.toInt(); }
        static QRegExp numberRegex() { return QRegExp("[0-9]+"); }
        static T modulo(T a, T b) { return a % b; }
    };
    template <>
    struct TypeTraits<int64_t>
    {
        typedef int64_t T;
        static bool isSigned() { return true; }
        static T toValue(const QString& t)
            { return t.toLongLong(); }
        static QRegExp numberRegex() { return QRegExp("[0-9]+"); }
        static T modulo(T a, T b) { return a % b; }
    };

    template <>
    struct TypeTraits<float>
    {
        typedef float T;
        static bool isSigned() { return true; }
        static T toValue(const QString& t) { return t.toFloat(); }
        static QRegExp numberRegex() { return QRegExp(
                "(\\d+(\\.\\d*)?|\\.\\d+)([eE][+-]?\\d+)?"); }
        static T modulo(T a, T b) { return std::fmod(a, b); }
    };
    template <>
    struct TypeTraits<double>
    {
        typedef double T;
        static bool isSigned() { return true; }
        static T toValue(const QString& t) { return t.toDouble(); }
        static QRegExp numberRegex() { return QRegExp(
                "(\\d+(\\.\\d*)?|\\.\\d+)([eE][+-]?\\d+)?"); }
        static T modulo(T a, T b) { return std::fmod(a, b); }
    };


} // namespace





template <typename F>
struct MathParser<F>::Private
{
    Private(MathParser* p)
        : p             (p)
        , needsReinit   (true)
        , ignoreZeroDiv (false)
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

    F f_modulo(F p1, F p2, const ParsedNode* n);
    F f_divide(F p1, F p2, const ParsedNode* n);

    MathParser* p;
    Parser parser;

    bool needsReinit, ignoreZeroDiv;

    QMap<QString, std::function<F(F, F)>> funcsBinary;

    QList<Node> stack;
};


template <typename F>
MathParser<F>::MathParser()
    : p_        (new Private(this))
{

}

template <typename F>
MathParser<F>::~MathParser()
{
    delete p_;
}

template <typename F>
const Parser& MathParser<F>::parser() const { return p_->parser; }

template <typename F>
const QString& MathParser<F>::expression() const
    { return p_->parser.text(); }

template <typename F>
void MathParser<F>::setIgnoreDivisionByZero(bool e)
{
    p_->ignoreZeroDiv = e;
}

template <typename F>
void MathParser<F>::addFunction(
        const QString& n, std::function<F(F,F)> foo)
{
    p_->needsReinit = p_->funcsBinary.isEmpty();
    p_->funcsBinary.insert(n, foo);
}


template <typename F>
void MathParser<F>::Private::init()
{
    Tokens lex;

    const bool
            isSigned = TypeTraits<F>::isSigned(),
            hasBinaryFunc = !funcsBinary.isEmpty();

    lex << Token("plus", "+")
        << Token("minus", "-")
        << Token("mul", "*")
        << Token("div", "/")
        << Token("mod", "%")
        << Token("bopen", "(")
        << Token("bclose", ")");
    if (!isSigned)
        lex << Token("num", TypeTraits<F>::numberRegex());
    else
        lex << Token("unsigned_num", TypeTraits<F>::numberRegex());
    if (hasBinaryFunc)
    {
        lex << Token("comma", ",");
        lex << Token("ident", QRegExp("[A-Za-z_]+"));
    }

    Rules rules;
    rules.addTokens(lex);

    // top rule
    rules.createAnd("expression",   "expr");

    rules.createOr( "op1",          "plus" , "minus");
    rules.createOr( "op2",          "mul" , "div", "mod");
    rules.createAnd("op1_term",     "op1" , "term");
    rules.createAnd("op2_factor",   "op2" , "factor");
    rules.createAnd("expr",         "term" , "[op1_term]*");
    rules.createAnd("term",         "factor" , "[op2_factor]*");
    QStringList factorList; factorList << "num" << "quoted_expr";
    if (!isSigned)
    {
        rules.createAnd("quoted_expr",  "bopen" , "expr" , "bclose");
    }
    else
    {
        rules.createAnd("quoted_expr",  "[op1]", "bopen",
                                        "expr" , "bclose");
        rules.createAnd("num",          "[op1]", "unsigned_num");
    }
    if (hasBinaryFunc)
    {
        rules.createAnd("binary_func",  QStringList()
                        << "ident" << "bopen" << "expr" << "comma"
                        << "expr" << "bclose");
        factorList << "binary_func";
    }
    rules.createOr( "factor",       factorList);

    rules.check();

    // num in factor
    rules.connect("factor", 0, [=](ParsedNode* n)
    {
        SYNTAK_DEBUG("EMIT " << n->toString());
        stack << n;
    });

    rules.connect("op1_term", [=](ParsedNode* n)
    {
        SYNTAK_DEBUG("EMIT " << n->toString());
        F p2 = takeLastValue(), p1 = takeLastValue();
        if (n->text().startsWith("+"))
            stack << Node(p1 + p2);
        else
            stack << Node(p1 - p2);
    });
    rules.connect("op2_factor", [=](ParsedNode* n)
    {
        SYNTAK_DEBUG("EMIT " << n->toString());
        F p2 = takeLastValue(), p1 = takeLastValue();
        if (n->text().startsWith("*"))
            stack << Node(p1 * p2);
        else if (n->text().startsWith("%"))
            stack << Node(f_modulo(p1, p2, n));
        else
            stack << Node(f_divide(p1, p2, n));
    });

    if (isSigned)
    {
        // sign of quoted expression
        rules.connect("quoted_expr", [=](ParsedNode* n)
        {
            SYNTAK_DEBUG("EMIT " << n->toString());
            // if expression is negative:
            // pop last value (evaluated result of quoted expression)
            // and push back negative
            if (n->text().startsWith("-"))
            {
                stack << Node(-takeLastValue());
            }
        });
    }
    if (hasBinaryFunc)
    {
        rules.connect("binary_func", [=](ParsedNode* n)
        {
            SYNTAK_DEBUG("EMIT " << n->toString());
            if (n->children().empty())
                SYNTAK_ERROR("Expected identifier node in "
                             "binary_func node");
            if (n->children()[0]->name() != "ident")
                SYNTAK_ERROR("Expected identifier node in "
                             "binary_func node, got "
                             << n->children()[0]->name());
            QString func = n->children()[0]->text();
            if (!funcsBinary.contains(func))
                SYNTAK_ERROR("Unknown function '" << func << "' at "
                             << n->pos().toString());
            F p2 = takeLastValue(), p1 = takeLastValue();
            stack << Node(funcsBinary.value(func)(p1, p2));
        });
    }

    parser.setTokens(lex);
    parser.setRules(rules);
}

template <typename F>
F MathParser<F>::Private::takeLastValue()
{
    if (stack.isEmpty())
        SYNTAK_ERROR("math stack empty");

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
F MathParser<F>::Private::f_modulo(F p1, F p2, const ParsedNode* n)
{
    if (p2 == F(0))
    {
        if (!ignoreZeroDiv)
            SYNTAK_ERROR("Modulo by zero at " << n->pos().toString())
        else
            return F(0);
    }
    else
        return TypeTraits<F>::modulo(p1, p2);
}

template <typename F>
F MathParser<F>::Private::f_divide(F p1, F p2, const ParsedNode* n)
{
    if (p2 == F(0))
    {
        if (!ignoreZeroDiv)
            SYNTAK_ERROR("Division by zero at " << n->pos().toString())
        else
            return F(0);
    }
    else
        return p1 / p2;
}

template <typename F>
F MathParser<F>::evaluate(const QString& expression)
{
    if (p_->needsReinit)
    {
        p_->needsReinit = false;
        p_->init();
    }

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
