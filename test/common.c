#include "common.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/wait.h>

#ifdef __APPLE__
#include <util.h>
#else
#include <pty.h>
#endif

double elapsed(struct timeval* tv0, struct timeval* tv1, uint32_t niter, const char* test_case)
{
    if (tv0 == NULL || tv1 == NULL)
        return 0.0;

    double MICROSEC_PER_SEC = 1e6;
    double elapsed_usec     = MICROSEC_PER_SEC * (tv1->tv_sec - tv0->tv_sec) + (tv1->tv_usec - tv0->tv_usec);
    printf("    %28s elapsed = %6.0f usec (%.3f usec per record)\n", test_case ? test_case : "", elapsed_usec, elapsed_usec / niter);
    return elapsed_usec / MICROSEC_PER_SEC;
}

void draw_literal(ui_box_t* b, char* out)
{
    strcpy(out, (char*)b->data1);
}

void init_test_ui(ui_t* u)
{
    memset(u, 0, sizeof(*u));
    if (tcgetattr(STDIN_FILENO, &(u->tio)) != 0)
    {
        memset(&(u->tio), 0, sizeof(u->tio));
        perror("tcgetattr() error");
    }

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &(u->ws)) != 0)
    {
        u->ws.ws_col = 80;
        u->ws.ws_row = 24;
    }

    u->screen     = 0;
    u->viewport_w = 80;
    u->viewport_h = 24;
    u->mode       = UI_MODE_FULLSCREEN;
}

void cleanup_test_ui(ui_t* u)
{
    ui_box_t* box;
    ui_evt_t* evt;
    int       i;

    for (i = 0; i < u->b.length; i++)
    {
        box = u->b.data[i];
        free(box->cache);
        free(box);
    }
    free(u->b.data);
    u->b.data = NULL;
    u->b.length = 0;
    u->b.capacity = 0;

    for (i = 0; i < u->e.length; i++)
    {
        evt = u->e.data[i];
        free(evt);
    }
    free(u->e.data);
    u->e.data = NULL;
    u->e.length = 0;
    u->e.capacity = 0;
}

int capture_stdout(char* out, size_t out_size, capture_fn_t fn, void* ctx)
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

int capture_with_pty(char* out, size_t out_size, capture_fn_t fn, void* ctx)
{
    int            master_fd;
    int            slave_fd;
    int            status;
    int            total = 0;
    pid_t          pid;
    ssize_t        nread;
    struct winsize ws;

    memset(&ws, 0, sizeof(ws));
    ws.ws_col = 80;
    ws.ws_row = 24;

    if (openpty(&master_fd, &slave_fd, NULL, NULL, &ws) != 0)
        return -1;

    pid = fork();
    if (pid < 0)
    {
        close(master_fd);
        close(slave_fd);
        return -1;
    }

    if (pid == 0)
    {
        fflush(stdout);
        fflush(stderr);

        close(master_fd);

        if (dup2(slave_fd, STDIN_FILENO) < 0 ||
            dup2(slave_fd, STDOUT_FILENO) < 0 ||
            dup2(slave_fd, STDERR_FILENO) < 0)
        {
            close(slave_fd);
            _exit(127);
        }

        close(slave_fd);

        fn(ctx);
        fflush(stdout);
        fflush(stderr);
        _exit(0);
    }

    close(slave_fd);

    while (total + 1 < (int)out_size)
    {
        nread = read(master_fd, out + total, out_size - 1 - total);
        if (nread <= 0)
            break;
        total += (int)nread;
    }

    out[total] = '\0';
    close(master_fd);
    waitpid(pid, &status, 0);

    return total;
}
