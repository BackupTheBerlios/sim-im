#include <gtest/gtest.h>

#include "tlv.h"

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
}
