#include "util.h"

#include <stdio.h>
#include <time.h>
#include <string.h>

void human_readable_size(char *buf, size_t buf_sz, double size, unsigned int decimals)
{
    
    double converted = size;

    // set up while loop divided by 1024 depending on what bytes
    int pow = 0;
    while (converted >= 1024) {
        converted = converted / 1024;
        pow++;
    }

    const char *unit[] = {"B", "KiB", "MiB", "GiB", "TiB", "PiB", "EiB", "ZiB"};
    
    snprintf(buf, buf_sz, " %.*f %s", decimals, converted, unit[pow]);

}

size_t simple_time_format(char *buf, size_t buf_sz, time_t time)
{
    struct tm *datef;
    // function localtime <- put the time
    // time(time);
    // and then strftime
    datef = localtime(&time);
    strftime(buf, buf_sz, "%b %d %Y", datef);
    
    int l = strlen(buf);

    if (buf_sz < l) {
        return 0;
    } else {
        return l;
    }
}
