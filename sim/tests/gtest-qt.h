#ifndef GTESTQT_H
#define GTESTQT_H

#include <ostream>
#include <QString>

std::ostream& operator<<(std::ostream& out, const QString& str)
{
    out << '"' << qPrintable(str) << '"';
    return out;
}

#endif // GTESTQT_H
