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
#include "MathParser.h"

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

private slots:

    void testBasics();
};

void SyntakTestMath::testBasics()
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

    SYNTAK__COMP( +1 );
    SYNTAK__COMP( -1 );
    SYNTAK__COMP( -1 + -2 );
    SYNTAK__COMP( -(1) );
    SYNTAK__COMP( -1+-2 );
    SYNTAK__COMP( -1*-2 );

    SYNTAK__COMP( -(-(1+-2)*-3) );

    SYNTAK__COMP( 1+2*3 );
    SYNTAK__COMP( 1-6/3 );
    SYNTAK__COMP( (1+2)*3 );
    SYNTAK__COMP( 1+2+3+4+5+6+7*8*9 );
    SYNTAK__COMP( ((((((1+2)*3+4)*5+6)*7+8*9+10)*11+12)*13+14)*15 );

}


QTEST_APPLESS_MAIN(SyntakTestMath)

#include "main.moc"
