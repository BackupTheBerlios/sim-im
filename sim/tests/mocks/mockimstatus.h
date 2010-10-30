#ifndef MOCKIMSTATUS_H
#define MOCKIMSTATUS_H

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "contacts/imstatus.h"
#include <QSharedPointer>

namespace MockObjects
{
    class MockIMStatus : public SIM::IMStatus
    {
    public:
        MOCK_CONST_METHOD0(id, QString());
        MOCK_CONST_METHOD0(name, QString());
        MOCK_CONST_METHOD0(hasText, bool());
        MOCK_METHOD1(setText, void(const QString& t));
        MOCK_CONST_METHOD0(text, QString());
        MOCK_CONST_METHOD0(icon, QPixmap());
        MOCK_CONST_METHOD0(group, int());
        MOCK_CONST_METHOD1(flag, bool(Flag fl));
        MOCK_METHOD2(setFlag, void(Flag fl, bool val));

        MOCK_METHOD0(clone, SIM::IMStatusPtr());
    };
    typedef QSharedPointer<MockIMStatus> MockIMStatusPtr;
}

#endif // MOCKIMSTATUS_H
