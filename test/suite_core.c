#include <stdint.h>

#include <tuibox.h>

#include "common.h"
#include "greatest.h"

TEST test_dummy(void)
{
    PASS();
}

SUITE(suite_core)
{
    RUN_TEST(test_dummy);
}
