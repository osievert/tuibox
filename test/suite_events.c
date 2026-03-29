#include <string.h>

#include "greatest.h"

static int key_calls;
static int click_calls;
static int hover_calls;
static int last_x;
static int last_y;
static int last_down;
static int last_box_id;

static void reset_callbacks(void)
{
    key_calls   = 0;
    click_calls = 0;
    hover_calls = 0;
    last_x      = 0;
    last_y      = 0;
    last_down   = 0;
    last_box_id = -1;
}

static void on_key(void)
{
    key_calls++;
}

static void on_click(ui_box_t* b, int x, int y)
{
    click_calls++;
    last_x      = x;
    last_y      = y;
    last_box_id = b->id;
}

static void on_hover(ui_box_t* b, int x, int y, int down)
{
    hover_calls++;
    last_x      = x;
    last_y      = y;
    last_down   = down;
    last_box_id = b->id;
}

TEST test_ui_key_dispatches_registered_handler(void)
{
    ui_t u;

    init_test_ui(&u);
    reset_callbacks();

    ui_key("q", on_key, &u);
    _ui_update("q", 1, &u);
    _ui_update("x", 1, &u);

    ASSERT_EQ(1, key_calls);

    cleanup_test_ui(&u);
    PASS();
}

TEST test_ui_update_dispatches_click_and_hover_handlers(void)
{
    ui_t u;
    int  id;

    init_test_ui(&u);
    reset_callbacks();

    id = ui_add(5, 5, 4, 3, 0, NULL, 0, draw_literal, on_click, on_hover, "XXXX", NULL, &u);

    _ui_update("\x1b[<0;6;6M", 10, &u);
    ASSERT_EQ(1, click_calls);
    ASSERT_EQ(id, last_box_id);
    ASSERT_EQ(6, last_x);
    ASSERT_EQ(6, last_y);

    _ui_update("\x1b[<32;7;7M", 11, &u);
    ASSERT_EQ(1, hover_calls);
    ASSERT_EQ(id, last_box_id);
    ASSERT_EQ(7, last_x);
    ASSERT_EQ(7, last_y);
    ASSERT_EQ(1, last_down);

    cleanup_test_ui(&u);
    PASS();
}

TEST test_ui_poll_dispatches_pending_input(void)
{
    ui_t u;
    int  pipefd[2];
    int  saved_stdin;

    init_test_ui(&u);
    reset_callbacks();
    ui_key("q", on_key, &u);

    ASSERT_EQ(0, pipe(pipefd));
    saved_stdin = dup(STDIN_FILENO);
    ASSERT_GT(saved_stdin, -1);

    ASSERT_EQ_FMT(1, (int)write(pipefd[1], "q", 1), "%d");
    close(pipefd[1]);
    ASSERT_GT(dup2(pipefd[0], STDIN_FILENO), -1);

    ASSERT_EQ(1, ui_poll(&u));
    ASSERT_EQ(1, key_calls);

    ASSERT_GT(dup2(saved_stdin, STDIN_FILENO), -1);
    close(saved_stdin);
    close(pipefd[0]);

    cleanup_test_ui(&u);
    PASS();
}

SUITE(suite_events)
{
    RUN_TEST(test_ui_key_dispatches_registered_handler);
    RUN_TEST(test_ui_update_dispatches_click_and_hover_handlers);
    RUN_TEST(test_ui_poll_dispatches_pending_input);
}
