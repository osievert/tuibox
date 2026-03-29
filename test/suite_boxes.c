#include <string.h>

#include "greatest.h"

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

TEST test_ui_text_uses_string_length_for_width(void)
{
    ui_t      u;
    int       id;
    ui_box_t* box;

    init_test_ui(&u);

    id  = ui_text(3, 4, "hello", 0, NULL, NULL, &u);
    box = ui_get(id, &u);

    ASSERT_EQ(5, box->w);
    ASSERT_EQ(1, box->h);
    ASSERT_EQ(3, box->x);
    ASSERT_EQ(4, box->y);
    ASSERT_STR_EQ("hello", (char*)box->data1);

    cleanup_test_ui(&u);
    PASS();
}

SUITE(suite_boxes)
{
    RUN_TEST(test_ui_add_centers_with_viewport_and_preserves_screen);
    RUN_TEST(test_ui_text_uses_string_length_for_width);
}
