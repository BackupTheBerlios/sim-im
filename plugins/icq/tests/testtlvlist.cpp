
#include <gtest/gtest.h>

#include "qt-gtest.h"

#include "tlv.h"
#include "tlvlist.h"

#include "bytearraybuilder.h"

namespace
{
    class TestTlvList : public ::testing::Test
    {
    protected:

        Tlv makeTestTlv(int id)
        {
            return Tlv(id, QByteArray("testdata"));
        }

        QByteArray makeRawTlvList()
        {
            ByteArrayBuilder builder;

            builder.appendWord(0x01);
            QByteArray data01("testdata");
            builder.appendWord(data01.size());
            builder.appendBytes(data01);

            builder.appendWord(0x02);
            QByteArray data02("test");
            builder.appendWord(data02.size());
            builder.appendBytes(data02);

            return builder.getArray();
        }

        QByteArray makeInvalidRawTlvList()
        {
            ByteArrayBuilder builder;

            builder.appendWord(0x01);
            QByteArray data01("testdata");
            builder.appendWord(data01.size());
            builder.appendBytes(data01);

            builder.appendWord(0x02);
            QByteArray data02("test");
            builder.appendWord(data02.size());
            builder.appendBytes(data02.left(2));

            return builder.getArray();
        }
    };

    TEST_F(TestTlvList, TlvManipulation)
    {
        TlvList list;
        Tlv tlv = makeTestTlv(0);

        list.append(tlv);

        ASSERT_EQ(list.size(), 1);
        ASSERT_EQ(tlv.id(), list.at(0).id());
        ASSERT_EQ(tlv.data(), list.at(0).data());
    }

    TEST_F(TestTlvList, fromData)
    {
        TlvList list = TlvList::fromByteArray(makeRawTlvList());

        ASSERT_EQ(2, list.size());
        ASSERT_EQ(0x01, list.at(0).id());
        ASSERT_EQ(QByteArray("testdata"), list.at(0).data());
        ASSERT_EQ(0x02, list.at(1).id());
        ASSERT_EQ(QByteArray("test"), list.at(1).data());
    }

    TEST_F(TestTlvList, fromData_truncatedLastTlv)
    {
        TlvList list = TlvList::fromByteArray(makeInvalidRawTlvList());

        ASSERT_EQ(1, list.size());
    }

    TEST_F(TestTlvList, toByteArray)
    {
        TlvList list;
        list.append(Tlv(0x01, "testdata"));
        list.append(Tlv(0x02, "test"));

        QByteArray rawdata = list.toByteArray();

        ASSERT_EQ(makeRawTlvList(), rawdata);
    }
}
