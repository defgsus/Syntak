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

#include "Tokens.h"

QString SourcePos::toString() const
{
    return line() <= 0 ? QString("%1").arg(pos())
                       : QString("%1:%2").arg(line()).arg(pos());
}

bool Token::isMatch(const QString& s, int* pos) const
{
    if (p_regexp.isEmpty())
    {
        int ps = *pos, pt = 0;
        while (ps < s.size() && pt < p_fixed.length())
        {
            if (s[ps] != p_fixed[pt])
                return false;
            ++pt;
            ++ps;
        }
        if (pt != p_fixed.length())
            return false;
        *pos = ps;
        return true;
    }
    else
    {
        int idx = p_regexp.indexIn(s, *pos);
        if (idx != *pos)
            return false;
        //qDebug() << *pos << s.mid(*pos,3) << idx << p_regexp.matchedLength();
        *pos += p_regexp.matchedLength();
        return true;
    }
}



void Tokenizer::tokenize(const QString& input)
{
    int srcPos=0, srcLine=0;
    p_parsedTokens.clear();

    for (int i=0; i<input.size(); ++i, ++srcPos)
    {
        if (input[i] == '\n')
            ++srcLine;

        if (input[i].isSpace())
            continue;

        int mp = -1;
        const Token* best = nullptr;
        QString value;
        for (const Token& t : p_tokens.tokens())
        {
            int p = i;
            if (t.isMatch(input, &p))
            {
                if (p > mp)
                    mp = p, best = &t,
                            value = input.mid(i, p-i);
            }
        }
        if (best)
        {
            p_parsedTokens.push_back(
                ParsedToken(best->name(), value,
                            SourcePos(srcPos, srcLine)) );
            while (i+1 < mp && i+1 < input.size())
            {
                ++i;
                ++srcPos;
                if (input[i] == '\n')
                    ++srcLine;
            }
        }
    }

    p_parsedTokens.push_back(
        ParsedToken("EOF", "", SourcePos(srcPos, srcLine)) );
}

QString Tokenizer::toString() const
{
    QString s;
    for (const ParsedToken& t : p_parsedTokens)
        s += QString("%1(%2)@%3 ")
                .arg(t.name()).arg(t.value()).arg(t.pos().pos());
    return s;
}
