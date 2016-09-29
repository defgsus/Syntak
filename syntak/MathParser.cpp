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
        static bool isFloat() { return false; }
        static T toValue(const QString& t) { return t.toUInt(); }
        static QRegExp numberRegex() { return QRegExp("[0-9]+"); }
        static T modulo(T a, T b) { return a % b; }
    };
    template <>
    struct TypeTraits<uint16_t>
    {
        typedef uint16_t T;
        static bool isSigned() { return false; }
        static bool isFloat() { return false; }
        static T toValue(const QString& t) { return t.toUInt(); }
        static QRegExp numberRegex() { return QRegExp("[0-9]+"); }
        static T modulo(T a, T b) { return a % b; }
    };
    template <>
    struct TypeTraits<uint32_t>
    {
        typedef uint32_t T;
        static bool isSigned() { return false; }
        static bool isFloat() { return false; }
        static T toValue(const QString& t) { return t.toUInt(); }
        static QRegExp numberRegex() { return QRegExp("[0-9]+"); }
        static T modulo(T a, T b) { return a % b; }
    };
    template <>
    struct TypeTraits<uint64_t>
    {
        typedef uint64_t T;
        static bool isSigned() { return false; }
        static bool isFloat() { return false; }
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
        static bool isFloat() { return false; }
        static T toValue(const QString& t) { return t.toInt(); }
        static QRegExp numberRegex() { return QRegExp("[0-9]+"); }
        static T modulo(T a, T b) { return a % b; }
    };
    template <>
    struct TypeTraits<int16_t>
    {
        typedef int16_t T;
        static bool isSigned() { return true; }
        static bool isFloat() { return false; }
        static T toValue(const QString& t) { return t.toInt(); }
        static QRegExp numberRegex() { return QRegExp("[0-9]+"); }
        static T modulo(T a, T b) { return a % b; }
    };
    template <>
    struct TypeTraits<int32_t>
    {
        typedef int32_t T;
        static bool isSigned() { return true; }
        static bool isFloat() { return false; }
        static T toValue(const QString& t) { return t.toInt(); }
        static QRegExp numberRegex() { return QRegExp("[0-9]+"); }
        static T modulo(T a, T b) { return a % b; }
    };
    template <>
    struct TypeTraits<int64_t>
    {
        typedef int64_t T;
        static bool isSigned() { return true; }
        static bool isFloat() { return false; }
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
        static bool isFloat() { return true; }
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
        static bool isFloat() { return true; }
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
    QString getFuncName(ParsedNode* n);
    F getFuncValue(QString name, int numAgrs, const ParsedNode* n);

    /** Returns true if functions and constant/variable
        identifiers share a name */
    bool hasIdentifierNameClash();

    F f_modulo(F p1, F p2, const ParsedNode* n);
    F f_divide(F p1, F p2, const ParsedNode* n);

    MathParser* p;
    Parser parser;

    bool needsReinit, ignoreZeroDiv;

    struct Function
    {
        std::function<F(F)> func1;
        std::function<F(F, F)> func2;
        std::function<F(F, F, F)> func3;
        std::function<F(F, F, F, F)> func4;
    };

    QMap<QString, Function> functions;
    QMap<QString, F> constants;

    QList<Node> stack;
};


template <typename F>
MathParser<F>::MathParser()
    : p_        (new Private(this))
{

}

template <typename F>
MathParser<F>::MathParser(const MathParser& o)
    : MathParser()
{
    *this = o;
}

template <typename F>
MathParser<F>::~MathParser()
{
    delete p_;
}

template <typename F>
MathParser<F>& MathParser<F>::operator =(const MathParser& o)
{
    *p_ = *o.p_;
    p_->p = this;
    return *this;
}

template <typename F>
bool MathParser<F>::isSigned() const { return TypeTraits<F>::isSigned(); }

template <typename F>
bool MathParser<F>::isFloat() const { return TypeTraits<F>::isFloat(); }

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
        const QString& n, std::function<F(F)> foo)
{
    if (!QRegExp("^[A-Za-z_][A-Za-z0-9_]*").exactMatch(n))
        SYNTAK_ERROR("Function name '" << n << "' is not a "
                     "valid identifier");

    p_->needsReinit = true;
    if (!p_->functions.contains(n))
    {
        typename Private::Function f;
        f.func1 = foo;
        p_->functions.insert(n, f);
    }
    else
        p_->functions[n].func1 = foo;
}

template <typename F>
void MathParser<F>::addFunction(
        const QString& n, std::function<F(F,F)> foo)
{
    if (!QRegExp("^[A-Za-z_][A-Za-z0-9_]*").exactMatch(n))
        SYNTAK_ERROR("Function name '" << n << "' is not a "
                     "valid identifier");

    p_->needsReinit = true;
    if (!p_->functions.contains(n))
    {
        typename Private::Function f;
        f.func2 = foo;
        p_->functions.insert(n, f);
    }
    else
        p_->functions[n].func2 = foo;
}

template <typename F>
void MathParser<F>::addFunction(
        const QString& n, std::function<F(F,F,F)> foo)
{
    if (!QRegExp("^[A-Za-z_][A-Za-z0-9_]*").exactMatch(n))
        SYNTAK_ERROR("Function name '" << n << "' is not a "
                     "valid identifier");

    p_->needsReinit = true;
    if (!p_->functions.contains(n))
    {
        typename Private::Function f;
        f.func3 = foo;
        p_->functions.insert(n, f);
    }
    else
        p_->functions[n].func3 = foo;
}

template <typename F>
void MathParser<F>::addFunction(
        const QString& n, std::function<F(F,F,F,F)> foo)
{
    if (!QRegExp("^[A-Za-z_][A-Za-z0-9_]*").exactMatch(n))
        SYNTAK_ERROR("Function name '" << n << "' is not a "
                     "valid identifier");

    p_->needsReinit = true;
    if (!p_->functions.contains(n))
    {
        typename Private::Function f;
        f.func4 = foo;
        p_->functions.insert(n, f);
    }
    else
        p_->functions[n].func4 = foo;
}

template <typename F>
bool MathParser<F>::hasFunctions() const
{
    return !p_->functions.isEmpty();
}

template <typename F>
QStringList MathParser<F>::getFunctionNames(size_t numArguments) const
{
    QStringList list;
    for (auto i=p_->functions.begin(); i!=p_->functions.end(); ++i)
    {
        switch (numArguments)
        {
            case 1: if (i.value().func1) list << i.key(); break;
            case 2: if (i.value().func2) list << i.key(); break;
            case 3: if (i.value().func3) list << i.key(); break;
            case 4: if (i.value().func4) list << i.key(); break;
        }
    }
    return list;
}

template <typename F>
void MathParser<F>::addConstant(const QString& n, F value)
{
    if (!QRegExp("^[A-Za-z_][A-Za-z0-9_]*").exactMatch(n))
        SYNTAK_ERROR("Constant name '" << n << "' is not a "
                     "valid identifier");

    p_->needsReinit = true;
    p_->constants.insert(n, value);
}

template <typename F>
const QMap<QString, F>& MathParser<F>::constants() const
{
    return p_->constants;
}



template <typename F>
void MathParser<F>::init()
{
    p_->init();
    p_->needsReinit = false;
}

template <typename F>
bool MathParser<F>::Private::hasIdentifierNameClash()
{
    for (auto i = constants.begin(); i!=constants.end(); ++i)
        if (functions.contains(i.key()))
            return true;
    return false;
}

template <typename F>
void MathParser<F>::Private::init()
{
    Tokens lex;

    const bool
            isSigned = TypeTraits<F>::isSigned(),
            hasConstants = !constants.isEmpty(),
            hasFuncs = !functions.isEmpty();

    // ----- setup terminal symbols -----

    lex << Token("plus", "+")
        << Token("minus", "-")
        << Token("mul", "*")
        << Token("div", "/")
        << Token("mod", "%")
        << Token("caret", "^")
        << Token("bopen", "(")
        << Token("bclose", ")")
        << Token(isSigned ? "unsigned_num"
                          : "num", TypeTraits<F>::numberRegex())
           ;

    if (hasFuncs || hasConstants)
    {
        // function identifier
        lex << Token(isSigned ? "unsigned_ident"
                              : "ident",
                     QRegExp("[A-Za-z_][A-Za-z0-9_]*"));
    }
    if (hasFuncs)
    {
        // comma for argument list
        lex << Token("comma", ",");
    }

    // --------- create rules ----------

    Rules rules;
    rules.addTokens(lex);

    // top rule
    rules.createAnd("expression",   "expr", "EOF");

    rules.createOr( "op1",          "plus" , "minus");
    rules.createOr( "op2",          "mul" , "div", "mod");
    rules.createAnd("op1_term",     "op1" , "term");
    rules.createAnd("op2_factor",   "op2" , "factor");
    rules.createAnd("expr",         "term" , "[op1_term]*");
#if 1
    rules.createAnd("term",         "factor" , "[op2_factor]*");
#else
    rules.createAnd("caret_factor", "caret" , "factor");
    rules.createAnd("term",         "pow_factor" , "[op2_factor]*");
    rules.createOr ("pow_factor",   "[pow_term]", "factor");
    rules.createAnd("pow_term",     "factor", "caret_factor",
                                             "[caret_factor]*");
#endif
    QStringList factorList; factorList << "num" << "quoted_expr";
    if (hasConstants)
        factorList << "ident";
    if (!isSigned)
    {
        rules.createAnd("quoted_expr",  "bopen" , "expr" , "bclose");
    }
    else
    {
        rules.createAnd("quoted_expr",  "[op1]", "bopen",
                                                 "expr" , "bclose");
        rules.createAnd("num",          "[op1]", "unsigned_num");
        if (hasFuncs || hasConstants)
            rules.createAnd("ident",    "[op1]", "unsigned_ident");
    }
    if (hasFuncs)
    {
        rules.createAnd("arg_list",     "expr", "[comma_expr]*");
        rules.createAnd("comma_expr",   "comma", "expr");

        rules.createAnd("func",         "ident", "bopen",
                                        "arg_list", "bclose");
        factorList << "func";
    }

    Rule::OrType orType = Rule::OR_FIRST;
    if (hasIdentifierNameClash())
        orType = Rule::OR_DEEPEST;

    rules.createOr ("factor", orType, factorList);

    rules.check();

    // ---------- connections -------------

    // num in factor
    rules.connect("factor", 0, [=](ParsedNode* n)
    {
        stack << n;
    });

    // ident in factor
    if (hasConstants)
    rules.connect("factor", 2, [=](ParsedNode* n)
    {
        int sign = 1;
        QString name = n->text();
        if (isSigned)
        {
            if (name.startsWith("-"))
                sign = -1;
            if (name.startsWith("+") || sign == -1)
                name = name.remove(0,1).trimmed();
        }
        auto i = constants.find(name);
        if (i == constants.end())
            SYNTAK_ERROR("Unknown identifier " << name
                         << " at " << n->pos().toString());
        stack << Node(sign * i.value());
    });

    rules.connect("op1_term", [=](ParsedNode* n)
    {
        F p2 = takeLastValue(), p1 = takeLastValue();
        if (n->text().startsWith("+"))
            stack << Node(p1 + p2);
        else
            stack << Node(p1 - p2);
    });
    rules.connect("op2_factor", [=](ParsedNode* n)
    {
        F p2 = takeLastValue(), p1 = takeLastValue();
        if (n->text().startsWith("*"))
            stack << Node(p1 * p2);
        else if (n->text().startsWith("%"))
            stack << Node(f_modulo(p1, p2, n));
        else
            stack << Node(f_divide(p1, p2, n));
    });
    /*
    rules.connect("caret_factor", [=](ParsedNode* )
    {
        F p2 = takeLastValue(), p1 = takeLastValue();
        stack << Node(std::pow(p1, p2));
    });*/

    if (isSigned)
    {
        // sign of quoted expression
        rules.connect("quoted_expr", [=](ParsedNode* n)
        {
            // if expression is negative:
            // pop last value (evaluated result of quoted expression)
            // and push back negative
            if (n->text().startsWith("-"))
            {
                stack << Node(-takeLastValue());
            }
        });
    }

    // getting the arguments works as follows:
    // arg_list will emit it's first argument which is 'expr'
    // and the optional following arguments as 'comma_expr'
    // once 'func' is emitted, we walk backwards in the stack
    // until the first 'expr' and count the instances of
    // 'comma_expr' to get the number of arguments
    // all 'expr' and 'comma_expr' are removed from stack
    // because their values are stored in factor's 'num' emits.
    if (hasFuncs)
    {
        // expr in arg_list
        rules.connect("arg_list", 0, [=](ParsedNode* n)
        {
            stack << n;
        });

        // comma_expr in arg_list
        rules.connect("arg_list", 1, [=](ParsedNode* n)
        {
            stack << n;
        });

        rules.connect("func", [=](ParsedNode* n)
        {
            int numArgs = 1;
            for (int i=stack.size()-1; i>=0; --i)
            {
                if (!stack[i].node)
                    continue;
                // Note: We don't use 'expr' as emitter anywhere
                // else so it's save to use it as stop symbol..
                if (stack[i].node->name() == "expr")
                {
                    stack.removeAt(i);
                    break;
                }
                if (stack[i].node->name() == "comma_expr")
                {
                    stack.removeAt(i);
                    ++numArgs;
                }
            }
            QString func = getFuncName(n);
            stack << Node(getFuncValue(func, numArgs, n));
        });
    }

    parser.setTokens(lex);
    parser.setRules(rules);
}

template <typename F>
QString MathParser<F>::Private::getFuncName(ParsedNode* n)
{
    if (n->children().empty())
        SYNTAK_ERROR("Expected identifier node in "
                     "binary_func node");
    if (n->children()[0]->name() != "ident")
        SYNTAK_ERROR("Expected identifier node in "
                     "binary_func node, got "
                     << n->children()[0]->name());
    return n->children()[0]->text();
}

template <typename F>
F MathParser<F>::Private::getFuncValue(
        QString name, int num, const ParsedNode* n)
{
    int sign = 1;
    if (p->isSigned())
    {
        if (name.startsWith("-"))
            sign = -1;
        if (name.startsWith("+") || sign == -1)
            name = name.remove(0,1).trimmed();
    }
    if (!functions.contains(name))
        SYNTAK_ERROR("Unknown function '"
                     << name << "' at " << n->pos().toString());
    Function& func = functions[name];
    if ((num == 1 && !func.func1)
     || (num == 2 && !func.func2)
     || (num == 3 && !func.func3)
     || (num == 4 && !func.func4)
            )
        SYNTAK_ERROR("Unknown " << num << "-argument function '"
                     << name << "' at " << n->pos().toString());
    if (num == 1)
    {
        F p1 = takeLastValue();
        return sign * func.func1(p1);
    }
    else if (num == 2)
    {
        F p2 = takeLastValue(), p1 = takeLastValue();
        return sign * func.func2(p1, p2);
    }
    else if (num == 3)
    {
        F p3 = takeLastValue(), p2 = takeLastValue(),
          p1 = takeLastValue();
        return sign * func.func3(p1, p2, p3);
    }
    else if (num == 4)
    {
        F p4 = takeLastValue(), p3 = takeLastValue(),
          p2 = takeLastValue(), p1 = takeLastValue();
        return sign * func.func4(p1, p2, p3, p4);
    }

    SYNTAK_ERROR(num << "-argument functions not supported");
    return 0;
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
