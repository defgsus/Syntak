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

#include <QString>
#include <QtTest>
#include <QTime>

#include "MathParser.h"
#include "SimpleMathParser.h"

//using namespace Syntak;



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
    char* toString(const LexxedToken& p)
    {
        return toString(p.name());
    }

    template <>
    char* toString(const ParsedToken& p)
    {
        return toString(p.toString());
    }

} // namespace QTest


class SyntakTestMath : public QObject
{
    Q_OBJECT

public:
    SyntakTestMath() { }

    static QString randomExpression(
            int minRecurseLevel, int maxRecurseLevel);
    static QString randomTerm(int recursionLevel);
    static QString randomFactor(int recursionLevel);

private slots:

    void testSimple();
    void testBasic();
    void testBigRandomExpressions();
};


QString SyntakTestMath::randomExpression(
        int minLevel, int maxLevel)
{
    return randomTerm(minLevel + (rand()%maxLevel));
}

QString SyntakTestMath::randomTerm(int recursionLevel)
{
    QString s = randomFactor(recursionLevel);
    for (int i=0; i<rand()%5; ++i)
    {
        switch (rand()%4)
        {
            case 0: s += "+"; break;
            case 1: s += "-"; break;
            case 2: s += "*"; break;
            case 3: s += "/"; break;
        }
        s += randomFactor(recursionLevel);
    }
    return s;
}

QString SyntakTestMath::randomFactor(int recursionLevel)
{
    if (recursionLevel < 1 || rand() % 10 == 0)
        return QString("%1").arg(rand()%1000);
    return "(" + randomTerm(rand()%recursionLevel) + ")";
}





void SyntakTestMath::testSimple()
{
#define SYNTAK__COMP(expr__) \
    { p.parse(#expr__); \
      int stack = p.takeLastInt(); \
      PRINT(p.parser.numNodesVisited() << " nodes in " \
            << p.parser.text() << " = " << stack); \
      if (stack != (expr__)) p.print(); \
      QCOMPARE(stack, (expr__)); }

    SimpleMathParser p;
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

#undef SYNTAK__COMP
#undef SYNTAK__COMP_VAR
}


void SyntakTestMath::testBasic()
{
#define SYNTAK__COMP_VAR(var__, int__) \
    { if (!p.variables.contains(var__)) \
        { p.print(); PARSE_ERROR("variable '" << var__ << "' not found"); } \
      if (p.variables[var__] != (int__)) p.print(); \
      QCOMPARE(p.variables[var__], (int__)); }

#define SYNTAK__COMP(expr__) \
    p.parse("result= " #expr__ ";"); \
    SYNTAK__COMP_VAR("result", (expr__)); \
    PRINT(p.parser.numNodesVisited() << " nodes in '" #expr__ "' = " \
          << p.variables["result"]);

    MathParser p;
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

void SyntakTestMath::testBigRandomExpressions()
{
    QStringList exps;
    for (int i=0; i<1000; ++i)
        exps << randomExpression(10, 50);

    SimpleMathParser p;
    QTime time;
    time.start();
    size_t numNodes = 0;
    for (auto& exp : exps)
    {
        p.parse( exp );
        numNodes += p.parser.numNodesVisited();
        PRINT(p.parser.numNodesVisited() << " nodes in '"
              << p.parser.text().left(20) << "...' = " \
              << p.takeLastInt());
    }
    int e = time.elapsed();
    PRINT(size_t(numNodes / (0.001*e)) << " per second");
}

QTEST_APPLESS_MAIN(SyntakTestMath)

#include "main.moc"
