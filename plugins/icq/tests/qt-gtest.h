#ifndef GTESTQT_H
#define GTESTQT_H

#include <ostream>
#include <QString>

std::ostream& operator<<(std::ostream& out, const QString& str);
std::ostream& operator<<(std::ostream& out, const QByteArray& arr);

#endif // GTESTQT_H
