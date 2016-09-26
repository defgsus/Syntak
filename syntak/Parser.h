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


class ParsedNode;

class Parser
{
public:
    Parser();

    const Rules& rules() const { return p_rules; }
    Rules& rules() { return p_rules; }
    const Tokens& tokens() const { return p_lexxer; }

    void setRules(const Rules& r) { p_rules = r; p_rules.check(); }
    void setTokens(const Tokens& t) { p_lexxer = t; }

    ParsedNode* parse(const QString& text);
    int numNodesVisited() const { return p_visited; }
    const QString& text() const { return p_text; }
    const std::vector<LexxedToken>& lexxedTokens() const
        { return p_tokens; }
    bool parseRule(ParsedNode* node, int subIdx=-1);
    bool parseRule_(ParsedNode* node);

    const LexxedToken& curToken() const { return p_look; }
    bool forward();
    void setPos(size_t);
    void pushPos() { p_posStack.push_back(p_lookPos); }
    void popPos();
    size_t lengthSince(size_t sourcePos) const;

    static Parser createYabnfParser();

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

class ParsedNode
{
public:
    ParsedNode() : p_length(0), p_parent(nullptr), p_rule(nullptr) { }
    ~ParsedNode();

    bool isValid() const { return p_rule; }

    const SourcePos& pos() const { return p_pos; }
    int length() const { return p_length; }
    const Rule* rule() const { return p_rule; }
    ParsedNode* parent() const { return p_parent; }
    QString name() const { return rule() ? rule()->name() : QString(); }

    bool contains(ParsedNode* n) const;
    const std::vector<ParsedNode*>& children() const { return p_children; }
    int numChildLevels() const;

    QString toString() const;
    QString toBracketString() const;

private:
    friend class Parser;
    void p_add(ParsedNode*n);
    void p_add(const std::vector<ParsedNode*>& n);
    SourcePos p_pos;
    int p_length, p_nextTokenPos;
    ParsedNode* p_parent;
    std::vector<ParsedNode*> p_children;
    const Rule* p_rule;
};

#endif // PARSER_H
