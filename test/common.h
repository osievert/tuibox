#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#include <sys/time.h>

double elapsed(struct timeval* tv0, struct timeval* tv1, uint32_t niter, const char* test_case);

#endif // COMMON_H