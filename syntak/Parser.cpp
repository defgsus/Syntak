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

#include "Parser.h"

#if 0
#   define P_DEBUG(arg__) \
        { QString indent__ = p_level ? QString("%1").arg(p_level) \
                                 : QString(" "); \
      indent__ += QString(" ").repeated(p_level+1 - indent__.size()); \
      qDebug().noquote().nospace() << indent__ << arg__; }
#else
#   define P_DEBUG(unused__) { }
#endif


ParsedNode::~ParsedNode()
{
    for (auto n : p_children)
        delete n;
}

bool ParsedNode::contains(ParsedNode* node) const
{
    for (auto n : p_children)
        if (n == node)
            return true;
    return false;
}

int ParsedNode::numChildLevels() const
{
    int m = 0;
    for (auto c : children())
        m = std::max(m, c->numChildLevels()+1);
    return m;
}

QString ParsedNode::toString() const
{
    if (!isValid())
        return "INVALID";
    return QString("%1(@%2-%3,d=%4)'%5'")
            .arg(rule()->name())
            .arg(pos().toString())
            .arg(pos().pos() + length())
            .arg(numChildLevels())
            .arg(text());
}

QString ParsedNode::text() const
{
    if (!isValid())
        return QString("INVALID");
    return p_parser->text().mid(pos().pos(), length());
}

QString ParsedNode::toBracketString() const
{
    if (!isValid())
        return QString("INVALID");

    QString s = name();
    if (!children().empty())
    {
        s += "{";
        for (auto c : children())
            s += " " + c->toBracketString();
        s += " }";
    }
    return s;
}

void ParsedNode::p_add(ParsedNode *n)
{
    n->p_parent = this;
    p_children.push_back(n);
}

void ParsedNode::p_add(const std::vector<ParsedNode*>& nodes)
{
    for (auto n : nodes)
        p_add(n);
}

Parser::Parser()
    : p_lookPos     (0)
    , p_level       (0)
    , p_visited     (0)
{

}

namespace
{
    class LevelInc
    {
    public:
        LevelInc(int* lev) : l(lev)
            { ++(*l); if (*l>1000) { PARSE_ERROR("TOO NESTED"); } }
        ~LevelInc() { --(*l); }
        int* l;
    };

}

bool Parser::forward()
{
    if (++p_lookPos >= p_lexer.parsedTokens().size())
    {
        p_look = ParsedToken();
        return false;
    }
    p_look = p_lexer.parsedTokens()[p_lookPos];
    return true;
}

void Parser::popPos()
{
    if (p_posStack.empty())
        return;
    p_lookPos = p_posStack.back();
    p_posStack.pop_back();
    setPos(p_lookPos);
}

void Parser::setPos(size_t p)
{
    p_lookPos = p;
    p_look = p_lookPos < p_lexer.parsedTokens().size()
            ? p_lexer.parsedTokens()[p_lookPos]
            : ParsedToken();
}

ParsedNode* Parser::parse(const QString &text)
{
    p_lookPos = 0;
    p_level = 0;
    p_visited = 0;
    p_text = text;
    p_lexer.tokenize(p_text);
    setPos(0);

    P_DEBUG("LEXED: " << p_lexer.toString());


    if (!p_rules.topRule())
        PARSE_ERROR("No top-level rule defined");

    auto node = new ParsedNode();
    node->p_parser = this;
    node->p_rule = p_rules.topRule();
    if (!parseRule(node))
        PARSE_ERROR("No top-level statement in source '"
                    << text << "'");
    node->p_length = lengthSince(0);
    return node;
}

size_t Parser::lengthSince(size_t oldPos) const
{
    size_t curPos = curToken().isValid() ? curToken().pos().pos()
                                         : p_text.size();
    // go to end of token w/o whitespace
    while ((int)curPos-1 < p_text.size()
           && curPos > oldPos && p_text[(int)curPos-1].isSpace())
        --curPos;
    return curPos - oldPos;
}

bool Parser::parseRule(ParsedNode* node, int subIdx)
{
    if (!curToken().isValid() || curToken().name() == "EOF")
        return false;

    LevelInc linc(&p_level);

    P_DEBUG(node->rule()->toString() << " ("
            << "\t\"" << p_text.mid(curToken().pos().pos()) << "\""
            << " " << subIdx
            );
    auto oldPos = curToken().pos();

    bool ret = parseRule_(node);
    P_DEBUG(") " << node->rule()->toString() << " =" << ret
            //<< "\t\"" << p_text.mid(curToken().pos().pos()) << "\""
            );

    node->p_length = lengthSince(oldPos.pos());

    auto parent = node->parent();
    bool emitMain = ret && node->rule()->p_func;
    bool emitSub = ret && parent && subIdx >= 0
            && subIdx < parent->rule()->subRules().size()
            && parent->rule()->subRules()[subIdx].func;

    // emit subrules
    if (emitSub)
    {
        P_DEBUG("EMIT " << node->toString());
        node->p_isEmitted = true;
        parent->rule()->subRules()[subIdx].func(node);
    }

    // emit rule
    if (emitMain)
    {
        P_DEBUG("EMIT " << node->toString());
        node->p_isEmitted = true;
        node->rule()->p_func(node);
    }
    ++p_visited;

    return ret;
}

bool Parser::parseRule_(ParsedNode* node)
{
    switch (node->rule()->type())
    {
        case Rule::T_TOKEN:
        {
            if (curToken().name() != node->rule()->name())
            {
                return false;
            }
            forward();
            return true;
        }

        case Rule::T_AND:
        {
            std::vector<ParsedNode*> subNodes;

            auto backup = p_lookPos;
            for (int idx=0; idx<node->rule()->subRules().size(); ++idx)
            {
                const Rule::SubRule& sub = node->rule()->subRules()[idx];

                auto subnode = new ParsedNode();
                subNodes.push_back(subnode);
                subnode->p_parser = this;
                subnode->p_rule = sub.rule;
                subnode->p_pos = curToken().pos();
                subnode->p_parent = node;

                //int startpos = curToken().pos().pos();
                bool ret = parseRule(subnode, idx);
                if (!ret)
                {
                    if (!sub.isOptional)
                    {
                        setPos(backup);
                        for (auto s : subNodes)
                            delete s;
                        return false;
                    }

                    delete subNodes.back();
                    subNodes.pop_back();
                }
                else
                {
                    //subnode->p_length = lengthSince(startpos);
                    if (sub.isRecursive)
                    {
                        auto pos = p_lookPos;
                        while (true)
                        {
                            auto subnode = new ParsedNode();
                            subnode->p_parser = this;
                            subnode->p_rule = sub.rule;
                            subnode->p_pos = curToken().pos();
                            subnode->p_parent = node;
                            //int startpos2 = curToken().pos().pos();
                            if (!parseRule(subnode, idx))
                            {
                                setPos(pos);
                                delete subnode;
                                break;
                            }
                            //subnode->p_length = lengthSince(startpos2);
                            subNodes.push_back(subnode);
                            pos = p_lookPos;
                        }
                    }
                }
            }
            node->p_add(subNodes);
            return true;
        }
        break;

        case Rule::T_OR:
        {
            std::vector<ParsedNode*> subNodes;

            auto pos = p_lookPos;
            for (int idx=0; idx<node->rule()->subRules().size(); ++idx)
            {
                const Rule::SubRule& sub = node->rule()->subRules()[idx];

                setPos(pos);

                auto subnode = new ParsedNode();
                subnode->p_parser = this;
                subnode->p_rule = sub.rule;
                subnode->p_pos = curToken().pos();
                subnode->p_parent = node;

                bool ret = parseRule(subnode, idx);
                if (ret)
                {
                    //subnode->p_length = lengthSince(pos);
                    subnode->p_nextTokenPos = p_lookPos;
                    /*
                    bool relevant = true;
                    for (auto s : subNodes)
                    if (subnode->length() < s->length()
                     || subnode->numChildLevels() < s->numChildLevels()
                            )
                    {
                        relevant = false;
                        break;
                    }
                    if (!relevant)
                    {
                        P_DEBUG("discarding OR " << subnode->toString());
                        delete subnode;
                    }
                    else*/
                        subNodes.push_back(subnode);
                }
                else
                    delete subnode;
            }

            if (subNodes.empty())
            {
                setPos(pos);
                return false;
            }

            if (subNodes.size() == 1)
            {
                //P_DEBUG("one resulting OR node "
                //        << subNodes.front()->toString());
                node->p_add(subNodes.front());
                setPos(subNodes.front()->p_nextTokenPos);
                return true;
            }

            // find most "relevant"
            ParsedNode* rel = 0;
            int maxCh = -1, maxLen = -1;
            for (auto s : subNodes)
            {
                if (rel == nullptr)
                {
                    rel = s;
                    break;
                }
                int len = s->length();
                if (len > maxLen)
                    rel = s, maxLen = len;
                else
                {
                    int ch = s->numChildLevels();
                    if (ch > maxCh)
                        rel = s, maxCh = ch;
                }
            }
            for (auto s : subNodes)
            if (s != rel)
            {
                P_DEBUG("discarding OR " << s->toString());
                delete s;
            }
            P_DEBUG("keeping OR " << rel->toString());
            node->p_add(rel);
            setPos(rel->p_nextTokenPos);
            return true;
        }
        break;
    }

    return false;
}
