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

    static QString randomExpression(bool asFloat,
            int minRecurseLevel, int maxRecurseLevel);
    static QString randomTerm(int recursionLevel, bool asFloat);
    static QString randomFactor(int recursionLevel, bool asFloat);

    template <typename T>
    void testBigRandomExpressionsImpl(bool asFloat);

private slots:

    void testUInt();
    void testInt();
    void testDouble();
    void testFunctions();
    void testBigRandomExpressionsUInt();
    void testBigRandomExpressionsInt();
    void testBigRandomExpressionsDouble();
    void testAdvanced();
};


QString SyntakTestMath::randomExpression(bool asFloat,
        int minLevel, int maxLevel)
{
    return randomTerm(minLevel + (rand()%maxLevel), asFloat);
}

QString SyntakTestMath::randomTerm(int recursionLevel, bool asFloat)
{
    QString s = randomFactor(recursionLevel, asFloat);
    for (int i=0; i<rand()%5; ++i)
    {
        switch (rand()%4)
        {
            case 0: s += "+"; break;
            case 1: s += "-"; break;
            case 2: s += "*"; break;
            case 3: s += "/"; break;
        }
        s += randomFactor(recursionLevel, asFloat);
    }
    return s;
}

QString SyntakTestMath::randomFactor(int recursionLevel, bool asFloat)
{
    if (recursionLevel < 1 || rand() % 10 == 0)
    {
        QString s = QString("%1").arg(rand()%1000);
        if (!asFloat)
            return s;
        if (rand()%8 == 0)
            s.prepend("-");
        if (rand()%2 == 0)
            s += QString(".%1").arg(rand()%1000000);
        if (rand()%10 == 0)
            s += QString("e%1%2")
                    .arg(rand()%2 ? "+":"-")
                    .arg(1 + rand()%20);
        return s;
    }
    return "(" + randomTerm(rand()%recursionLevel, asFloat) + ")";
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

#undef SYNTAK__COMP
#undef SYNTAK__COMP_VAR
}

void SyntakTestMath::testFunctions()
{
    typedef double T;
#define SYNTAK__COMP(expr__) \
    { T res = p.evaluate(#expr__); \
      PRINT(p.parser().numNodesVisited() << " nodes in " \
            << p.parser().text() << " = " << res); \
      QCOMPARE(res, T(expr__)); }


    MathParser<double> p;
    p.addFunction("sin", [](T a){ return std::sin(a); });
    p.addFunction("pow", [](T a, T b){ return std::pow(a, b); });

    using namespace std;

    SYNTAK__COMP( sin(3.14159265) );
    SYNTAK__COMP( pow(2., 3.) );

#undef SYNTAK__COMP
#undef SYNTAK__COMP_VAR
}


void SyntakTestMath::testBigRandomExpressionsUInt()
{
    testBigRandomExpressionsImpl<uint64_t>(false);
}

void SyntakTestMath::testBigRandomExpressionsInt()
{
    testBigRandomExpressionsImpl<int64_t>(false);
}

void SyntakTestMath::testBigRandomExpressionsDouble()
{
    testBigRandomExpressionsImpl<double>(true);
}

template <typename T>
void SyntakTestMath::testBigRandomExpressionsImpl(bool asFloat)
{
    QStringList exps;
    for (int i=0; i<1000; ++i)
        exps << randomExpression(asFloat, 10, 50);

    MathParser<T> p;
    p.setIgnoreDivisionByZero(true);

    QTime time;
    time.start();
    size_t numNodesSum = 0, maxNumNodes = 0;
    QString heavyString;
    for (auto& exp : exps)
    {
        double res = p.evaluate( exp );
        numNodesSum += p.parser().numNodesVisited();
        if (p.parser().numNodesVisited() > maxNumNodes)
            maxNumNodes = p.parser().numNodesVisited(),
            heavyString = p.parser().text();
#if 0
        PRINT(p.parser().numNodesVisited() << " nodes in '"
              << p.parser().text().left(20) << "...' = " \
              << res);
#else
        Q_UNUSED(res);
#endif
    }
    int e = time.elapsed();
    PRINT(size_t(numNodesSum / (0.001*e)) << " nodes per second");
    PRINT("max number nodes: " << maxNumNodes << " in string of length "
          << heavyString.size() << ":");
    PRINT(heavyString.left(60) << "...");
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
