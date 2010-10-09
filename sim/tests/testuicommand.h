#ifndef TESTUICOMMAND_H
#define TESTUICOMMAND_H

#include <QObject>

namespace Test
{
    class TestObject : public QObject
    {
        Q_OBJECT
    public:
        TestObject() : calls(0) {}
        int calls;

    public slots:
        void cmdTriggered()
        {
            calls++;
        }
    };

}

#endif // TESTUICOMMAND_H
