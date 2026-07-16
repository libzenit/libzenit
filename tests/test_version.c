//
//    LibZenit
//    Copyright (C) 2026  Ian Torres
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU Affero General Public License version 3
//    as published by the Free Software Foundation.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU Affero General Public License for more details.
//
//    You should have received a copy of the GNU Affero General Public License
//    along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

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
