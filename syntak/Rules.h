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

#ifndef SYNTAKSRC_SYNTAK_RULES_H
#define SYNTAKSRC_SYNTAK_RULES_H

#include <vector>
#include <map>
#include <memory>

#include <QString>
#include <QList>

#include "Tokens.h"

namespace Syntak {

class ParsedNode;

/** A whole or piece of a production */
class Rule
{
    Rule() : p_id(-1), p_isTop(false), p_orType(OR_FIRST) { }
    Rule(const Rule&) = delete;
    void operator=(const Rule&) = delete;

public:
    typedef std::function<void(ParsedNode*)> Callback;

    enum Type
    {
        T_TOKEN,
        T_AND,
        T_OR
    };

    /** Defines which node to pick in an OR-rule.
        All types except OR_FIRST evaluate all OR-branches
        in parallel! */
    enum OrType
    {
        /** The first matching subrule is picked */
        OR_FIRST,
        /** The subrule that captures the most source is picked */
        OR_LONGEST,
        /** The subrule that captures the fewest source is picked */
        OR_SHORTEST,
        /** The subrule that expands deepest is picked */
        OR_DEEPEST,
        /** The subrule that expands the less is picked */
        OR_SHALLOWEST
    };

    struct SubRule
    {
        Rule* rule;
        QString name;
        bool isOptional;
        bool isRecursive;
        Callback func;
    };

    Type type() const { return p_type; }
    const QString& name() const { return p_name; }
    bool isTop() const { return p_isTop; }
    bool isConnected() const;

    // only valid for T_TOKEN
    int id() const { return p_id; }
    // only valid for T_TOKEN
    const Token& token() const { return p_token; }
    /** Only valid for T_OR. @see OrType */
    OrType orType() const { return p_orType; }

    const QList<SubRule>& subRules() const { return p_subRules; }
    bool contains(const QString& name) const;
    bool wants(const QString& name) const;

    void connect(Callback f) { p_func = f; }
    void connect(int idx, Callback f);

    const char* typeName() const {
        return type() == T_TOKEN ? "term"
                                 : type() == T_OR ? "or" : "and"; }

    QString toString() const { return QString("%1(%2)").arg(name())
                                                       .arg(typeName()); }
    QString toDefinitionString() const;

private:
    friend class Rules;
    friend class Parser;
    int p_id;
    QString p_name;
    Type p_type;
    Token p_token;
    QList<SubRule> p_subRules;
    Callback p_func;
    bool p_isTop;
    OrType p_orType;
};


/** A set of rules describing a grammar */
class Rules
{
public:
    Rules() : p_checked(false), p_connected(false) { }

    /** Is any of the rules connected? */
    bool isConnected() const { return p_connected; }

    Rule* find(const QString& name);
    Rule* topRule() const { return p_topRule; }

    //const std::vector<Rule*>& rules() const { return p_rulesVec; }
    //const std::vector<Rule*>& rulesTerm() const { return p_rulesTerm; }

    Rule* createToken(const Token& t);

    Rule* createAnd(const QString& name, const QStringList& symbols);

    Rule* createOr(const QString& name, const QStringList& symbols);
    Rule* createOr(const QString& name, Rule::OrType orType,
                   const QStringList& symbols);

    Rule* createAnd(const QString& name, const QString& sym1);
    Rule* createAnd(const QString& name,
                    const QString& sym1, const QString& sym2);
    Rule* createAnd(const QString& name, const QString& sym1,
                    const QString& sym2,const QString& sym3);
    Rule* createAnd(const QString& name, const QString& sym1,
                    const QString& sym2,const QString& sym3,
                    const QString& sym4);

    Rule* createOr (const QString& name, const QString& sym1);
    Rule* createOr (const QString& name,
                    const QString& sym1, const QString& sym2);
    Rule* createOr (const QString& name, const QString& sym1,
                    const QString& sym2,const QString& sym3);
    Rule* createOr (const QString& name, const QString& sym1,
                    const QString& sym2,const QString& sym3,
                    const QString& sym4);

    Rule* createOr (const QString& name, Rule::OrType orType,
                    const QString& sym1);
    Rule* createOr (const QString& name, Rule::OrType orType,
                    const QString& sym1, const QString& sym2);
    Rule* createOr (const QString& name, Rule::OrType orType,
                    const QString& sym1, const QString& sym2,
                    const QString& sym3);
    Rule* createOr (const QString& name, Rule::OrType orType,
                    const QString& sym1, const QString& sym2,
                    const QString& sym3, const QString& sym4);

    void addTokens(const Tokens&);

    QString toDefinitionString() const;
    void check() { if (!p_checked) p_check(); }

    void connect(const QString& name, Rule::Callback f);
    void connect(const QString& name, int idx, Rule::Callback f);

private:
    static Rule::SubRule makeSubRule(const QString& s);
    void p_add(Rule*);
    void p_check();
    bool p_checked;
    std::map<QString, std::shared_ptr<Rule>> p_rules;
    Rule* p_topRule;
    bool p_connected;
};

} // namespace Syntak

#endif // SYNTAKSRC_SYNTAK_RULES_H
