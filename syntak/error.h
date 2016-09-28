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

#ifndef SYNTAKSRC_SYNTAK_ERROR_H
#define SYNTAKSRC_SYNTAK_ERROR_H

#include <exception>
#include <QString>
#include <QDebug>


#define SYNTAK_ERROR(arg__) \
    { throw ::Syntak::SyntakException() << arg__; }

namespace Syntak {

class SyntakException : public std::exception
{
public:
    SyntakException() noexcept { }

    const char* what() const noexcept override
        { return p_text.toStdString().c_str(); }

    const QString& text() const { return p_text; }

    template <class T>
    SyntakException& operator << (const T& v)
    {
        QDebug d(&p_text); d.noquote().nospace() << v;
        return *this;
    }

private:
    QString p_text;
};

} // namespace Syntak


#endif // SYNTAKSRC_SYNTAK_ERROR_H

