#include <stdarg.h>
#include <stdio.h>
#include <time.h>

#include "console.h"

static void get_formatted_time(char* dest, unsigned short dest_capacity)
{
    //time_t rawtime;
    //struct tm* info;

    //time(&rawtime);
    //info = localtime(&rawtime);

    ///* %x: 08/19/12, %X: 02:50:06 */
    //strftime(dest, dest_capacity, "%x %X", info);

    clock_t clk = clock();

    unsigned short h = clk / (CLOCKS_PER_SEC * 3600) % 24;
    unsigned short m = clk / (CLOCKS_PER_SEC * 60) % 60;
    unsigned short s = (clk / CLOCKS_PER_SEC) % 60;
    unsigned short ms = (1000 * clk) / CLOCKS_PER_SEC % 1000;

    sprintf(dest, "%02u:%02u:%02u.%03u", h, m, s, ms);
}

void log(const char *fmt, ...)
{
    char formatted_time[18], buffer[256];
    va_list args;

    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    get_formatted_time(formatted_time, 18);
    printf("[%s] %s\n", formatted_time, buffer);
}