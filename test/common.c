#include "common.h"

#include <stdio.h>

double elapsed(struct timeval* tv0, struct timeval* tv1, uint32_t niter, const char* test_case)
{
    if (tv0 == NULL || tv1 == NULL)
        return 0.0;

    double MICROSEC_PER_SEC = 1e6;
    double elapsed_usec     = MICROSEC_PER_SEC * (tv1->tv_sec - tv0->tv_sec) + (tv1->tv_usec - tv0->tv_usec);
    printf("    %28s elapsed = %6.0f usec (%.3f usec per record)\n", test_case ? test_case : "", elapsed_usec, elapsed_usec / niter);
    return elapsed_usec / MICROSEC_PER_SEC;
}
