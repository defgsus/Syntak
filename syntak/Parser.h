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

#ifndef PARSER_H
#define PARSER_H

#include "Tokens.h"
#include "Rules.h"




class Parser
{
public:
    Parser();

    const Rules& rules() const { return p_rules; }
    const Tokens& lexxer() const { return p_lexxer; }

    void setRules(const Rules& r) { p_rules = r; p_rules.check(); }
    void setLexxer(const Tokens& t) { p_lexxer = t; }

    void parse(const QString& text);
    int numNodesVisited() const { return p_visited; }
    const QString& text() const { return p_text; }

    bool parseRule(const Rule* r, const Rule* parent=nullptr, int subIdx=-1);
    bool parseRule_(const Rule* r);

    const LexxedToken& curToken() const { return p_look; }
    bool forward();
    void setPos(size_t);
    void pushPos() { p_posStack.push_back(p_lookPos); }
    void popPos();

private:
    Rules p_rules;
    Tokens p_lexxer;
    QString p_text;
    std::vector<LexxedToken> p_tokens;
    std::vector<size_t> p_posStack;
    LexxedToken p_look;
    size_t p_lookPos;
    int p_level, p_visited;
};

class ParsedToken
{
public:
    ParsedToken() : p_rule(nullptr) { }

    bool isValid() const { return !p_text.isEmpty(); }

    const SourcePos& pos() const { return p_pos; }
    const QString& text() const { return p_text; }
    const Rule* rule() const { return p_rule; }

    QString toString() const
        { return QString("%1 \"%2\" @ %3")
                .arg(rule() ? rule()->name() : QString("NULL"))
                .arg(text())
                .arg(pos().toString()); }
private:
    friend class Parser;
    SourcePos p_pos;
    QString p_text;
    const Rule* p_rule;
};

#endif // PARSER_H
