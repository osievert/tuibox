/*
 * demo_bounce.c: inline bouncing physics box
 */

#include <math.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <tuibox/tuibox.h>

#define WINDOW_HEIGHT 4
#define WINDOW_WIDTH 50
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

typedef struct
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
} rgb_t;

typedef struct
{
    uint8_t h;
    uint8_t s;
    uint8_t l;
} hsl_t;

rgb_t hsl_to_rgb(hsl_t hsl)
{
    rgb_t rgb;
    double c = (1.0 - fabs(2.0 * hsl.l / 255.0 - 1.0)) * (hsl.s / 255.0);
    double x = c * (1.0 - fabs(fmod(hsl.h / 42.0, 2) - 1.0));
    double m = hsl.l / 255.0 - c / 2.0;

    if (hsl.h < 42)
    {
        rgb.r = (uint8_t)lround((c + m) * 255);
        rgb.g = (uint8_t)lround((x + m) * 255);
        rgb.b = (uint8_t)lround(m * 255);
    }
    else if (hsl.h < 84)
    {
        rgb.r = (uint8_t)lround((x + m) * 255);
        rgb.g = (uint8_t)lround((c + m) * 255);
        rgb.b = (uint8_t)lround(m * 255);
    }
    else if (hsl.h < 126)
    {
        rgb.r = (uint8_t)lround(m * 255);
        rgb.g = (uint8_t)lround((c + m) * 255);
        rgb.b = (uint8_t)lround((x + m) * 255);
    }
    else if (hsl.h < 168)
    {
        rgb.r = (uint8_t)lround(m * 255);
        rgb.g = (uint8_t)lround((x + m) * 255);
        rgb.b = (uint8_t)lround((c + m) * 255);
    }
    else if (hsl.h < 210)
    {
        rgb.r = (uint8_t)lround((x + m) * 255);
        rgb.g = (uint8_t)lround(m * 255);
        rgb.b = (uint8_t)lround((c + m) * 255);
    }
    else
    {
        rgb.r = (uint8_t)lround((c + m) * 255);
        rgb.g = (uint8_t)lround(m * 255);
        rgb.b = (uint8_t)lround((x + m) * 255);
    }
    return rgb;
}

static void draw(ui_box_t* b, char* out)
{
    int x, y;
    static hsl_t hsl = {0, 255, 128};
    static char buf[256];

    out[0] = '\0';
    for (y = 0; y < b->h; y++)
    {
        for (x = 0; x < b->w; x++)
        {
            rgb_t rgb = hsl_to_rgb(hsl);
            sprintf(buf, "\x1b[48;2;%d;%d;%dm \x1b[0m", rgb.r, rgb.g, rgb.b);
            strcat(out, buf);
            hsl.h = (hsl.h + 3) % 256;
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
    uint32_t frame_counter = 0;

    printf("Press 'q' ot ctl-c to quit.\n");

    ui_new_inline(0, WINDOW_WIDTH, WINDOW_HEIGHT, &u);

    box_id = ui_add((int)lround(px),
                    (int)lround(py),
                    1,
                    1,
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

    while (running && frame_counter++ < 300)
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
        ui_poll(&u);
        usleep(FRAME_DELAY_US);
    }

    ui_free(&u);

    return 0;
}
