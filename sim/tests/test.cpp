
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <QApplication>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
	::testing::InitGoogleTest(&argc, argv);
    ::testing::InitGoogleMock(&argc, argv);
    int ret = RUN_ALL_TESTS();
#ifdef WIN32
    getchar();
#endif
	return ret;
}

