
#include "testuserdata.h"
#include "contacts/userdata.h"

namespace testUserData
{
	void Test::initTestCase()
	{
	}

	void Test::cleanupTestCase()
	{
	}

	void Test::userDataManipulation()
	{
		SIM::UserDataPtr data = SIM::UserData::create();
		SIM::PropertyHubPtr hub = data->getUserData("nonexistant");
		QVERIFY(hub.isNull());
		
		hub = data->createUserData("alpha");
		QVERIFY(!hub.isNull());
		hub->setValue("foo", 12);
		QCOMPARE(hub->value("foo").toInt(), 12);

		SIM::PropertyHubPtr anotherhub = data->getUserData("alpha");
		QCOMPARE(anotherhub->value("foo").toInt(), 12);

		hub.clear();
		anotherhub.clear();
		data->destroyUserData("alpha");
		hub = data->getUserData("alpha");
		QVERIFY(hub.isNull());
	}
}

