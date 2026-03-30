/*
 * tuibox.h: simple tui library
 */

#ifndef TUIBOX_H
#define TUIBOX_H

#include <termios.h>
#include <unistd.h>

#include <sys/ioctl.h>

/*
 * PREPROCESSOR
 */
#define MAXCACHESIZE 65535

#define CURSOR_Y(b) ((b)->y + (n + 1) + ((u)->canscroll ? (u)->scroll : 0))

#define box_contains(x, y, b) ((x) >= (b)->x && (x) <= (b)->x + (b)->w && (y) >= (b)->y && (y) <= (b)->y + (b)->h)

#define ui_screen(s, u) (u)->screen = (s); (u)->force = 1

#define ui_center_x(w, u) (((u)->viewport_w - (w)) / 2)
#define ui_center_y(h, u) (((u)->viewport_h - (h)) / 2)

#define UI_CENTER_X -1
#define UI_CENTER_Y -1

#define UI_MODE_FULLSCREEN 0
#define UI_MODE_INLINE 1

/* The argument isn't actually necessary here, but it helps with design consistency */
#define ui_loop(u) char buf[64]; int n; while ((n = read(STDIN_FILENO, buf, sizeof(buf))) > 0)

#define ui_update(u) _ui_update(buf, n, u)

#define ui_get(id, u) ((u)->b.data[id])

/*
 * TYPES
 */
typedef struct ui_box_t ui_box_t;
typedef struct ui_evt_t ui_evt_t;

typedef void (*ui_draw_func_t)(ui_box_t* b, char* out);
typedef void (*ui_click_func_t)(ui_box_t* b, int x, int y);
typedef void (*ui_hover_func_t)(ui_box_t* b, int x, int y, int down);
typedef void (*ui_key_func_t)(void);

struct ui_box_t {
  int id;
  int x, y;
  int w, h;
  int screen;
  char* cache;
  char* watch;
  char last;
  ui_draw_func_t draw;
  ui_click_func_t onclick;
  ui_hover_func_t onhover;
  void* data1;
  void* data2;
};

struct ui_evt_t {
  char* c;
  ui_key_func_t f;
};

typedef struct vec_box_t {
  ui_box_t** data;
  int length;
  int capacity;
} vec_box_t;

typedef struct vec_evt_t {
  ui_evt_t** data;
  int length;
  int capacity;
} vec_evt_t;

typedef struct ui_t {
  struct termios tio;
  struct winsize ws;
  vec_box_t b;
  vec_evt_t e;
  ui_box_t* click;
  int mouse;
  int screen;
  int scroll;
  int canscroll;
  int id;
  int force;
  int mode;
  int viewport_w;
  int viewport_h;
  int frame_drawn;
  int drawing;
} ui_t;

/*
 * API
 */
void ui_new(int s, ui_t* u);
void ui_new_inline(int s, int w, int h, ui_t* u);
void ui_free(ui_t* u);

int ui_add(
  int x,
  int y,
  int w,
  int h,
  int screen,
  char* watch,
  char initial,
  ui_draw_func_t draw,
  ui_click_func_t onclick,
  ui_hover_func_t onhover,
  void* data1,
  void* data2,
  ui_t* u
);

void ui_key(char* c, ui_key_func_t f, ui_t* u);
void ui_clear(ui_t* u);
void ui_draw_one(ui_box_t* tmp, int flush, ui_t* u);
void ui_draw(ui_t* u);
void ui_redraw(ui_t* u);
void _ui_update(char* c, int n, ui_t* u);
int ui_poll(ui_t* u);

int ui_text(
  int x,
  int y,
  char* str,
  int screen,
  ui_click_func_t click,
  ui_hover_func_t hover,
  ui_t* u
);

#endif
