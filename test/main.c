#include <stdio.h>

#include "greatest.h"

extern SUITE(suite_core);

GREATEST_MAIN_DEFS();
#ifdef OS_IOS
int runtests(int argc, char* argv[])
#else
int main(int argc, char* argv[])
#endif
{
    GREATEST_MAIN_BEGIN();

    RUN_SUITE(suite_core);

    GREATEST_MAIN_END();
}
