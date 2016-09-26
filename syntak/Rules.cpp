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


#include <QDebug>

#include "Rules.h"
#include "Tokens.h"

bool Rule::contains(const QString& n) const
{
    for (auto& i : subRules())
        if (i.name == n)
            return true;
    return false;
}

bool Rule::wants(const QString &n) const
{
    if (type() == T_TOKEN && name() == n)
        return true;
    if (type() == T_OR)
    {
        for (const SubRule& r : subRules())
            if (r.name == n)
                return true;
    }
    if (type() == T_AND)
    {
        for (const SubRule& r : subRules())
        {
            if (r.name == n)
                return true;
            if (!r.isOptional)
                return false;
        }
    }
    return false;
}

void Rule::connect(int idx, Callback f)
{
    if (idx >= 0 && idx < p_subRules.size())
        p_subRules[idx].func = f;
}


Rule* Rules::find(const QString &name)
{
    auto i = p_rules.find(name);
    if (i == p_rules.end())
    {
        qWarning()<<"rule "<<name<<" not added";
        abort();
    }
    return i->second.get();
}

QString Rule::toDefinitionString() const
{
    QString s = name();
    if (p_func)
        s += "!";
    s += " : ";
    switch (type())
    {
        case T_TOKEN: s += "\"" + token().tokenString() + "\""; break;
        case T_AND:
        case T_OR:
        {
            bool space=false;
            for (const SubRule& r : subRules())
            {
                if (!space)
                    space = true;
                else
                    s += (type() == T_OR ? " | " : " ");

                if (r.isOptional)
                    s += "[" + r.name + "]";
                else
                    s += r.name;

                if (r.isRecursive)
                    s += "*";

                if (r.func)
                    s += "!";
            }
        }
        break;
    }
    return s;
}


Rule::SubRule Rules::makeSubRule(const QString& s)
{
    Rule::SubRule r;
    r.rule = nullptr;
    r.isOptional = s.startsWith("[");
    r.isRecursive = s.endsWith("*");
    r.name = s;
    r.name.remove("[").remove("]").remove("*");
    return r;

}

void Rules::p_add(Rule* r)
{
    auto sp = std::shared_ptr<Rule>(r);
    p_rules.insert(std::make_pair(r->name(), sp));
    p_checked = false;
}

Rule* Rules::createAnd(const QString& name,
                       const QStringList& rules)
{
    auto r = new Rule();
    r->p_name = name;
    r->p_type = Rule::T_AND;
    for (auto& s : rules)
        r->p_subRules << makeSubRule(s);
    p_add(r);
    return r;
}

Rule* Rules::createOr(const QString& name,
                      const QStringList& rules)
{
    auto r = createAnd(name, rules);
    r->p_type = Rule::T_OR;
    return r;
}

Rule* Rules::createToken(const Token& t)
{
    auto r = new Rule();
    r->p_name = t.name();
    r->p_type = Rule::T_TOKEN;
    r->p_token = t;
    p_add(r);
    return r;
}

void Rules::addTokens(const Tokens& tok)
{
    for (auto& t : tok.tokens())
        createToken(t);
}

void Rules::connect(const QString &name, Rule::Callback f)
{
    if (auto r = find(name))
        r->connect(f);
}

void Rules::connect(const QString &name, int idx, Rule::Callback f)
{
    if (auto r = find(name))
        r->connect(idx, f);
}

QString Rules::toDefinitionString() const
{
    QList<Rule*> ru;
    for (auto& i : p_rules)
        if (i.second->type() == Rule::T_TOKEN)
            ru.append(i.second.get());
    for (auto& i : p_rules)
        if (i.second->type() != Rule::T_TOKEN)
            ru.append(i.second.get());

    QString s;
    for (Rule* r : ru)
    {
        if (!s.isEmpty())
            s += "\n";
        s += r->toDefinitionString();
    }
    return s;
}

void Rules::p_check()
{
    p_topRule = nullptr;

    for (auto& i : p_rules)
    {
        i.second->p_isTop = false;
        for (Rule::SubRule& sub : i.second->p_subRules)
        {
            sub.rule = find(sub.name);
            if (!sub.rule)
                PARSE_ERROR("Subrule "<<sub.name<<" in "<<i.second->name()
                            <<" not known");
        }
    }

    // find top rule
    for (auto& i : p_rules)
    if (i.second->type() != Rule::T_TOKEN)
    {
        bool contained = false;
        for (auto& j : p_rules)
        if (i.second != j.second)
        {
            if (j.second->contains(i.second->name()))
            {
                contained = true;
                break;
            }
        }
        if (!contained)
        {
            p_topRule = i.second.get();
            i.second->p_isTop = true;
            //qDebug() << "toprule" << i.second->toString();
            break;
        }
    }

    p_checked = true;
}
