
#include <QDomDocument>
#include "testconfig.h"
#include "cfg.h"
#include "propertyhub.h"

namespace testConfig
{
	using namespace SIM;
	void Test::initTestCase()
	{
		ConfigPtr config = ConfigPtr(new Config("nonexistant"));
		PropertyHubPtr alpha = PropertyHub::create("alpha");
		PropertyHubPtr beta = PropertyHub::create("beta");
		PropertyHubPtr gamma = PropertyHub::create("gamma");

		config->rootPropertyHub()->setValue("test1", true);
		config->rootPropertyHub()->setValue("test2", 444);

		alpha->setValue("foo", 12);
		beta->setValue("foo", 23);
		gamma->setValue("foo", 42);

		alpha->setValue("bar", "alpha");
		beta->setValue("bar", "beta");
		gamma->setValue("bar", "gamma");

		QStringList list;
		list.append("foo");
		list.append("bar");
		list.append("baz");
		list.append("banana");
		alpha->setValue("stringlist", list);

		config->addPropertyHub(alpha);
		config->addPropertyHub(beta);
		config->addPropertyHub(gamma);
		m_config = config->serialize();
	}

	void Test::cleanupTestCase()
	{

	}

	void Test::testSerialization()
	{
		QVERIFY(!m_config.isEmpty());
		QDomDocument doc;
		bool valid = doc.setContent(m_config);
		QVERIFY(valid);
		QCOMPARE(doc.elementsByTagName("propertyhub").size(), 4);
	}

	void Test::testDeserialization()
	{
		ConfigPtr config = ConfigPtr(new Config("nonexistant"));
		bool success = config->deserialize(m_config);
		QVERIFY(success);
		QCOMPARE(config->propertyHubCount(), 3);
		PropertyHubPtr alpha = config->propertyHub("alpha");
		PropertyHubPtr beta = config->propertyHub("beta");
		PropertyHubPtr gamma = config->propertyHub("gamma");
		QVERIFY(!alpha.isNull());
		QVERIFY(!beta.isNull());
		QVERIFY(!gamma.isNull());

		QCOMPARE(alpha->value("foo").toInt(), 12);
		QCOMPARE(beta->value("foo").toInt(), 23);
		QCOMPARE(gamma->value("foo").toInt(), 42);

		QCOMPARE(alpha->value("bar").toString(), QString("alpha"));
		QCOMPARE(beta->value("bar").toString(), QString("beta"));
		QCOMPARE(gamma->value("bar").toString(), QString("gamma"));

		QStringList list = alpha->value("stringlist").toStringList();
		QCOMPARE(list.size(), 4);
		QCOMPARE(list.at(0), QString("foo"));
		QCOMPARE(list.at(1), QString("bar"));
		QCOMPARE(list.at(2), QString("baz"));
		QCOMPARE(list.at(3), QString("banana"));

		QCOMPARE(config->rootPropertyHub()->value("test1").toBool(), true);
		QCOMPARE(config->rootPropertyHub()->value("test2").toInt(), 444);
	}
}

// vim: set expandtab:

