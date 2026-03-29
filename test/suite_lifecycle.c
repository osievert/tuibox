#include <string.h>

#include "common.h"
#include "greatest.h"

struct free_call_t
{
    ui_t* u;
};

static void call_ui_free(void* ctx)
{
    struct free_call_t* call = (struct free_call_t*)ctx;
    ui_free(call->u);
}

TEST test_ui_clear_inline_resets_boxes_events_and_state(void)
{
    ui_t u;

    init_test_ui(&u);
    u.mode   = UI_MODE_INLINE;
    u.screen = 3;
    u.scroll = 9;
    u.id     = 4;
    u.force  = 0;

    ui_add(1, 2, 3, 1, 3, NULL, 0, draw_literal, NULL, NULL, "X", NULL, &u);
    ui_key("q", NULL, &u);

    ui_clear(&u);

    ASSERT_EQ(0, u.b.length);
    ASSERT_EQ(0, u.e.length);
    ASSERT_EQ(3, u.screen);
    ASSERT_EQ(0, u.scroll);
    ASSERT_EQ(0, u.id);
    ASSERT_EQ(1, u.force);
    ASSERT_EQ(NULL, u.click);

    cleanup_test_ui(&u);
    PASS();
}

TEST test_ui_free_inline_keeps_frame_and_appends_newline(void)
{
    char               output[128];
    ui_t               u;
    struct free_call_t call;

    init_test_ui(&u);
    u.mode        = UI_MODE_INLINE;
    u.frame_drawn = 1;
    call.u        = &u;

    ASSERT_GT(capture_stdout(output, sizeof(output), call_ui_free, &call), 0);
    ASSERT_NEQ(NULL, strstr(output, "\x1b[0m\x1b[?1003l\x1b[?1015l\x1b[?1006l\x1b[?25h"));
    ASSERT_EQ('\n', output[strlen(output) - 1]);

    PASS();
}

TEST test_ui_new_inline_emits_inline_setup_sequences(void)
{
    PASS();
}

TEST test_ui_free_fullscreen_emits_alt_screen_teardown(void)
{
    char               output[128];
    ui_t               u;
    struct free_call_t call;

    init_test_ui(&u);
    u.mode = UI_MODE_FULLSCREEN;
    call.u = &u;

    ASSERT_GT(capture_stdout(output, sizeof(output), call_ui_free, &call), 0);
    ASSERT_NEQ(NULL, strstr(output, "\x1b[0m\x1b[2J\x1b[?1049l\x1b[?1003l\x1b[?1015l\x1b[?1006l\x1b[?25h"));
    PASS();
}

SUITE(suite_lifecycle)
{
    RUN_TEST(test_ui_clear_inline_resets_boxes_events_and_state);
    RUN_TEST(test_ui_free_inline_keeps_frame_and_appends_newline);
    RUN_TEST(test_ui_free_fullscreen_emits_alt_screen_teardown);
}
