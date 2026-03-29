#ifndef COMMON_H
#define COMMON_H

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#include <sys/time.h>

#include <tuibox/tuibox.h>

double elapsed(struct timeval* tv0, struct timeval* tv1, uint32_t niter, const char* test_case);

typedef void (*capture_fn_t)(void* ctx);

void draw_literal(ui_box_t* b, char* out);
void init_test_ui(ui_t* u);
void cleanup_test_ui(ui_t* u);
int  capture_stdout(char* out, size_t out_size, capture_fn_t fn, void* ctx);
int  capture_with_pty(char* out, size_t out_size, capture_fn_t fn, void* ctx);

#endif // COMMON_H
