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

#include <cmath>

#include <QString>
#include <QtTest>
#include <QTime>

#include "MathParser.h"
#include "AdvancedMathParser.h"

using namespace Syntak;


namespace QTest {

    template <>
    char* toString(const Rule& p)
    {
        return toString(p.toDefinitionString());
    }

    template <>
    char* toString(const Token& p)
    {
        return toString(p.tokenString());
    }

    template <>
    char* toString(const ParsedToken& p)
    {
        return toString(p.name());
    }

    template <>
    char* toString(const ParsedNode& p)
    {
        return toString(p.toString());
    }

} // namespace QTest


class SyntakTestMath : public QObject
{
    Q_OBJECT

public:
    SyntakTestMath() { }

    template <typename T>
    static QString randomExpression(
            int minRecurseLevel, int maxRecurseLevel,
            const MathParser<T>& p);
    template <typename T>
    static QString randomTerm(
            int recursionLevel, const MathParser<T>& p);
    template <typename T>
    static QString randomFactor(
            int recursionLevel, const MathParser<T>& p);

    template <typename T>
    void testBigRandomExpressionsImpl(
            MathParser<T> p = MathParser<T>());

private slots:

    void testUInt();
    void testInt();
    void testDouble();
    void testConstants();
    void testFunctions();
    void testFunctionsAndConstants();
    void testBigRandomUInt();
    void testBigRandomInt();
    void testBigRandomDouble();
    void testBigRandomDoubleConstants();
    void testBigRandomDoubleFuncs();
    void testBigRandomDoubleFuncsConstants();
    void testAdvanced();
};

template <typename T>
QString SyntakTestMath::randomExpression(
        int minLevel, int maxLevel, const MathParser<T>& p)
{
    return randomTerm(minLevel + (rand()%maxLevel), p);
}

template <typename T>
QString SyntakTestMath::randomTerm(
        int recursionLevel, const MathParser<T>& p)
{
    if (!p.hasFunctions() || rand()%10 < 6)
    {
        QString s = randomFactor(recursionLevel, p);
        for (int i=0; i<rand()%5; ++i)
        {
            switch (rand()%4)
            {
                case 0: s += "+"; break;
                case 1: s += "-"; break;
                case 2: s += "*"; break;
                case 3: s += "/"; break;
            }
            s += randomFactor(recursionLevel, p);
        }
        return s;
    }
    else
    {
        int count = 1 + rand()%4;
        QStringList funcs = p.getFunctionNames(count);
        if (funcs.isEmpty())
            return randomFactor(recursionLevel, p);
        QString s = funcs[rand()%funcs.size()];
        s += "(" + randomFactor(recursionLevel, p);
        for (int i=1; i < count; ++i)
            s += "," + randomFactor(recursionLevel, p);
        s += ")";
        return s;
    }
}

template <typename T>
QString SyntakTestMath::randomFactor(
        int recursionLevel, const MathParser<T>& p)
{
    if (recursionLevel < 1 || rand() % 10 == 0)
    {
        if (!p.constants().isEmpty() && rand() % 4 == 0)
        {
            auto keys = p.constants().keys();
            QString s = keys[rand()%keys.size()];
            if (p.isSigned() && rand()%8 == 0)
                s.prepend("-");
            return s;
        }
        QString s = QString("%1").arg(rand()%1000);
        if (p.isSigned() && rand()%8 == 0)
            s.prepend("-");
        if (!p.isFloat())
            return s;
        if (rand()%2 == 0)
            s += QString(".%1").arg(rand()%1000000);
        if (rand()%10 == 0)
            s += QString("e%1%2")
                    .arg(rand()%2 ? "+":"-")
                    .arg(1 + rand()%20);
        return s;
    }
    return "(" + randomTerm(rand()%recursionLevel, p) + ")";
}





void SyntakTestMath::testUInt()
{
#define SYNTAK__COMP(expr__) \
    { auto res = p.evaluate(#expr__); \
      PRINT(p.parser().numNodesVisited() << " nodes in " \
            << p.parser().text() << " = " << res); \
      QCOMPARE(res, (decltype(res))(expr__)); }

    MathParser<uint> p;
    SYNTAK__COMP( 1 );
    SYNTAK__COMP( 123 );
    SYNTAK__COMP( 12345 );

    SYNTAK__COMP( 1+2 );
    SYNTAK__COMP( 2-1 );
    SYNTAK__COMP( 2*3 );
    SYNTAK__COMP( 6/3 );

    SYNTAK__COMP( 1+2*3 );
    SYNTAK__COMP( 3-6/3 );
    SYNTAK__COMP( (1+2)*3 );
    SYNTAK__COMP( 1+2+3+4+5+6+7*8*9 );
    SYNTAK__COMP( (((((((1+2)*3+4)*5+6)*7+8)*9+10)*11+12)*13+14)*15 );

#undef SYNTAK__COMP
#undef SYNTAK__COMP_VAR
}


void SyntakTestMath::testInt()
{
#define SYNTAK__COMP(expr__) \
    { int res = p.evaluate(#expr__); \
      PRINT(p.parser().numNodesVisited() << " nodes in " \
            << p.parser().text() << " = " << res); \
      QCOMPARE(res, (expr__)); }

    MathParser<int> p;
    SYNTAK__COMP( 1 );
    SYNTAK__COMP( 123 );
    SYNTAK__COMP( 12345 );

    SYNTAK__COMP( 1+2 );
    SYNTAK__COMP( 1-2 );
    SYNTAK__COMP( 2*3 );
    SYNTAK__COMP( 6/3 );

    SYNTAK__COMP( 1+2*3 );
    SYNTAK__COMP( 1-6/3 );
    SYNTAK__COMP( (1+2)*3 );
    SYNTAK__COMP( 1+2+3+4+5+6+7*8*9 );
    SYNTAK__COMP( (((((((1+2)*3+4)*5+6)*7+8)*9+10)*11+12)*13+14)*15 );

    SYNTAK__COMP( 3 % 2 );
    SYNTAK__COMP( 9 % 5 % 3 );
    SYNTAK__COMP( (1+4) % (2+1) % 2 );

#undef SYNTAK__COMP
#undef SYNTAK__COMP_VAR
}


void SyntakTestMath::testDouble()
{
#define SYNTAK__COMP(expr__) \
    { double res = p.evaluate(#expr__); \
      PRINT(p.parser().numNodesVisited() << " nodes in " \
            << p.parser().text() << " = " << res); \
      QCOMPARE(res, double(expr__)); }

    MathParser<double> p;
    SYNTAK__COMP( 1 );
    SYNTAK__COMP( 1.0 );
    SYNTAK__COMP( 1. );
    SYNTAK__COMP( .1 );
    SYNTAK__COMP( .123 );
    SYNTAK__COMP( 123. );
    SYNTAK__COMP( 123.45 );
    SYNTAK__COMP( 1234.56789 );

    SYNTAK__COMP( 1e+10 );
    SYNTAK__COMP( 3.456e+21 );
    SYNTAK__COMP( 3.456e-11 );

    SYNTAK__COMP( -1 );
    SYNTAK__COMP( -1.0 );
    SYNTAK__COMP( -1. );
    SYNTAK__COMP( -.1 );
    SYNTAK__COMP( -.123 );
    SYNTAK__COMP( -123. );
    SYNTAK__COMP( -123.45 );
    SYNTAK__COMP( -1234.56789 );

    SYNTAK__COMP( -1e+10 );
    SYNTAK__COMP( -3.456e+21 );
    SYNTAK__COMP( -3.456e-11 );

    SYNTAK__COMP( 1+2 );
    SYNTAK__COMP( 1-2 );
    SYNTAK__COMP( 2*3 );
    SYNTAK__COMP( 6/3 );

    SYNTAK__COMP( 1.2+3.4 );
    SYNTAK__COMP( 1.2-3.4 );
    SYNTAK__COMP( 2.3*4.5 );
    SYNTAK__COMP( 6./4. );

    SYNTAK__COMP( 1+2*3 );
    SYNTAK__COMP( 1-6/3 );
    SYNTAK__COMP( (1+2)*3 );
    SYNTAK__COMP( 1+2+3+4+5+6+7*8*9 );
    SYNTAK__COMP( (((((((1+2)*3+4)*5+6)*7+8)*9+10)*11+12)*13+14)*15 );

    SYNTAK__COMP( 1+-2 );
    SYNTAK__COMP( 1+ +2 );
    SYNTAK__COMP( 3*+2 );
    SYNTAK__COMP( 3*-2 );
    SYNTAK__COMP( 3./+2 );
    SYNTAK__COMP( 3./-2 );

    SYNTAK__COMP( -(3) );
    SYNTAK__COMP( 3 * -(2) );
    SYNTAK__COMP( 3*-(-(2)) );
    SYNTAK__COMP( 3*-(2+-(4+-(5+-6))) );
    SYNTAK__COMP( -(3+4+5) );

    SYNTAK__COMP( 199.120581e+18/-526+(76) );

#undef SYNTAK__COMP
#undef SYNTAK__COMP_VAR
}

void SyntakTestMath::testConstants()
{
    typedef double T;
#define SYNTAK__COMP(expr__) \
    { double res = p.evaluate(#expr__); \
      PRINT(p.parser().numNodesVisited() << " nodes in " \
            << p.parser().text() << " = " << res); \
      QCOMPARE(res, double(expr__)); }

    const T PI = 3.14;
    const T E = 2.7;
    const T PHI = 1.618;
    const T UNKNOWN = 0;

    MathParser<double> p;
    p.addConstant("PI",     PI);
    p.addConstant("E",      E);
    p.addConstant("PHI",    PHI);

    SYNTAK__COMP( PI );
    SYNTAK__COMP( -PI );
    SYNTAK__COMP( - PI );
    SYNTAK__COMP( 2*E );
    SYNTAK__COMP( PHI*4 );
    SYNTAK__COMP( PI/PHI );
    SYNTAK__COMP( -PI / -PHI );
    try
    {
        SYNTAK__COMP( PI * UNKNOWN );
        QFAIL("No error for unknown identifier generated");
    }
    catch (SyntakException e)
    {
        PRINT(e.text());
    }

#undef SYNTAK__COMP
#undef SYNTAK__COMP_VAR
}

void SyntakTestMath::testFunctions()
{
    typedef double T;
#define SYNTAK__COMP(expr__, cexpr__) \
    { T res = p.evaluate(#expr__); \
      PRINT(p.parser().numNodesVisited() << " nodes in " \
            << p.parser().text() << " = " << res); \
      QCOMPARE(res, T(cexpr__)); }


    MathParser<double> p;
    p.addFunction("sin", [](T a){ return std::sin(a); });
    p.addFunction("pow", [](T a, T b){ return std::pow(a, b); });
    p.addFunction("sum", [](T a, T b, T c){ return a + b + c; });
    p.addFunction("sum", [](T a, T b, T c, T d){ return a + b + c + d; });

    SYNTAK__COMP( sin(3.14159265),      std::sin(3.14159265) );
    SYNTAK__COMP( pow(2, 3),            std::pow(2., 3.) );
    SYNTAK__COMP( sum(2, 3, 4),         2+3+4 );
    SYNTAK__COMP( sum(2, 3, 4, 5),      2+3+4+5 );
    SYNTAK__COMP( sum(2,3,sum(4,5,6)),  2+3+4+5+6 );

#undef SYNTAK__COMP
#undef SYNTAK__COMP_VAR
}

void SyntakTestMath::testFunctionsAndConstants()
{
    typedef double T;
#define SYNTAK__COMP(expr__, cexpr__) \
    { T res = p.evaluate(#expr__); \
      PRINT(p.parser().numNodesVisited() << " nodes in " \
            << p.parser().text() << " = " << res); \
      QCOMPARE(res, T(cexpr__)); }

    const T sin = 1.5;

    // test name-clash between function and constant identifiers
    MathParser<double> p;
    p.addFunction("sin", [](T a){ return std::sin(a); });
    p.addConstant("sin", sin);

    SYNTAK__COMP( sin,              sin );
    SYNTAK__COMP( sin(1.),          std::sin(1.) );
    SYNTAK__COMP( sin * sin(1.),    sin * std::sin(1.) );
    SYNTAK__COMP( sin(sin),         std::sin(sin) );

#undef SYNTAK__COMP
#undef SYNTAK__COMP_VAR
}


void SyntakTestMath::testBigRandomUInt()
{
    testBigRandomExpressionsImpl<uint64_t>();
}

void SyntakTestMath::testBigRandomInt()
{
    testBigRandomExpressionsImpl<int64_t>();
}

void SyntakTestMath::testBigRandomDouble()
{
    testBigRandomExpressionsImpl<double>();
}

void SyntakTestMath::testBigRandomDoubleConstants()
{
    typedef double T;
    MathParser<T> p;
    p.addConstant("PI", 3.14159265);
    p.addConstant("e", 2.7);
    p.addConstant("phi", 0.618);
    p.addConstant("PHI", 1.618);
    p.addConstant("root2", std::sqrt(2.));
    p.addConstant("root3", std::sqrt(3.));

    testBigRandomExpressionsImpl<T>(p);
}

void SyntakTestMath::testBigRandomDoubleFuncs()
{
    typedef double T;
    MathParser<T> p;
    p.addFunction("sin", [](T a){ return std::sin(a); });
    p.addFunction("cos", [](T a){ return std::cos(a); });
    p.addFunction("floor", [](T a){ return std::floor(a); });
    p.addFunction("pow", [](T a, T b){ return std::pow(a, b); });
    p.addFunction("atan2", [](T a, T b){ return std::atan2(a, b); });
    p.addFunction("sum", [](T a, T b){ return a + b; });
    p.addFunction("diff", [](T a, T b){ return a - b; });
    p.addFunction("sum", [](T a, T b, T c){ return a + b + c; });
    p.addFunction("_sum", [](T a, T b, T c, T d){ return a + b + c + d; });
    p.addFunction("a_sum", [](T a, T b, T c, T d){ return a-b+c-d; });

    testBigRandomExpressionsImpl<T>(p);
}

void SyntakTestMath::testBigRandomDoubleFuncsConstants()
{
    typedef double T;
    MathParser<T> p;
    p.addFunction("sin", [](T a){ return std::sin(a); });
    p.addFunction("cos", [](T a){ return std::cos(a); });
    p.addFunction("floor", [](T a){ return std::floor(a); });
    p.addFunction("pow", [](T a, T b){ return std::pow(a, b); });
    p.addFunction("atan2", [](T a, T b){ return std::atan2(a, b); });
    p.addFunction("sum", [](T a, T b){ return a + b; });
    p.addFunction("diff", [](T a, T b){ return a - b; });
    p.addFunction("sum", [](T a, T b, T c){ return a + b + c; });
    p.addFunction("_sum", [](T a, T b, T c, T d){ return a + b + c + d; });
    p.addFunction("a_sum", [](T a, T b, T c, T d){ return a-b+c-d; });

    p.addConstant("PI", 3.14159265);
    p.addConstant("e", 2.7);
    p.addConstant("phi", 0.618);
    p.addConstant("PHI", 1.618);
    p.addConstant("root2", std::sqrt(2.));
    p.addConstant("root3", std::sqrt(3.));
    // test name-clash between function and constant identifiers
    p.addConstant("atan2", 23.);
    p.addConstant("sum", 42.);

    testBigRandomExpressionsImpl<T>(p);
}

template <typename T>
void SyntakTestMath::testBigRandomExpressionsImpl(MathParser<T> p)
{
    try
    {
        /*
        p.init();
        PRINT("RULES:\n" << p.parser().rules().toDefinitionString());
        PRINT("TOP-LEVEL: " << p.parser().rules().topRule()->toString());
        */

        QStringList exps;
        for (int i=0; i<200; ++i)
            exps << randomExpression(10, 50, p);

        p.setIgnoreDivisionByZero(true);

        QTime time;
        time.start();
        size_t numNodesSum = 0, maxNumNodes = 0;
        double lengthNodeFactor = 0.;
        QString heavyString;
        for (auto& exp : exps)
        {
            double res = p.evaluate( exp );
            numNodesSum += p.parser().numNodesVisited();
            if (p.parser().numNodesVisited() > maxNumNodes)
                maxNumNodes = p.parser().numNodesVisited(),
                heavyString = p.parser().text();
            double fac = double(p.parser().numNodesVisited())
                            / exp.size();
            lengthNodeFactor += fac;
    #if 0
            PRINT(p.parser().numNodesVisited() << " nodes in '"
                  << p.parser().text().left(20) << "...' = " \
                  << res);
    #else
            Q_UNUSED(res);
    #endif
        }
        int e = time.elapsed();
        lengthNodeFactor /= exps.size();

        PRINT(size_t(numNodesSum / (0.001*e)) << " nodes per second");
        PRINT("average length-to-nodes factor: " << lengthNodeFactor);
        PRINT("max number nodes: " << maxNumNodes << " in string of length "
              << heavyString.size() << ":");
        PRINT(heavyString.left(60) << "...");
    }
    catch (SyntakException e)
    {
        qWarning() << e.text();
        QFAIL("Caught exception");
    }
}

void SyntakTestMath::testAdvanced()
{
    return;

#define SYNTAK__COMP_VAR(var__, int__) \
    { if (!p.variables.contains(var__)) \
        { p.print(); SYNTAK_ERROR("variable '" << var__ << "' not found"); } \
      if (p.variables[var__] != (int__)) p.print(); \
      QCOMPARE(p.variables[var__], (int__)); }

#define SYNTAK__COMP(expr__) \
    p.parse("result= " #expr__ ";"); \
    SYNTAK__COMP_VAR("result", (expr__)); \
    PRINT(p.parser.numNodesVisited() << " nodes in '" #expr__ "' = " \
          << p.variables["result"]);

    AdvancedMathParser p;
    SYNTAK__COMP( 1+2 );
    SYNTAK__COMP( 1-2 );
    SYNTAK__COMP( 2*3 );
    SYNTAK__COMP( 6/3 );

    SYNTAK__COMP( 1+2*3 );
    SYNTAK__COMP( 1-6/3 );
    SYNTAK__COMP( (1+2)*3 );
    SYNTAK__COMP( 1+2+3+4+5+6+7*8*9 );
    SYNTAK__COMP( (((((((1+2)*3+4)*5+6)*7+8)*9+10)*11+12)*13+14)*15 );

    /*
    SYNTAK__COMP( +1 );
    SYNTAK__COMP( -1 );
    SYNTAK__COMP( -1 + -2 );
    SYNTAK__COMP( -1+-2 );
    SYNTAK__COMP( -1*-2 );

    SYNTAK__COMP( -(1) );
    SYNTAK__COMP( -(-(1+-2)*-3) );
    */
}


QTEST_APPLESS_MAIN(SyntakTestMath)

#include "main.moc"
