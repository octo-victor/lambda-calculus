#ifdef _POSIX_VERSION

#include <time.h>
struct timespec begin;
struct timespec end;

void clock_begin() {
    timespec_get(&begin, TIME_UTC);
}

void clock_end() {
    timespec_get(&end, TIME_UTC);
}

double get_dt() {
    return (end.tv_sec - begin.tv_sec) + (end.tv_nsec - begin.tv_nsec) / 1e9;
}

#elif defined(_WIN32)

#include <windows.h>
LARGE_INTEGER frequency;
LARGE_INTEGER begin;
LARGE_INTEGER end;

void clock_begin() {
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&begin);
}

void clock_end() {
    QueryPerformanceCounter(&end);
}

double get_dt() {
    return (double)(end.QuadPart - begin.QuadPart) / frequency.QuadPart;
}

#else

#include <time.h>
clock_t begin;
clock_t end;

void clock_begin() {
    begin = clock();
}

void clock_end() {
    end = clock();
}

double get_dt() {
    return ((double)(end - begin)) / (double) CLOCKS_PER_SEC;
}

#endif