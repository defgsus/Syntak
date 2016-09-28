/***************************************************************************

Copyright (C) 2016  stefan.berke @ modular-audio-graphics.com

This source is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either
version 3.0 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this software; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

****************************************************************************/

#ifndef SYNTAKSRC_SYNTAK_MATHPARSER_H
#define SYNTAKSRC_SYNTAK_MATHPARSER_H

#include <functional>

#include <QString>

namespace Syntak {

class Parser;

/** A parser/evaluater for simple mathematical equations.
    Template param T can be float, double, int8-64, uint8-64 */
template <typename T>
class MathParser
{
public:

    MathParser();
    MathParser(const MathParser&);
    ~MathParser();

    MathParser& operator=(const MathParser&);

    /** Optional initialization.
        Normally the first call to evaluate() will initialize
        everything. If you want to access the Parser internals
        then call init() first. */
    void init();

    T evaluate(const QString& expression);


    const Parser& parser() const;
    const QString& expression() const;

    void setIgnoreDivisionByZero(bool e);

    /** Adds an unary function to the grammar */
    void addFunction(const QString& name, std::function<T(T)>);
    /** Adds a binary function to the grammar */
    void addFunction(const QString& name, std::function<T(T, T)>);
    /** Adds a ternary function to the grammar */
    void addFunction(const QString& name, std::function<T(T, T, T)>);
    /** Adds a function with 4 arguments to the grammar */
    void addFunction(const QString& name, std::function<T(T, T, T, T)>);

    bool hasFunctions() const;
    QStringList getFunctionNames(size_t numArguments) const;

private:
    struct Private;
    Private* p_;
};

} // namespace Syntak

#endif // SYNTAKSRC_SYNTAK_MATHPARSER_H
