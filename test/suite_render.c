#include <string.h>

#include <tuibox/tuibox.h>

#include "common.h"
#include "greatest.h"

struct draw_one_call_t
{
    ui_box_t* box;
    int       flush;
    ui_t*     u;
};

struct draw_call_t
{
    ui_t* u;
};

static void call_draw_one(void* ctx)
{
    struct draw_one_call_t* call = (struct draw_one_call_t*)ctx;
    ui_draw_one(call->box, call->flush, call->u);
}

static void call_draw(void* ctx)
{
    struct draw_call_t* call = (struct draw_call_t*)ctx;
    ui_draw(call->u);
}

TEST test_ui_draw_one_clips_right_edge(void)
{
    char                   output[64];
    ui_t                   u;
    int                    id;
    struct draw_one_call_t call;

    init_test_ui(&u);
    u.ws.ws_col = 10;
    u.ws.ws_row = 5;

    id = ui_add(9, 1, 3, 1, 0, NULL, 0, draw_literal, NULL, NULL, "\x1b[31mABC\x1b[0m", NULL, &u);

    call.box   = ui_get(id, &u);
    call.flush = 1;
    call.u     = &u;

    ASSERT_GT(capture_stdout(output, sizeof(output), call_draw_one, &call), 0);
    ASSERT_STR_EQ("\x1b[1;9H\x1b[31mAB\x1b[0m", output);

    cleanup_test_ui(&u);
    PASS();
}

TEST test_ui_draw_inline_preserves_last_frame_position(void)
{
    char               output[256];
    ui_t               u;
    struct draw_call_t call;

    init_test_ui(&u);
    u.mode       = UI_MODE_INLINE;
    u.viewport_w = 6;
    u.viewport_h = 2;

    ui_add(1, 0, 2, 1, 0, NULL, 0, draw_literal, NULL, NULL, "XY", NULL, &u);
    call.u = &u;

    ASSERT_GT(capture_stdout(output, sizeof(output), call_draw, &call), 0);
    ASSERT_NEQ(NULL, strstr(output, "\x1b[s"));
    ASSERT_NEQ(NULL, strstr(output, "\x1b[u"));
    ASSERT_NEQ(NULL, strstr(output, "XY"));
    ASSERT_EQ(1, u.frame_drawn);

    cleanup_test_ui(&u);
    PASS();
}

SUITE(suite_render)
{
    RUN_TEST(test_ui_draw_one_clips_right_edge);
    RUN_TEST(test_ui_draw_inline_preserves_last_frame_position);
}
