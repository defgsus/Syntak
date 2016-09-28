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
#include "error.h"

#if 0
#   define SYNTAK_DO_DEBUG
#   define SYNTAK_DEBUG(arg__) \
        { QString indent__ = p_level ? QString("%1").arg(p_level) \
                                 : QString(" "); \
      indent__ += QString(" ").repeated(p_level+1 - indent__.size()); \
      qDebug().noquote().nospace() << indent__ << arg__; }
#else
#   define SYNTAK_DEBUG(unused__) { }
#endif

#if 0 || defined(SYNTAK_DO_DEBUG)
#   define SYNTAK_DEBUG_EMIT(arg__) \
        { qDebug().noquote().nospace() << arg__; }
#else
#   define SYNTAK_DEBUG_EMIT(unused__) { }
#endif


namespace Syntak {

ParsedNode::~ParsedNode()
{
    for (auto n : p_children)
        delete n;
}

bool ParsedNode::contains(const ParsedNode* node) const
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
        return "*INVALID*";
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
        return QString("*INVALID*");
    return p_parser->text().mid(pos().pos(), length());
}

namespace {

QString toBracketStringWithLineBreaks(
        const ParsedNode* n, const QString& indent, bool withContent)
{
    if (!n->isValid())
        return QString("*INVALID*");

    QString s = indent + n->name();
    if (!n->children().empty())
    {
        s += " {\n";
        for (auto c : n->children())
            s += toBracketStringWithLineBreaks(
                        c, indent + " ", withContent);
        s += indent + "}";
    }
    else if (withContent)
        s += " \"" + n->text() + "\"";
    s += "\n";
    return s;
}

} // namespace

QString ParsedNode::toBracketString(
        bool withContent, bool lineBreaks) const
{
    if (!isValid())
        return QString("*INVALID*");

    if (!lineBreaks)
    {
        QString s = name();
        if (!children().empty())
        {
            s += "{";
            for (auto c : children())
                s += " " + c->toBracketString();
            s += " }";
        }
        else if (withContent)
            s += " \"" + text() + "\"";
        return s;
    }
    else
        return toBracketStringWithLineBreaks(this, "", withContent);
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


// ####################### Parser ##############################

namespace
{
    class LevelInc
    {
    public:
        LevelInc(int* lev) : l(lev)
            { ++(*l); if (*l>1000) { SYNTAK_ERROR("TOO NESTED"); } }
        ~LevelInc() { --(*l); }
        int* l;
    };

}

struct Parser::Private
{
    Private(Parser* p)
        : p         (p)
        , p_lookPos     (0)
        , p_level       (0)
        , p_visited     (0)
    {

    }

    void clear();
    ParsedNode* parse(const QString& text);
    bool parseRule(ParsedNode* node);
    bool parseRuleToken(ParsedNode* node);
    bool parseRuleAnd(ParsedNode* node);
    bool parseRuleOr(ParsedNode* node);

    void emitNodes(ParsedNode* root);

    const ParsedToken& curToken() const { return p_look; }
    bool forward();
    void setPos(size_t);
    size_t lengthSince(size_t sourcePos) const;

    const ParsedNode* findLeaveNode(const ParsedNode* n);

    Parser* p;
    Rules p_rules;
    Tokenizer p_lexer;
    QString p_text;
    ParsedToken p_look;
    size_t p_lookPos;
    int p_level, p_visited;
};



Parser::Parser()
    : p_        (new Private(this))
{

}

Parser::Parser(const Parser& p)
    : Parser()
{
    *this = p;
}

Parser::~Parser()
{
    p_->clear();
    delete p_;
}

Parser& Parser::operator = (const Parser& p)
{
    *p_ = *p.p_;
    p_->p = this;
    return *this;
}

const Rules& Parser::rules() const { return p_->p_rules; }
Rules& Parser::rules() { return p_->p_rules; }
const Tokenizer& Parser::tokenizer() const { return p_->p_lexer; }
int Parser::numNodesVisited() const { return p_->p_visited; }
const QString& Parser::text() const { return p_->p_text; }

void Parser::setRules(const Rules& r)
    { p_->p_rules = r; p_->p_rules.check(); }
void Parser::setTokens(const Tokens& t) { p_->p_lexer.setTokens(t); }


bool Parser::Private::forward()
{
    if (++p_lookPos >= p_lexer.parsedTokens().size())
    {
        p_look = ParsedToken();
        return false;
    }
    p_look = p_lexer.parsedTokens()[p_lookPos];
    return true;
}

void Parser::Private::setPos(size_t p)
{
    p_lookPos = p;
    p_look = p_lookPos < p_lexer.parsedTokens().size()
            ? p_lexer.parsedTokens()[p_lookPos]
            : ParsedToken();
}

void Parser::Private::clear()
{
    p_lookPos = 0;
    p_level = 0;
    p_visited = 0;
}

ParsedNode* Parser::parse(const QString &text)
{
    return p_->parse(text);
}

ParsedNode* Parser::Private::parse(const QString &text)
{
    clear();
    p_text = text;
    p_lexer.tokenize(p_text);
    setPos(0);

    SYNTAK_DEBUG("LEXED: " << p_lexer.toString());

    if (!p_rules.topRule())
        SYNTAK_ERROR("No top-level rule defined");

    auto node = new ParsedNode();
    node->p_parser = p;
    node->p_rule = p_rules.topRule();
    if (!parseRule(node))
        SYNTAK_ERROR("No top-level statement in source '"
                    << text << "'");
    node->p_length = lengthSince(0);

    if (p_rules.isConnected())
        emitNodes(node);

    return node;
}

size_t Parser::Private::lengthSince(size_t oldPos) const
{
    size_t curPos = curToken().isValid() ? curToken().pos().pos()
                                         : p_text.size();
    // go to end of token w/o whitespace
    while ((int)curPos-1 < p_text.size()
           && curPos > oldPos && p_text[(int)curPos-1].isSpace())
        --curPos;
    return curPos - oldPos;
}

bool Parser::Private::parseRule(ParsedNode* node)
{
    if (!curToken().isValid() || curToken().name() == "EOF")
        return false;

    //if (!node->rule()->wants(curToken().name()))
    //    return false;

    LevelInc linc(&p_level);

    SYNTAK_DEBUG(node->rule()->toString() << " ("
            << "\t\"" << p_text.mid(curToken().pos().pos(), 4) << "\"");

    auto oldPos = curToken().pos();

    bool ret;
    switch (node->rule()->type())
    {
        case Rule::T_TOKEN: ret = parseRuleToken(node); break;
        case Rule::T_AND: ret = parseRuleAnd(node); break;
        case Rule::T_OR: ret = parseRuleOr(node); break;
        default: SYNTAK_ERROR("Unknown rule type "
                              << node->rule()->type() << " in node "
                              << node->toString());
    }

    SYNTAK_DEBUG(") " << node->rule()->toString() << " =" << ret
            //<< "\t\"" << p_text.mid(curToken().pos().pos()) << "\""
            );

    node->p_length = lengthSince(oldPos.pos());

    ++p_visited;

    return ret;
}


bool Parser::Private::parseRuleToken(ParsedNode* node)
{
    if (curToken().name() != node->rule()->name())
    {
        return false;
    }
    forward();
    return true;
}

bool Parser::Private::parseRuleAnd(ParsedNode* node)
{
    std::vector<ParsedNode*> subNodes;

    auto backup = p_lookPos;
    for (int idx=0; idx<node->rule()->subRules().size(); ++idx)
    {
        const Rule::SubRule& sub = node->rule()->subRules()[idx];

        auto subnode = new ParsedNode();
        subNodes.push_back(subnode);
        subnode->p_parser = p;
        subnode->p_rule = sub.rule;
        subnode->p_pos = curToken().pos();
        subnode->p_parent = node;

        //int startpos = curToken().pos().pos();
        bool ret = parseRule(subnode);
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
                    subnode->p_parser = p;
                    subnode->p_rule = sub.rule;
                    subnode->p_pos = curToken().pos();
                    subnode->p_parent = node;
                    //int startpos2 = curToken().pos().pos();
                    if (!parseRule(subnode))
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

#if 0
bool Parser::Private::parseRuleOr(ParsedNode* node)
{
    std::vector<ParsedNode*> subNodes;

    auto pos = p_lookPos;
    for (int idx=0; idx<node->rule()->subRules().size(); ++idx)
    {
        const Rule::SubRule& sub = node->rule()->subRules()[idx];

        setPos(pos);

        auto subnode = new ParsedNode();
        subnode->p_parser = p;
        subnode->p_rule = sub.rule;
        subnode->p_pos = curToken().pos();
        subnode->p_parent = node;

        bool ret = parseRule(subnode);
        if (ret)
        {
            subnode->p_nextTokenPos = p_lookPos;
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
        SYNTAK_DEBUG("discarding OR " << s->toString());
        delete s;
    }
    SYNTAK_DEBUG("keeping OR " << rel->toString());
    node->p_add(rel);
    setPos(rel->p_nextTokenPos);
    return true;
}
#else
bool Parser::Private::parseRuleOr(ParsedNode* node)
{
    auto pos = p_lookPos;
    for (int idx=0; idx<node->rule()->subRules().size(); ++idx)
    {
        const Rule::SubRule& sub = node->rule()->subRules()[idx];

        setPos(pos);

        auto subnode = new ParsedNode();
        subnode->p_parser = p;
        subnode->p_rule = sub.rule;
        subnode->p_pos = curToken().pos();
        subnode->p_parent = node;

        bool ret = parseRule(subnode);
        if (ret)
        {
            subnode->p_nextTokenPos = p_lookPos;
            node->p_add(subnode);
            return true;
        }
        else
            delete subnode;
    }

    setPos(pos);
    return false;
}
#endif

void Parser::Private::emitNodes(ParsedNode* node)
{
    // depth-first
    for (auto c : node->children())
        emitNodes(c);

    if (!node->rule())
        return;

    // emit rule
    if (node->rule()->p_func)
    {
        SYNTAK_DEBUG_EMIT("EMIT " << node->toString());
        node->rule()->p_func(node);
    }

    // emit this rule as parent's subrule
    if (node->parent())
    {
        for (const Rule::SubRule& sr : node->parent()->rule()->subRules())
        {
            if (sr.func && sr.name == node->name())
            {
                SYNTAK_DEBUG_EMIT(
                            "EMIT subrule " << node->toString()
                            << " from " << node->parent()->toString());
                sr.func(node);
            }
        }
    }
}


const ParsedNode* Parser::Private::findLeaveNode(const ParsedNode* n)
{
    if (n->isLeave() || n->children().size() > 1)
        return n;
    return findLeaveNode(n->children()[0]);
}

ParsedNode* Parser::reduceTree(const ParsedNode* n)
{
    auto newParent = new ParsedNode(*n);
    newParent->p_children.clear();

    for (auto c : n->children())
    {
        ParsedNode* newNode;
        auto l = p_->findLeaveNode(c);
        if (l != c)
            newNode = reduceTree(l);
        else
            newNode = reduceTree(c);
        newParent->p_add(newNode);
    }
    return newParent;
}

} // namespace Syntak
