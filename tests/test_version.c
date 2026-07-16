#include <libzenit/version.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    libzenit_version_t v = libzenit_version();

    if (v.major != 0) {
        fprintf(stderr, "FAIL: major expected 0 got %d\n", v.major);
        return 1;
    }
    if (v.minor != 1) {
        fprintf(stderr, "FAIL: minor expected 1 got %d\n", v.minor);
        return 1;
    }
    if (v.patch != 0) {
        fprintf(stderr, "FAIL: patch expected 0 got %d\n", v.patch);
        return 1;
    }

    printf("PASS: libzenit v%d.%d.%d (%s)\n", v.major, v.minor, v.patch, v.name);
    return 0;
}
