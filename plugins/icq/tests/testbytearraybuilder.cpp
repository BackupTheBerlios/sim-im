
#include <gtest/gtest.h>
#include "bytearraybuilder.h"
#include "qt-gtest.h"

namespace
{
    class TestByteArrayBuilder : public ::testing::Test
    {
    protected:
    };

    TEST_F(TestByteArrayBuilder, appendByte)
    {
        ByteArrayBuilder builder;

        builder.appendByte(0x12);
        builder.appendByte(0x34);

        ASSERT_EQ(QByteArray("\x12\x34", 2), builder.getArray());
    }

    TEST_F(TestByteArrayBuilder, appendWord_bigEndian)
    {
        ByteArrayBuilder builder;

        builder.appendWord(0x1234);

        ASSERT_EQ(QByteArray("\x12\x34", 2), builder.getArray());
    }

    TEST_F(TestByteArrayBuilder, appendWord_littleEndian)
    {
        ByteArrayBuilder builder(ByteArrayBuilder::LittleEndian);

        builder.appendWord(0x1234);

        ASSERT_EQ(QByteArray("\x34\x12", 2), builder.getArray());
    }

    TEST_F(TestByteArrayBuilder, appendDword_bigEndian)
    {
        ByteArrayBuilder builder;

        builder.appendDword(0x12345678);

        ASSERT_EQ(QByteArray("\x12\x34\x56\x78", 4), builder.getArray());
    }

    TEST_F(TestByteArrayBuilder, appendDword_littleEndian)
    {
        ByteArrayBuilder builder(ByteArrayBuilder::LittleEndian);

        builder.appendDword(0x12345678);

        ASSERT_EQ(QByteArray("\x78\x56\x34\x12", 4), builder.getArray());
    }

    TEST_F(TestByteArrayBuilder, appendBytes)
    {
        ByteArrayBuilder builder;
        QByteArray first("\x12\x34");
        QByteArray second("\x56\x78");

        builder.appendBytes(first);
        builder.appendBytes(second);

        ASSERT_EQ(first + second, builder.getArray());
    }
}
