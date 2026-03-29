// Required because tuibox.h is a single header library do we can't include it in multiple
// translation units without causing multiple definition errors. By including all test files in this one
// translation unit, we can avoid multiple definition errors and still have all tests run.
// tuibox.h is included by common.h, so we don't need to include it here.

#include "common.c"
#include "suite_boxes.c"
#include "suite_events.c"
#include "suite_lifecycle.c"
#include "suite_render.c"
