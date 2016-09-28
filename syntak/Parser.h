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

#ifndef SYNTAKSRC_SYNTAK_PARSER_H
#define SYNTAKSRC_SYNTAK_PARSER_H

#include "Tokens.h"
#include "Rules.h"

namespace Syntak {

class ParsedNode;

class Parser
{
public:
    Parser();
    Parser(const Parser&);
    ~Parser();

    Parser& operator=(const Parser&);

    // ---- getter ----

    const Rules& rules() const;
    const Tokenizer& tokenizer() const;
    int numNodesVisited() const;
    const QString& text() const;

    // ---- setter ----

    void setRules(const Rules& r);
    void setTokens(const Tokens& t);

    Rules& rules();

    // ---- parsing ----

    /** Returns the complete parse tree for the given text */
    ParsedNode* parse(const QString& text);

    /** Reduces the parse tree in @p root to a syntax tree,
        e.g. it reduces each node to the deepest leave node
        if possible. */
    ParsedNode* reduceTree(const ParsedNode* root);

    static Parser createYabnfParser();

private:

    struct Private;
    Private* p_;
};


class ParsedNode
{
public:
    ParsedNode()
        : p_parser(nullptr)
        , p_rule(nullptr)
        , p_isEmitted(false)
        , p_length(0)
        , p_parent(nullptr) { }

    ~ParsedNode();

    bool isValid() const { return p_parser && p_rule; }
    bool isEmitted() const { return p_isEmitted; }
    bool isLeave() const { return p_children.empty(); }
    bool isRoot() const { return p_parent == nullptr; }

    const SourcePos& pos() const { return p_pos; }
    int length() const { return p_length; }
    const Rule* rule() const { return p_rule; }
    ParsedNode* parent() const { return p_parent; }
    QString name() const { return rule() ? rule()->name() : QString(); }
    Parser* parser() const { return p_parser; }
    QString text() const;

    bool contains(const ParsedNode* n) const;
    const std::vector<ParsedNode*>& children() const { return p_children; }
    int numChildLevels() const;

    QString toString() const;
    QString toBracketString(bool withContent=false,
                            bool withLineBeaks = false) const;

private:
    friend class Parser;
    void p_add(ParsedNode*n);
    void p_add(const std::vector<ParsedNode*>& n);
    Parser* p_parser;
    const Rule* p_rule;
    SourcePos p_pos;
    bool p_isEmitted;
    int p_length, p_nextTokenPos;
    ParsedNode* p_parent;
    std::vector<ParsedNode*> p_children;
};

} // namespace Syntak

#endif // SYNTAKSRC_SYNTAK_PARSER_H
