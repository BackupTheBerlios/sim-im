
#ifndef SIM_TESTBUFFER_H
#define SIM_TESTBUFFER_H

#include <QtTest/QtTest>

class TestBuffer : public QObject
{
	Q_OBJECT
private slots:
	void testCommonTypes();
	void testStdintTypes();
	void testScan();

};

#endif

// vim: set expandtab:

