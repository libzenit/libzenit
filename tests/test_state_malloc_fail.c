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

#include <libzenit/state.h>
#include <stdio.h>
#include "test_malloc_fail.h"

int main(void) {
    zenit_state_transition_t t = {0, 0, 0, NULL};

    const zenit_state_t *state;
    /* Wrap only the first call; reset immediately so stdio works. */
    malloc_fail_countdown = 0;
    state = zenit_state_allocate(&t, 1, 0);
    malloc_fail_countdown = -1;

    if (state == NULL) {
        printf("PASS: malloc failure returns NULL\n");
        return 0;
    }
    printf("FAIL: expected NULL\n");
    return 1;
}
