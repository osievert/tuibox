#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/select.h>

#include <tuibox/tuibox.h>

#include "vec.h"

#define COORDINATE_DECODE() \
  tok = strtok(NULL, ";"); \
  x = atoi(tok); \
  tok = strtok(NULL, ";"); \
  y = strtol(tok, NULL, 10) - (u->canscroll ? u->scroll : 0)

#define CLICK_COMPARATOR(x, y, tmp) \
  (u->click == tmp || \
   (box_contains(x, y, tmp) && u->click == NULL))

#define HOVER_COMPARATOR(x, y, tmp) \
  (box_contains(x, y, tmp))

#define LOOP_AND_EXECUTE_CLICK(f) \
  do { \
    vec_foreach(&(u->b), tmp, ind){ \
      if(tmp->screen == u->screen && \
         f != NULL && \
         CLICK_COMPARATOR(x, y, tmp) \
      ){ \
        f(tmp, x, y); \
        u->click = tmp; \
      } \
    } \
  } while(0)

#define LOOP_AND_EXECUTE_HOVER(f) \
  do { \
    vec_foreach(&(u->b), tmp, ind){ \
      if(tmp->screen == u->screen && \
         f != NULL && \
         HOVER_COMPARATOR(x, y, tmp) \
      ){ \
        f(tmp, x, y, u->mouse); \
      } \
    } \
  } while(0)

static int _ui_visible_strlen(char* s)
{
  int n = 0;

  while (*s != '\0')
  {
    if (*s == '\x1b')
    {
      s++;
      if (*s == '[')
      {
        s++;
        while (*s != '\0' && ((*s >= '0' && *s <= '9') || *s == ';' || *s == '?' || *s == '<' || *s == '=' || *s == '>'))
        {
          s++;
        }
      }
      if (*s != '\0')
      {
        s++;
      }
    }
    else
    {
      n++;
      s++;
    }
  }

  return n;
}

static void _ui_print_clipped(char* s, int start, int end)
{
  int col = 0;
  char* esc;

  while (*s != '\0')
  {
    if (*s == '\x1b')
    {
      esc = s;
      s++;
      if (*s == '[')
      {
        s++;
        while (*s != '\0' && ((*s >= '0' && *s <= '9') || *s == ';' || *s == '?' || *s == '<' || *s == '=' || *s == '>'))
        {
          s++;
        }
      }
      if (*s != '\0')
      {
        s++;
      }
      fwrite(esc, 1, s - esc, stdout);
    }
    else
    {
      if (col >= start && col < end)
      {
        putchar(*s);
      }
      col++;
      s++;
    }
  }
}

static void _ui_new(int s, int mode, int w, int h, ui_t* u)
{
  struct termios raw;

  ioctl(STDOUT_FILENO, TIOCGWINSZ, &(u->ws));

  tcgetattr(STDIN_FILENO, &(u->tio));
  raw = u->tio;
  raw.c_lflag &= ~(ECHO | ICANON);
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);

  vec_init(&(u->b));
  vec_init(&(u->e));

  u->click = NULL;

  if (mode == UI_MODE_FULLSCREEN)
  {
    printf("\x1b[?1049h\x1b[0m\x1b[2J\x1b[?1003h\x1b[?1015h\x1b[?1006h\x1b[?25l");
  }
  else
  {
    printf("\x1b[?1003h\x1b[?1015h\x1b[?1006h\x1b[?25l");
  }

  u->mouse = 0;
  u->screen = s;
  u->scroll = 0;
  u->canscroll = (mode == UI_MODE_FULLSCREEN);
  u->id = 0;
  u->force = 0;
  u->mode = mode;
  u->viewport_w = (w > 0 ? w : u->ws.ws_col);
  u->viewport_h = (h > 0 ? h : u->ws.ws_row);
  u->frame_drawn = 0;
  u->drawing = 0;
}

static void _ui_draw_one_inline(ui_box_t* tmp, ui_t* u)
{
  char *buf, *tok;
  int n = 0;
  int start_x, end_x;

  if (tmp->screen != u->screen)
    return;

  buf = (char*)calloc(1, strlen(tmp->cache) * 2);
  if (u->force || tmp->watch == NULL || *(tmp->watch) != tmp->last)
  {
    tmp->draw(tmp, buf);
    if (tmp->watch != NULL)
      tmp->last = *(tmp->watch);
    strcpy(tmp->cache, buf);
  }
  else
  {
    strcpy(buf, tmp->cache);
  }

  tok = strtok(buf, "\n");
  while (tok != NULL)
  {
    start_x = (tmp->x < 0 ? 0 : tmp->x);
    end_x = tmp->x + _ui_visible_strlen(tok);
    if (end_x > u->viewport_w)
      end_x = u->viewport_w;

    if (start_x < end_x && tmp->y + n >= 0 && tmp->y + n < u->viewport_h)
    {
      printf("\x1b[u");
      if (tmp->y + n > 0)
        printf("\x1b[%iB", tmp->y + n);
      if (start_x > 0)
        printf("\x1b[%iC", start_x);
      _ui_print_clipped(tok, start_x - tmp->x, end_x - tmp->x);
    }
    tok = strtok(NULL, "\n");
    n++;
  }

  free(buf);
}

static void _ui_text(ui_box_t* b, char* out)
{
  sprintf(out, "%s", (char*)b->data1);
}

void ui_new(int s, ui_t* u)
{
  _ui_new(s, UI_MODE_FULLSCREEN, 0, 0, u);
}

void ui_new_inline(int s, int w, int h, ui_t* u)
{
  _ui_new(s, UI_MODE_INLINE, w, h, u);
}

void ui_free(ui_t* u)
{
  ui_box_t* val;
  ui_evt_t* evt;
  int i;
  char* term;

  if (u->mode == UI_MODE_FULLSCREEN)
  {
    printf("\x1b[0m\x1b[2J\x1b[?1049l\x1b[?1003l\x1b[?1015l\x1b[?1006l\x1b[?25h");
  }
  else
  {
    printf("\x1b[0m\x1b[?1003l\x1b[?1015l\x1b[?1006l\x1b[?25h");
    if (u->frame_drawn)
    {
      putchar('\n');
    }
  }
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &(u->tio));

  vec_foreach(&(u->b), val, i)
  {
    free(val->cache);
    free(val);
  }
  vec_deinit(&(u->b));

  vec_foreach(&(u->e), evt, i)
  {
    free(evt);
  }
  vec_deinit(&(u->e));

  term = getenv("TERM");
  if (term != NULL && (strncmp(term, "screen", 6) == 0 || strncmp(term, "tmux", 4) == 0))
  {
    printf("Note: Terminal multiplexer detected.\n  For best performance (i.e. reduced flickering), running natively inside\n  a GPU-accelerated terminal such as alacritty or kitty is recommended.\n");
  }
}

int ui_add(int x, int y, int w, int h, int screen, char* watch, char initial, ui_draw_func_t draw, ui_click_func_t onclick, ui_hover_func_t onhover, void* data1, void* data2, ui_t* u)
{
  ui_box_t* b = (ui_box_t*)malloc(sizeof(ui_box_t));

  b->id = u->id++;
  b->x = (x == UI_CENTER_X ? ui_center_x(w, u) : x);
  b->y = (y == UI_CENTER_Y ? ui_center_y(h, u) : y);
  b->w = w;
  b->h = h;
  b->screen = screen;
  b->watch = watch;
  b->last = initial;
  b->draw = draw;
  b->onclick = onclick;
  b->onhover = onhover;
  b->data1 = data1;
  b->data2 = data2;
  b->cache = (char*)malloc(MAXCACHESIZE);
  draw(b, b->cache);
  b->cache = (char*)realloc(b->cache, strlen(b->cache) * 2);

  vec_push(&(u->b), b);

  return b->id;
}

void ui_key(char* c, ui_key_func_t f, ui_t* u)
{
  ui_evt_t* e = (ui_evt_t*)malloc(sizeof(ui_evt_t));
  e->c = c;
  e->f = f;

  vec_push(&(u->e), e);
}

void ui_clear(ui_t* u)
{
  ui_box_t* box;
  ui_evt_t* evt;
  int tmp = u->screen;
  int i;

  if (u->mode == UI_MODE_INLINE)
  {
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

    vec_init(&(u->b));
    vec_init(&(u->e));
    u->click = NULL;
    u->screen = tmp;
    u->scroll = 0;
    u->id = 0;
    u->force = 1;
    return;
  }

  ui_free(u);
  ui_new(tmp, u);
}

void ui_draw_one(ui_box_t* tmp, int flush, ui_t* u)
{
  char *buf, *tok;
  int n = -1;

  if (u->mode == UI_MODE_INLINE)
  {
    if (u->drawing)
    {
      _ui_draw_one_inline(tmp, u);
      if (flush)
        fflush(stdout);
    }
    else
    {
      ui_draw(u);
    }
    return;
  }

  if (tmp->screen != u->screen)
    return;

  buf = (char*)calloc(1, strlen(tmp->cache) * 2);
  if (u->force || tmp->watch == NULL || *(tmp->watch) != tmp->last)
  {
    tmp->draw(tmp, buf);
    if (tmp->watch != NULL)
      tmp->last = *(tmp->watch);
    strcpy(tmp->cache, buf);
  }
  else
  {
    strcpy(buf, tmp->cache);
  }

  tok = strtok(buf, "\n");
  while (tok != NULL)
  {
    if (tmp->x > 0 && tmp->x <= u->ws.ws_col && CURSOR_Y(tmp) > 0 && CURSOR_Y(tmp) <= u->ws.ws_row)
    {
      printf("\x1b[%i;%iH", CURSOR_Y(tmp), tmp->x);
      _ui_print_clipped(tok, 0, u->ws.ws_col - tmp->x + 1);
      n++;
    }
    tok = strtok(NULL, "\n");
  }
  free(buf);

  if (flush)
    fflush(stdout);
}

void ui_draw(ui_t* u)
{
  ui_box_t* tmp;
  int i;

  if (u->mode == UI_MODE_INLINE)
  {
    if (u->frame_drawn)
    {
      printf("\r");
      if (u->viewport_h > 1)
        printf("\x1b[%iA", u->viewport_h - 1);
    }

    for (i = 0; i < u->viewport_h; i++)
    {
      printf("\x1b[0m");
      printf("%*s", u->viewport_w, "");
      if (i + 1 < u->viewport_h)
        putchar('\n');
    }

    printf("\r");
    if (u->viewport_h > 1)
      printf("\x1b[%iA", u->viewport_h - 1);
    printf("\x1b[s");

    u->drawing = 1;
    vec_foreach(&(u->b), tmp, i)
    {
      _ui_draw_one_inline(tmp, u);
    }
    u->drawing = 0;

    printf("\x1b[u");
    if (u->viewport_h > 1)
      printf("\x1b[%iB", u->viewport_h - 1);
    printf("\r");

    fflush(stdout);
    u->force = 0;
    u->frame_drawn = 1;
    return;
  }

  printf("\x1b[0m\x1b[2J");

  vec_foreach(&(u->b), tmp, i)
  {
    ui_draw_one(tmp, 0, u);
  }
  fflush(stdout);
  u->force = 0;
}

void ui_redraw(ui_t* u)
{
  u->force = 1;
  ui_draw(u);
}

void _ui_update(char* c, int n, ui_t* u)
{
  ui_box_t* tmp;
  ui_evt_t* evt;
  int ind, x, y;
  char cpy[n], *tok;

  if (n >= 4 && c[0] == '\x1b' && c[1] == '[' && c[2] == '<')
  {
    strncpy(cpy, c, n);
    tok = strtok(cpy + 3, ";");

    switch (tok[0])
    {
      case '0':
        u->mouse = (strchr(c, 'm') == NULL);
        COORDINATE_DECODE();
        LOOP_AND_EXECUTE_CLICK(tmp->onclick);
        if (!u->mouse)
        {
          u->click = NULL;
        }
        break;
      case '3':
        u->mouse = (strcmp(tok, "32") == 0);
        COORDINATE_DECODE();
        LOOP_AND_EXECUTE_HOVER(tmp->onhover);
        break;
      case '6':
        if (u->canscroll)
        {
          u->scroll += (4 * (tok[1] == '4')) - 2;
          if (u->mode == UI_MODE_FULLSCREEN)
          {
            printf("\x1b[0m\x1b[2J");
          }
          ui_draw(u);
        }
        break;
    }
  }

  vec_foreach(&(u->e), evt, ind)
  {
    if (strncmp(c, evt->c, strlen(evt->c)) == 0 && evt->f != NULL)
      evt->f();
  }
}

int ui_poll(ui_t* u)
{
  char buf[64];
  fd_set rfds;
  struct timeval tv;
  int n;

  FD_ZERO(&rfds);
  FD_SET(STDIN_FILENO, &rfds);

  tv.tv_sec = 0;
  tv.tv_usec = 0;

  n = select(STDIN_FILENO + 1, &rfds, NULL, NULL, &tv);
  if (n <= 0 || !FD_ISSET(STDIN_FILENO, &rfds))
  {
    return 0;
  }

  n = read(STDIN_FILENO, buf, sizeof(buf));
  if (n > 0)
  {
    _ui_update(buf, n, u);
    return n;
  }

  return 0;
}

int ui_text(int x, int y, char* str, int screen, ui_click_func_t click, ui_hover_func_t hover, ui_t* u)
{
  return ui_add(x, y, strlen(str), 1, screen, NULL, 0, _ui_text, click, hover, str, NULL, u);
}
