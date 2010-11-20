
#include <gtest/gtest.h>
#include "bytearrayparser.h"
#include "qt-gtest.h"

namespace
{
    class TestByteArrayParser : public ::testing::Test
    {
    protected:
        virtual void SetUp()
        {
            arr = QByteArray("\x12\x34\xab\xcd", 4);
        }

        QByteArray arr;
    };

    TEST_F(TestByteArrayParser, readByte)
    {
        ByteArrayParser parser(arr);

        quint8 b = parser.readByte();

        ASSERT_EQ(0x12, b);
    }

    TEST_F(TestByteArrayParser, readWord_bigEndian)
    {
        ByteArrayParser parser(arr);

        quint16 word1 = parser.readWord();
        quint16 word2 = parser.readWord();

        ASSERT_EQ(0x1234, word1);
        ASSERT_EQ(0xabcd, word2);
    }

    TEST_F(TestByteArrayParser, readWord_littleEndian)
    {
        ByteArrayParser parser(arr, ByteArrayParser::LittleEndian);

        quint16 word1 = parser.readWord();
        quint16 word2 = parser.readWord();

        ASSERT_EQ(0x3412, word1);
        ASSERT_EQ(0xcdab, word2);
    }

    TEST_F(TestByteArrayParser, readDword_bigEndian)
    {
        ByteArrayParser parser(arr);

        quint32 dword = parser.readDword();

        ASSERT_EQ(0x1234abcd, dword);
    }

    TEST_F(TestByteArrayParser, readDword_littleEndian)
    {
        ByteArrayParser parser(arr, ByteArrayParser::LittleEndian);

        quint32 dword = parser.readDword();

        ASSERT_EQ(0xcdab3412, dword);
    }

    TEST_F(TestByteArrayParser, readBytes)
    {
        ByteArrayParser parser(arr);

        QByteArray arr = parser.readBytes(2);

        ASSERT_EQ(arr.size(), 2);
        ASSERT_EQ(arr, QByteArray("\x12\x34"));
    }

    TEST_F(TestByteArrayParser, atEnd)
    {
        ByteArrayParser parser(arr);
        ASSERT_FALSE(parser.atEnd());

        parser.readDword();

        ASSERT_TRUE(parser.atEnd());
    }

    TEST_F(TestByteArrayParser, readAll)
    {
        ByteArrayParser parser(arr);
        parser.readByte();

        QByteArray arr = parser.readAll();

        ASSERT_EQ(QByteArray("\x34\xab\xcd"), arr);
    }
}
