#include <gtest/gtest.h>

#include "tlv.h"
#include "qt-gtest.h"

namespace
{
    TEST(TestTlv, toUint16)
    {
        QByteArray data("\x12\x34");
        Tlv tlv(0, data);

        ASSERT_EQ(0x1234, tlv.toUint16());
    }

    TEST(TestTlv, toUint32)
    {
        QByteArray data("\x12\x34\x56\x78");
        Tlv tlv(0, data);

        ASSERT_EQ(0x12345678, tlv.toUint32());
    }

    TEST(TestTlv, fromUint16)
    {
        Tlv tlv = Tlv::fromUint16(0, 0x12);

        ASSERT_EQ(QByteArray("\x00\x12", 2), tlv.data());
    }

    TEST(TestTlv, fromUint32)
    {
        Tlv tlv = Tlv::fromUint32(0, 0x12345678);

        ASSERT_EQ(QByteArray("\x12\x34\x56\x78", 4), tlv.data());
    }
}
