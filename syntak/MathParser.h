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
    ~MathParser();

    T evaluate(const QString& expression);


    const Parser& parser() const;
    const QString& expression() const;

    void setIgnoreDivisionByZero(bool e);

    /** Adds a binary function to the syntax */
    void addFunction(const QString& name, std::function<T(T, T)>);

private:
    struct Private;
    Private* p_;
};

} // namespace Syntak

#endif // SYNTAKSRC_SYNTAK_MATHPARSER_H