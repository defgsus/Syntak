
#include <QDebug>

#include "Parser.h"

#define PRINT(arg__) { qDebug().noquote().nospace() << arg__; }


void simple()
{
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

    Parser p;
    p.setTokens(tok);
    p.setRules(rules);

    auto node = p.parse("AB");
    PRINT(node->toBracketString());
}



void yabnf()
{
    Parser p = Parser::createYabnfParser();
    //PRINT(p.rules().toDefinitionString());

    p.rules().connect("rule", [=](const ParsedToken& t)
    {
        PRINT(t.toString());
    });

    p.rules().connect("factor", [=](const ParsedToken& t)
    {
        PRINT(t.toString());
    });

    QString s =
#if 0
    "expr : term [('+' | '-') term]*; "
    "term : factor [('*' | '/') factor]*; "
    "factor : '(' expr ')' | num; "
    "num : digit [digit]*; "
    "digit : '1' | '2' | '3' | '4' | '5' | '6' | '7' | "
            "'8' | '9' | '0';"
#else
    //"factor : '(' expr ')'; "
    "num : digit [digit]*; "
    "digit : '1' | '2' | '3';"
#endif
            ;

    std::vector<LexxedToken> lt;
    p.tokens().tokenize(s, lt);
    PRINT(Tokens::toString(lt));
    auto node = p.parse(s);
    PRINT(node->toBracketString());
}


int main(int, char**)
{
    //simple();
    yabnf();
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

