#ifndef IMSTATUSINFO_H
#define IMSTATUSINFO_H

#include <QString>
#include <QIcon>

namespace SIM {

class IMStatusInfo
{
public:
    enum Flag {
        fCheckable = 0,
        fMessage = 1
    };

    IMStatusInfo();

    virtual QString id() const = 0;
    virtual QString name() const = 0;
    virtual QIcon icon() const = 0;
    virtual bool flag(Flag fl) = 0;
    virtual int group() { return DefaultGroup; }

    static const int DefaultGroup = 0;
};

} // namespace SIM

#endif // IMSTATUSINFO_H
