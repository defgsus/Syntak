
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
    rules.createAnd("seq", "S", "[seq]*");
    rules.createOr("S", "C", "D");
    rules.createOr("C", "A", "B");
    rules.createAnd("D", "A", "B");

    Parser p;
    p.setLexxer(tok);
    p.setRules(rules);

    auto node = p.parse("AB");
    PRINT(node->toBracketString());
}


int main(int, char**)
{
    simple();

    return 0;
}

