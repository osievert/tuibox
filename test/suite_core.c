#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "../tuibox.h"
#include "common.h"
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

static void init_test_ui(ui_t* u)
{
    memset(u, 0, sizeof(*u));
    vec_init(&(u->b));
    vec_init(&(u->e));
    u->screen     = 0;
    u->viewport_w = 80;
    u->viewport_h = 24;
    u->ws.ws_col  = 80;
    u->ws.ws_row  = 24;
    u->mode       = UI_MODE_FULLSCREEN;
}

static void cleanup_test_ui(ui_t* u)
{
    ui_box_t* box;
    ui_evt_t* evt;
    int       i;

    vec_foreach(&(u->b), box, i)
    {
        free(box->cache);
        free(box);
    }
    vec_deinit(&(u->b));

    vec_foreach(&(u->e), evt, i)
    {
        free(evt);
    }
    vec_deinit(&(u->e));
}

static void draw_literal(ui_box_t* b, char* out)
{
    strcpy(out, (char*)b->data1);
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

typedef void (*capture_fn_t)(void* ctx);

static int capture_stdout(char* out, size_t out_size, capture_fn_t fn, void* ctx)
{
    int pipefd[2];
    int saved_stdout;
    int nread;

    fflush(stdout);
    if (pipe(pipefd) != 0)
        return -1;

    saved_stdout = dup(STDOUT_FILENO);
    if (saved_stdout < 0)
    {
        close(pipefd[0]);
        close(pipefd[1]);
        return -1;
    }

    if (dup2(pipefd[1], STDOUT_FILENO) < 0)
    {
        close(saved_stdout);
        close(pipefd[0]);
        close(pipefd[1]);
        return -1;
    }

    close(pipefd[1]);

    fn(ctx);
    fflush(stdout);

    dup2(saved_stdout, STDOUT_FILENO);
    close(saved_stdout);

    nread = (int)read(pipefd[0], out, out_size - 1);
    if (nread < 0)
        nread = 0;
    out[nread] = '\0';
    close(pipefd[0]);

    return nread;
}

struct clipped_call_t
{
    char* s;
    int   start;
    int   end;
};

static void call_print_clipped(void* ctx)
{
    struct clipped_call_t* call = (struct clipped_call_t*)ctx;
    _ui_print_clipped(call->s, call->start, call->end);
}

struct draw_one_call_t
{
    ui_box_t* box;
    int       flush;
    ui_t*     u;
};

static void call_draw_one(void* ctx)
{
    struct draw_one_call_t* call = (struct draw_one_call_t*)ctx;
    ui_draw_one(call->box, call->flush, call->u);
}

TEST test_ui_add_centers_with_viewport_and_preserves_screen(void)
{
    ui_t      u;
    int       id;
    ui_box_t* box;

    init_test_ui(&u);
    u.screen     = 2;
    u.viewport_w = 50;
    u.viewport_h = 20;
    u.ws.ws_col  = 120;
    u.ws.ws_row  = 60;

    id  = ui_add(UI_CENTER_X, UI_CENTER_Y, 10, 4, 7, NULL, 0, draw_literal, NULL, NULL, "X", NULL, &u);
    box = ui_get(id, &u);

    ASSERT_EQ(0, id);
    ASSERT_EQ(20, box->x);
    ASSERT_EQ(8, box->y);
    ASSERT_EQ(7, box->screen);

    cleanup_test_ui(&u);
    PASS();
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
    ui_key("q", on_key, &u);

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

TEST test_ui_visible_strlen_ignores_escape_sequences(void)
{
    ASSERT_EQ(3, _ui_visible_strlen("\x1b[31mhi\x1b[0m!"));
    PASS();
}

TEST test_ui_print_clipped_preserves_escape_sequences(void)
{
    char                  output[64];
    struct clipped_call_t call = {"\x1b[31mABCD\x1b[0m", 1, 3};

    ASSERT_EQ_FMT(11, capture_stdout(output, sizeof(output), call_print_clipped, &call), "%d");
    ASSERT_STR_EQ("\x1b[31mBC\x1b[0m", output);
    PASS();
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

SUITE(suite_core)
{
    RUN_TEST(test_ui_add_centers_with_viewport_and_preserves_screen);
    RUN_TEST(test_ui_key_dispatches_registered_handler);
    RUN_TEST(test_ui_update_dispatches_click_and_hover_handlers);
    RUN_TEST(test_ui_clear_inline_resets_boxes_events_and_state);
    RUN_TEST(test_ui_visible_strlen_ignores_escape_sequences);
    RUN_TEST(test_ui_print_clipped_preserves_escape_sequences);
    RUN_TEST(test_ui_draw_one_clips_right_edge);
    RUN_TEST(test_ui_poll_dispatches_pending_input);
}
