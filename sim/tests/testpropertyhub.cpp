
#include <QDomDocument>
#include "testpropertyhub.h"

namespace testPropertyHub
{
    void Test::initTestCase()
    {
    }

    void Test::cleanupTestCase()
    {
    }

    void Test::testDataManipulation()
    {
        PropertyHubPtr hub = PropertyHubPtr(new PropertyHub("namespace"));
        hub->setValue("intValue", 12);
        QCOMPARE(hub->value("intValue").toInt(), 12);

        hub->setValue("stringValue", "foo");
        QCOMPARE(hub->value("stringValue").toString(), QString("foo"));

        // Test conversion
        hub->setValue("bar", "16");
        QCOMPARE(hub->value("bar").toInt(), 16);

        // Test defaults
        QVERIFY(!hub->value("nonexistant").isValid());
        QCOMPARE(hub->value("nonexistant").toString(), QString(""));
        QCOMPARE(hub->value("nonexistant").toInt(), 0);

        hub->setStringMapValue("map", 12, "value12");
        QCOMPARE(hub->stringMapValue("map", 12), QString("value12"));

        QVERIFY(hub->stringMapValue("map", 25).isEmpty());
        QVERIFY(hub->stringMapValue("foomap", 25).isEmpty());
        QVERIFY(hub->stringMapValue("bar", 42).isEmpty());

        QCOMPARE(hub->allKeys().count(), 4);
        hub->clear();
        QCOMPARE(hub->allKeys().count(), 0);
    }

    void Test::testSerializationInnerByteArray()
    {
        QByteArray arr = "ABC";
        PropertyHubPtr hub = PropertyHub::create();
        QDomDocument doc;
        QDomElement el = doc.createElement("root");

        QVERIFY(hub->serializeByteArray(el, arr));
        QVERIFY(el.firstChild().isText());
        QCOMPARE(el.firstChild().toText().data(), QString("QUJD"));

        el = doc.createElement("root");
        QVERIFY(hub->serializeVariant(el, arr));
        QDomNode value = el.firstChild();
        QVERIFY(value.isText());
        QCOMPARE(value.toText().data(), QString("QUJD"));
    }

    void Test::testSerializationInnerStringList()
    {
        QStringList list;
        list.append("foo");
        list.append("bar");
        list.append("baz");

        PropertyHubPtr hub = PropertyHub::create();
        QDomDocument doc;
        QDomElement listelement = doc.createElement("list");

        QVERIFY(hub->serializeStringList(listelement, list));

        QCOMPARE(listelement.elementsByTagName("string").size(), 3);
        QVERIFY(listelement.elementsByTagName("string").at(0).toElement().firstChild().isText());
        QCOMPARE(listelement.elementsByTagName("string").at(0).toElement().firstChild().toText().data(), QString("foo"));

        QVERIFY(listelement.elementsByTagName("string").at(1).toElement().firstChild().isText());
        QCOMPARE(listelement.elementsByTagName("string").at(1).toElement().firstChild().toText().data(), QString("bar"));
        
        QVERIFY(listelement.elementsByTagName("string").at(2).toElement().firstChild().isText());
        QCOMPARE(listelement.elementsByTagName("string").at(2).toElement().firstChild().toText().data(), QString("baz"));
    }

    void Test::testSerialization()
    {
        QByteArray abc = "ABC";
        QStringList list;
        list.append("alpha");
        list.append("beta");
        list.append("gamma");
        PropertyHubPtr hub = PropertyHubPtr(new PropertyHub("root"));
        hub->setValue("foo", 12);
        hub->setValue("bar", "baz");
        hub->setValue("quux", abc);
        hub->setValue("quuux", list);
        hub->setValue("bool_true", true);
        hub->setValue("bool_false", false);
        hub->setStringMapValue("map", 12, "qux");
        QDomDocument doc;
        QDomElement el = doc.createElement("root");
        QVERIFY(hub->serialize(el));
        PropertyHubPtr anotherhub = PropertyHubPtr(new PropertyHub("root"));
        QVERIFY(anotherhub->deserialize(el));

        QCOMPARE(anotherhub->value("foo").toInt(), 12);
        QCOMPARE(anotherhub->value("bar").toString(), QString("baz"));
        QCOMPARE(anotherhub->stringMapValue("map", 12), QString("qux"));
        QCOMPARE(anotherhub->value("quux").toByteArray(), abc);
        QCOMPARE(anotherhub->value("quuux").toStringList().at(0), QString("alpha"));
        QCOMPARE(anotherhub->value("quuux").toStringList().at(1), QString("beta"));
        QCOMPARE(anotherhub->value("quuux").toStringList().at(2), QString("gamma"));
        QCOMPARE(anotherhub->value("bool_true").toBool(), true);
        QCOMPARE(anotherhub->value("bool_false").toBool(), false);
    }
}

// vim: set expandtab:

