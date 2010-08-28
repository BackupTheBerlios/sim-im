#ifndef TESTEVENTHUB_H
#define TESTEVENTHUB_H

#include <QObject>
#include <QtTest>

namespace testEventHub
{

class Test : public QObject
{
    Q_OBJECT

public slots:
    void testEvent1();

private slots:
    void init();
    void cleanup();

    void testRegistration();
    void testEventConnection();

private:
    int m_testEvent1;

};

}

#endif // TESTEVENTHUB_H
