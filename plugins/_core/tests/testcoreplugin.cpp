
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "core.h"

namespace {
	class TestCorePlugin : public ::testing::Test
	{
	protected:
	};

	TEST_F(TestCorePlugin, Constructor)
	{
        CorePlugin plugin();
	}
}
