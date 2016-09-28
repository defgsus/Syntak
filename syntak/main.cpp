
#include <QDebug>

#include "Parser.h"
#include "MathParser.h"
#include "error.h"

#define PRINT(arg__) { qDebug().noquote().nospace() << arg__; }


void simple()
{
    using namespace Syntak;

    Tokens tok;
    tok << Token("A", "A")
        << Token("B", "B");

    Rules rules;
    rules.addTokens(tok);
    rules.createAnd("seq", "S", "[S]*");
    rules.createOr ("S", "C", "D");
    rules.createAnd("C", "AorB");
    rules.createAnd("D", "A");
    rules.createOr ("AorB", "A", "B");

    rules.connect("S", [=](ParsedNode* n)
        { PRINT("--'"<<n->text()<<"' "<<n->toString()); });

    Parser p;
    p.setTokens(tok);
    p.setRules(rules);

    auto node = p.parse("AB");
    PRINT(node->toBracketString());
}



void yabnf()
{
    using namespace Syntak;

    Parser p = Parser::createYabnfParser();
    //PRINT(p.rules().toDefinitionString());

    p.rules().connect("rule", [=](ParsedNode* t)
    {
        PRINT(t->toString());
    });

    p.rules().connect("factor", [=](ParsedNode* t)
    {
        PRINT(t->toString());
    });

    QString s =
#if 1
    "expr : term [('+' | '-') term]*; "
    "term : factor [('*' | '/') factor]*; "
    "factor : '(' expr ')' | num; "
    "num : digit [digit]*; "
    "digit : '1' | '2' | '3' | '4' | '5' | '6' | '7' | "
            "'8' | '9' | '0';"
#else
    "factor : '(' expr ')'; "
    "num : digit [digit]*; "
    "digit : '1' | '2' | '3';"
#endif
            ;

    auto node = p.parse(s);
    PRINT(p.reduceTree(node)->toBracketString(true, true));
}


template <typename T>
void math()
{
    using namespace Syntak;

    MathParser<T> p;
    p.addFunction("sin", [](T a){ return std::sin(a); });
    p.addFunction("cos", [](T a){ return std::cos(a); });
    p.addFunction("floor", [](T a){ return std::floor(a); });
    p.addFunction("pow", [](T a, T b){ return std::pow(a, b); });
    p.addFunction("atan2", [](T a, T b){ return std::atan2(a, b); });
    p.addFunction("sum", [](T a, T b){ return a + b; });
    p.addFunction("diff", [](T a, T b){ return a - b; });
    p.addFunction("sum", [](T a, T b, T c){ return a + b + c; });
    p.addFunction("sum", [](T a, T b, T c, T d){ return a + b + c + d; });
    p.addFunction("asum", [](T a, T b, T c, T d){ return a - b + c - d; });
    //p.addFunction("floor", [](T a) { return std::floor(a); });
    //p.addFunction("pow", [](T a, T b) { return std::pow(a, b); });

    //T res = p.evaluate("1+2");
    //T res = p.evaluate("3*-(2+-(4+-(5+-6)))");
    //T res = p.evaluate("-(4)");
    //T res = p.evaluate("-(3+4+5)");
    //T res = p.evaluate("pow(pow(2,3), 2)");
    T res = p.evaluate("pow(2, 3)");
    PRINT("'" << p.expression() << "' = " << res);
}


int main(int, char**)
{
    try
    {
        //simple();
        //yabnf();
        math<float>();
        //PRINT(bugString.count("(") << " " << bugString.count(")"));

        /*
        QString s = "aber das '%%hallo'";
        //QRegExp ident("[A-Za-z][A-Za-z0-9]*");
        QRegExp ident("'[\x01-\x7f]*'");
        PRINT("idx=" << ident.indexIn(s));
        PRINT("len=" << ident.matchedLength());
        PRINT("'" << s.mid(ident.indexIn(s), ident.matchedLength()) << "'");
        */
        return 0;
    }
    catch (Syntak::SyntakException e)
    {
        PRINT(e.text());
        return -1;
    }
}

