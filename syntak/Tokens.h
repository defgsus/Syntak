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

#ifndef TOKENS_H
#define TOKENS_H

#include <map>
#include <set>

#include <QString>
#include <QRegExp>

/** Position within a source text */
class SourcePos
{
public:
    SourcePos(int pos = 0, int line = 0)
        : p_pos(pos), p_line(line) { }
    int pos() const { return p_pos; }
    int line() const { return p_line; }
    QString toString() const;
private:
    int p_pos, p_line;
};


/** A matcher for a terminal symbol */
class Token
{
public:
    Token() { }
    Token(const QString& name, const QString& fixedString)
        : p_name    (name)
        , p_fixed   (fixedString)
    { }
    Token(const QString& name, const QRegExp& regexp)
        : p_name    (name)
        , p_regexp  (regexp)
    { }

    const QString& name() const { return p_name; }
    const QString& fixedString() const { return p_fixed; }

    /** find match for token,
        change @p pos to next index after recognized token */
    bool isMatch(const QString& s, int* pos) const;

    QString tokenString() const
        { return p_regexp.isEmpty() ? p_fixed : p_regexp.pattern(); }
private:
    QString p_name, p_fixed;
    QRegExp p_regexp;
};


/** A terminal symbol that was matched in the source text */
class ParsedToken
{
public:
    ParsedToken() { }
    ParsedToken(const QString& name, const QString& value,
                SourcePos pos)
        : p_name    (name)
        , p_value   (value)
        , p_pos     (pos)
    { }

    bool isValid() const { return !p_name.isEmpty(); }
    const QString& name() const { return p_name; }
    const QString& value() const { return p_value; }
    const SourcePos& pos() const { return p_pos; }

private:
    friend class Parser;
    QString p_name, p_value;
    SourcePos p_pos;
};


/** Collection of terminal symbols */
class Tokens
{
public:

    Tokens& add(const Token& t)
    {
        //p_tokens.insert(std::make_pair(t.name(), t));
        p_tokens.push_back(t);
        return *this;
    }

    Tokens& operator << (const Token& t)
        { return add(t); }

    const std::vector<Token>& tokens() const { return p_tokens; }

private:
    std::vector<Token> p_tokens;
};



class Tokenizer
{
public:
    Tokenizer() { }
    Tokenizer(const Tokens& t) : p_tokens(t) { }

    const std::vector<ParsedToken>& parsedTokens() const
        { return p_parsedTokens; }

    void setTokens(const Tokens& t) { p_tokens = t; }

    void tokenize(const QString& input);

    QString toString() const;

private:
    Tokens p_tokens;
    std::vector<ParsedToken> p_parsedTokens;
};



#endif // TOKENS_H
