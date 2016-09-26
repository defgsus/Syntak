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


bool ParsedNode::contains(ParsedNode* node) const
{
    for (auto n : p_children)
        if (n == node)
            return true;
    return false;
}

QString ParsedNode::toString() const
{
    if (!isValid())
        return "INVALID";
    return QString("%1@%2").arg(rule()->name()).arg(pos().toString());
}

void ParsedNode::p_add(ParsedNode *n)
{
    n->p_parent = this;
    p_children.push_back(n);
}

Parser::Parser()
{

}

namespace
{
    class LevelInc
    {
    public:
        LevelInc(int* lev) : l(lev)
            { ++(*l); if (*l>100) { PARSE_ERROR("TOO NESTED"); } }
        ~LevelInc() { --(*l); }
        int* l;
    };

#if 1
#   define P_DEBUG(arg__) \
        { QString indent__ = p_level ? QString("%1").arg(p_level) \
                                 : QString(" "); \
      indent__ += QString(" ").repeated(p_level+1 - indent__.size()); \
      qDebug().noquote().nospace() << indent__ << arg__; }
#else
#   define P_DEBUG(unused__) { }
#endif
}

bool Parser::forward()
{
    if (++p_lookPos >= p_tokens.size())
    {
        p_look = LexxedToken();
        return false;
    }
    p_look = p_tokens[p_lookPos];
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
    p_look = p_lookPos < p_tokens.size() ? p_tokens[p_lookPos]
                                         : LexxedToken();
}

ParsedNode* Parser::parse(const QString &text)
{
    p_tokens.clear();
    p_lookPos = 0;
    p_level = 0;
    p_visited = 0;
    p_text = text;
    p_lexxer.tokenize(p_text, p_tokens);
    setPos(0);

    P_DEBUG("LEXXED: " << p_lexxer.toString(p_tokens));


    if (!p_rules.topRule())
        PARSE_ERROR("No top-level rule defined");

    auto node = new ParsedNode();
    node->p_rule = p_rules.topRule();
    if (!parseRule(node))
        PARSE_ERROR("No top statement found");
    return node;
}

bool Parser::parseRule(ParsedNode* parent, int subIdx)
{
    if (!curToken().isValid() || curToken().name() == "EOF")
        return false;

    LevelInc linc(&p_level);
    P_DEBUG(parent->rule()->toString() << " ("
            << "\t\"" << p_text.mid(curToken().pos().pos()) << "\""
            << " " << subIdx
            );
    auto oldPos = curToken().pos();
    bool ret = parseRule_(parent);
    P_DEBUG(") " << parent->rule()->toString() << " =" << ret
            //<< "\t\"" << p_text.mid(curToken().pos().pos()) << "\""
            );

    bool emitMain = ret && parent->rule()->p_func;
    bool emitSub = ret && parent && subIdx >= 0
            && subIdx < parent->rule()->subRules().size()
            && parent->rule()->subRules()[subIdx].func;

    // emit subrules
    if (emitSub)
    {
        int curPos = curToken().isValid() ? curToken().pos().pos()
                                          : p_text.size();
        while (curPos-1 < p_text.size()
               && curPos > oldPos.pos() && p_text[curPos-1].isSpace())
            --curPos;

        ParsedToken t;
        t.p_pos = oldPos;
        t.p_text = p_text.mid(oldPos.pos(), curPos - oldPos.pos());
        t.p_rule = parent->rule()->subRules()[subIdx].rule;
        P_DEBUG("EMIT " << t.toString());
        parent->rule()->subRules()[subIdx].func(t);
    }

    // emit rule
    if (emitMain)
    {
        int curPos = curToken().isValid() ? curToken().pos().pos()
                                          : p_text.size();
        while (curPos-1 < p_text.size()
               && curPos > oldPos.pos() && p_text[curPos-1].isSpace())
            --curPos;

        ParsedToken t;
        t.p_pos = oldPos;
        t.p_text = p_text.mid(oldPos.pos(), curPos - oldPos.pos());
        t.p_rule = parent->rule();
        P_DEBUG("EMIT " << t.toString());
        parent->rule()->p_func(t);
    }
    ++p_visited;

    return ret;
}

bool Parser::parseRule_(ParsedNode* parent)
{
    auto node = new ParsedNode();
    node->p_rule = parent->rule();
    node->p_pos = curToken().pos();

    switch (parent->rule()->type())
    {
        case Rule::T_TOKEN:
        {
            if (curToken().name() != parent->rule()->name())
            {
                delete node;
                return false;
            }
            forward();
            parent->p_add(node);
            return true;
        }

        case Rule::T_AND:
        {
            auto backup = p_lookPos;
            for (int idx=0; idx<parent->rule()->subRules().size(); ++idx)
            {
                const Rule::SubRule& sub = parent->rule()->subRules()[idx];

                auto subnode = new ParsedNode();
                subnode->p_rule = sub.rule;
                subnode->p_pos = curToken().pos();

                bool ret = parseRule(subnode, idx);
                if (!sub.isOptional && !ret)
                {
                    setPos(backup);
                    delete subnode;
                    delete node;
                    return false;
                }

                if (sub.isRecursive && ret)
                {
                    auto pos = p_lookPos;
                    while (true)
                    {
                        if (!parseRule(subnode, idx))
                        {
                            setPos(pos);
                            break;
                        }
                        node->p_add(subnode);
                        pos = p_lookPos;
                    }
                }
            }
            parent->p_add(node);
            return true;
        }
        break;

        case Rule::T_OR:
        {
            auto pos = p_lookPos;
            for (int idx=0; idx<parent->rule()->subRules().size(); ++idx)
            {
                const Rule::SubRule& sub = parent->rule()->subRules()[idx];

                setPos(pos);

                auto subnode = new ParsedNode();
                subnode->p_rule = sub.rule;
                subnode->p_pos = curToken().pos();

                bool ret = parseRule(subnode, idx);
                if (ret)
                {
                    node->p_add(subnode);
                    return true;
                }
                delete subnode;
            }
            setPos(pos);
            delete node;
            return false;
        }
        break;
    }

    return false;
}
