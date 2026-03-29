/*
 * demo_bounce.c: inline bouncing physics box
 */

#include <math.h>
#include <signal.h>
#include <stdio.h>

#include "../tuibox.h"

#define WINDOW_HEIGHT 5
#define WINDOW_WIDTH 20
#define FRAME_DELAY_US 16000

static volatile sig_atomic_t running = 1;

static ui_t u;

static void stop(int sig)
{
    (void)sig;
    running = 0;
}

static void stop_key(void)
{
    running = 0;
}

static void draw(ui_box_t* b, char* out)
{
    int x, y;

    out[0] = '\0';
    for (y = 0; y < b->h; y++)
    {
        for (x = 0; x < b->w; x++)
        {
            strcat(out, "\x1b[48;2;255;255;255m \x1b[0m");
        }

        if (y + 1 < b->h)
        {
            strcat(out, "\n");
        }
    }
}

int main(void)
{
    int box_id;
    ui_box_t* box;
    double   vx = 0.65, vy = 0.0;
    double   ax = 0.0, ay = 0.08;
    double   px = 4.0, py = 1.0;

    printf("Press 'q' ot ctl-c to quit.\n");

    ui_new_inline(0, WINDOW_WIDTH, WINDOW_HEIGHT, &u);

    box_id = ui_add((int)lround(px),
                    (int)lround(py),
                    2,
                    2,
                    0,
                    NULL,
                    0,
                    draw,
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    &u);
    box = ui_get(box_id, &u);

    signal(SIGTERM, stop);
    signal(SIGQUIT, stop);
    signal(SIGINT, stop);

    ui_key("q", stop_key, &u);

    while (running)
    {
        px += vx;
        py += vy;

        vx += ax;
        vy += ay;

        if (px < 0.0)
        {
            px = 0.0;
            vx *= -1.0;
        }
        else if (px + box->w > WINDOW_WIDTH)
        {
            px = WINDOW_WIDTH - box->w;
            vx *= -1.0;
        }

        if (py < 0.0)
        {
            py = 0.0;
            vy *= -1.0;
        }
        else if (py + box->h > WINDOW_HEIGHT)
        {
            py = WINDOW_HEIGHT - box->h;
            vy *= -0.92;
        }

        box->x = (int)lround(px);
        box->y = (int)lround(py);

        ui_draw(&u);
        usleep(FRAME_DELAY_US);
    }

    ui_free(&u);

    return 0;
}
