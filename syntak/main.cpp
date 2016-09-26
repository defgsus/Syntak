
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

