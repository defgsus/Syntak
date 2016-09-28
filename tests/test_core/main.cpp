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

#include "Parser.h"

using namespace Syntak;

#if 1
#   include <QDebug>
#   define PRINT(arg__) \
        { qDebug().noquote().nospace() << arg__; }
#else
#   define PRINT(unused__) { }
#endif


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


class SyntakTestCore : public QObject
{
    Q_OBJECT

public:
    SyntakTestCore() { }


private slots:

    void testTokenizerSingle();
    void testTokenizerMulti();
    void testTokenizerRegex();
    void testTokenizerRegex2();
    void testTokenizerMultiSpeed();

};



void SyntakTestCore::testTokenizerSingle()
{
    Tokens tok;
    tok << Token("d0", "0")
        << Token("d1", "1")
        << Token("d2", "2")
        << Token("d3", "3")
        << Token("d4", "4")
        << Token("d5", "5")
        << Token("d6", "6")
        << Token("d7", "7")
        << Token("d8", "8")
        << Token("d9", "9")
    ;
    Tokenizer lex(tok);

    lex.tokenize("0123456789");
    QString s = lex.toString(),
            e = QString("d0(0)@0 d1(1)@1 d2(2)@2 d3(3)@3 d4(4)@4 d5(5)@5 "
                        "d6(6)@6 d7(7)@7 d8(8)@8 d9(9)@9 EOF()@10 ");
    QCOMPARE(s, e);

    lex.tokenize("");
    s = lex.toString();
    e = QString("EOF()@0 ");
    QCOMPARE(s, e);

    lex.tokenize("1 2 3");
    s = lex.toString();
    e = QString("d1(1)@0 d2(2)@2 d3(3)@4 EOF()@5 ");
    QCOMPARE(s, e);
}

void SyntakTestCore::testTokenizerMulti()
{
    Tokens tok;
    tok << Token("a", "ab")
        << Token("c", "cde")
        << Token("f", "fghi")
        << Token("j", "jklmn")
    ;
    Tokenizer lex(tok);

    lex.tokenize("abcdefghijklmn");
    QString s = lex.toString(),
            e = QString("a(ab)@0 c(cde)@2 f(fghi)@5 j(jklmn)@9 "
                        "EOF()@14 ");
    QCOMPARE(s, e);

    lex.tokenize("jklm");
    s = lex.toString();
    e = QString("EOF()@4 ");
    QCOMPARE(s, e);

}


void SyntakTestCore::testTokenizerRegex()
{
    Tokens tok;
    tok << Token("d", QRegExp("[0-9]"))
        << Token("c", QRegExp("[A-Za-z]"))
        << Token("q", QRegExp("'[\x01-\xff]*'"))
    ;
    Tokenizer lex(tok);
    QString s, e;

    lex.tokenize("123abc'quoted'");
    s = lex.toString();
    e = QString("d(1)@0 d(2)@1 d(3)@2 c(a)@3 c(b)@4 c(c)@5 "
                "q('quoted')@6 EOF()@14 ");
    QCOMPARE(s, e);

    lex.tokenize("'quot");
    s = lex.toString();
    e = QString("c(q)@1 c(u)@2 c(o)@3 c(t)@4 EOF()@5 ");
    QCOMPARE(s, e);
}

void SyntakTestCore::testTokenizerRegex2()
{
    Tokens tok;
    tok << Token("dbl", QRegExp(
        "[+-]?(\\d+(\\.\\d*)?|\\.\\d+)([eE][+-]?\\d+)?"))
    ;
    Tokenizer lex(tok);
    QString s, e;

    lex.tokenize("1"); s = lex.toString();
    e = "dbl(1)@0 EOF()@1 ";
    QCOMPARE(s, e);

    lex.tokenize("1."); s = lex.toString();
    e = "dbl(1.)@0 EOF()@2 ";
    QCOMPARE(s, e);

    lex.tokenize("1.1"); s = lex.toString();
    e = "dbl(1.1)@0 EOF()@3 ";
    QCOMPARE(s, e);

    lex.tokenize("-1.1"); s = lex.toString();
    e = "dbl(-1.1)@0 EOF()@4 ";
    QCOMPARE(s, e);

    lex.tokenize("-.1"); s = lex.toString();
    e = "dbl(-.1)@0 EOF()@3 ";
    QCOMPARE(s, e);

    lex.tokenize("+.0"); s = lex.toString();
    e = "dbl(+.0)@0 EOF()@3 ";
    QCOMPARE(s, e);

    lex.tokenize("123."); s = lex.toString();
    e = "dbl(123.)@0 EOF()@4 ";
    QCOMPARE(s, e);

    lex.tokenize("123.56"); s = lex.toString();
    e = "dbl(123.56)@0 EOF()@6 ";
    QCOMPARE(s, e);

    lex.tokenize("."); s = lex.toString();
    e = "EOF()@1 ";
    QCOMPARE(s, e);

}

void SyntakTestCore::testTokenizerMultiSpeed()
{
    // create random tokens
    QStringList fixedTokens;
    fixedTokens << "hallo" << "welt";
    for (int i=0; i<50; ++i)
    {
        QString s;
        for (int j=0; j<3+rand()%30; ++j)
            s += QChar(33 + rand()%(256-33));
        fixedTokens << s;
    }

    // create tokenizer
    Tokens tok;
    for (auto& t : fixedTokens)
        tok << Token(t, t);
    Tokenizer lex(tok);

    // create random text
    QString text;
    for (int i=0; i<100000; ++i)
    {
        text += fixedTokens[rand()%fixedTokens.size()];
    }

    QTime tm;
    tm.start();
    int numChars = 0;
    for (int i=0; i<5; ++i)
    {
        numChars += text.size();
        lex.tokenize(text);
    }
    int e = tm.elapsed();

    PRINT("parsed " << numChars << " chars with "
          << tok.tokens().size() << " tokens in " << e << "ms"
          " := " << int(numChars / (.001*e)) << " chars per second."
          );
}





QTEST_APPLESS_MAIN(SyntakTestCore)

#include "main.moc"
