#ifndef GTESTQT_H
#define GTESTQT_H

#include <ostream>
#include <QString>

std::ostream& operator<<(std::ostream& out, const QString& str)
{
    out << '"' << qPrintable(str) << '"';
    return out;
}

std::ostream& operator<<(std::ostream& out, const QByteArray& arr)
{
    out << "QByteArray(" << arr.toHex().data() << ")";
    return out;
}

#endif // GTESTQT_H
