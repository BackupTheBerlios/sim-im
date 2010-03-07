
#ifndef SIM_TESTUSERDATA_H
#define SIM_TESTUSERDATA_H

#include <QObject>
#include <QtTest/QtTest>

namespace testUserData
{
	class Test : public QObject
	{
        Q_OBJECT
    private slots:
        void initTestCase();
        void cleanupTestCase();

		void userDataManipulation();
	};
}

#endif

