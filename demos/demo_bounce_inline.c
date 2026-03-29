/*
 * demo_bounce.c: inline bouncing physics box
 */

#include <math.h>
#include <signal.h>

#include "../tuibox.h"

#define WINDOW_HEIGHT 5
#define WINDOW_WIDTH 10
#define FRAME_DELAY_US 16000

static volatile sig_atomic_t running = 1;
static int frame_drawn = 0;

static void stop(int sig){
  (void)sig;
  running = 0;
}

static void render_frame(const ui_box_t *box){
  int x, y;

  if(frame_drawn){
    printf("\r\x1b[%dA", WINDOW_HEIGHT - 1);
  } else {
    printf("\x1b[?25l");
  }

  for(y=0;y<WINDOW_HEIGHT;y++){
    for(x=0;x<WINDOW_WIDTH;x++){
      if(x >= box->x && x < box->x + box->w &&
         y >= box->y && y < box->y + box->h){
        printf("\x1b[48;2;255;255;255m \x1b[0m");
      } else {
        putchar(' ');
      }
    }

    if(y + 1 < WINDOW_HEIGHT){
      putchar('\n');
    }
  }

  fflush(stdout);
  frame_drawn = 1;
}

int main(void){
  ui_box_t box = {0};
  double vx = 0.65, vy = 0.0;
  double ax = 0.0, ay = 0.08;
  double px = 4.0, py = 1.0;

  box.w = 2;
  box.h = 2;
  box.x = (int)lround(px);
  box.y = (int)lround(py);

  signal(SIGTERM, stop);
  signal(SIGQUIT, stop);
  signal(SIGINT, stop);

  while(running){
    px += vx;
    py += vy;

    vx += ax;
    vy += ay;

    if(px < 0.0){
      px = 0.0;
      vx *= -1.0;
    } else if(px + box.w > WINDOW_WIDTH){
      px = WINDOW_WIDTH - box.w;
      vx *= -1.0;
    }

    if(py < 0.0){
      py = 0.0;
      vy *= -1.0;
    } else if(py + box.h > WINDOW_HEIGHT){
      py = WINDOW_HEIGHT - box.h;
      vy *= -0.92;
    }

    box.x = (int)lround(px);
    box.y = (int)lround(py);

    render_frame(&box);
    usleep(FRAME_DELAY_US);
  }

  if(frame_drawn){
    printf("\x1b[0m\x1b[?25h\n");
  }

  return 0;
}
