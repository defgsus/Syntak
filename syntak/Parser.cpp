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

#if 0
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

void Parser::parse(const QString &text)
{
    p_tokens.clear();
    p_lookPos = 0;
    p_level = 0;
    p_visited = 0;
    p_text = text;
    p_lexxer.tokenize(p_text, p_tokens);
    setPos(0);

    P_DEBUG("LEXXED: " p_lexxer.toString(p_tokens));


    if (!p_rules.topRule())
        PARSE_ERROR("No top-level rule defined");

    if (!parseRule(p_rules.topRule()))
        PARSE_ERROR("No top statement found");
}

bool Parser::parseRule(const Rule* r, const Rule* parent, int subIdx)
{
    if (!curToken().isValid() || curToken().name() == "EOF")
        return false;

    LevelInc linc(&p_level);
    P_DEBUG(r->toString() << " ("
            << "\t\"" << p_text.mid(curToken().pos().pos()) << "\""
            << " " << subIdx
            );
    auto oldPos = curToken().pos();
    bool ret = parseRule_(r);
    P_DEBUG(") " << r->toString() << " =" << ret
            //<< "\t\"" << p_text.mid(curToken().pos().pos()) << "\""
            );

    // emit subrules
    if (ret && parent && subIdx >= 0 && subIdx < parent->subRules().size()
        && parent->subRules()[subIdx].func)
    {
        int curPos = curToken().isValid() ? curToken().pos().pos()
                                          : p_text.size();
        while (curPos-1 < p_text.size()
               && curPos > oldPos.pos() && p_text[curPos-1].isSpace())
            --curPos;

        ParsedToken t;
        t.p_pos = oldPos;
        t.p_text = p_text.mid(oldPos.pos(), curPos - oldPos.pos());
        t.p_rule = parent->subRules()[subIdx].rule;
        parent->subRules()[subIdx].func(t);
    }

    // emit rule
    if (ret && r->p_func)
    {
        int curPos = curToken().isValid() ? curToken().pos().pos()
                                          : p_text.size();
        while (curPos-1 < p_text.size()
               && curPos > oldPos.pos() && p_text[curPos-1].isSpace())
            --curPos;

        ParsedToken t;
        t.p_pos = oldPos;
        t.p_text = p_text.mid(oldPos.pos(), curPos - oldPos.pos());
        t.p_rule = r;
        r->p_func(t);
    }
    ++p_visited;
    return ret;
}

bool Parser::parseRule_(const Rule* r)
{
    switch (r->type())
    {
        case Rule::T_TOKEN:
            if (curToken().name() != r->name())
                return false;
            forward();
            return true;

        case Rule::T_AND:
        {
            auto backup = p_lookPos;
            for (int idx=0; idx<r->subRules().size(); ++idx)
            {
                const Rule::SubRule& sub = r->subRules()[idx];

                bool ret = parseRule(sub.rule, r, idx);
                if (!sub.isOptional && ret == false)
                {
                    setPos(backup);
                    return false;
                }

                if (sub.isRecursive && ret)
                {
                    auto pos = p_lookPos;
                    while (true)
                    {
                        if (!parseRule(sub.rule, r, idx))
                        {
                            setPos(pos);
                            break;
                        }
                        pos = p_lookPos;
                    }
                }
            }
            return true;
        }
        break;

        case Rule::T_OR:
        {
            auto pos = p_lookPos;
            for (int idx=0; idx<r->subRules().size(); ++idx)
            {
                const Rule::SubRule& sub = r->subRules()[idx];

                setPos(pos);
                bool ret = parseRule(sub.rule, r, idx);
                if (ret)
                    return true;
            }
            setPos(pos);
            return false;
        }
        break;
    }

    return false;
}
