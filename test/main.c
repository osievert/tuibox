#include <stdio.h>

#include "greatest.h"

extern SUITE(suite_boxes);
extern SUITE(suite_events);
extern SUITE(suite_lifecycle);
extern SUITE(suite_render);

GREATEST_MAIN_DEFS();

#ifdef OS_IOS
int runtests(int argc, char* argv[])
#else
int main(int argc, char* argv[])
#endif
{
    GREATEST_MAIN_BEGIN();

    RUN_SUITE(suite_boxes);
    RUN_SUITE(suite_events);
    RUN_SUITE(suite_lifecycle);
    RUN_SUITE(suite_render);

    GREATEST_MAIN_END();
}
