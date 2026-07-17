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
#include <libzenit/result.h>
#include <stdio.h>
#include <stdlib.h>

enum { ST_IDLE, ST_ACTIVE, ST_ERROR };
enum { EV_START, EV_STOP, EV_FAIL };

static int callback_invoked = 0;
static int last_event;
static int last_from;
static int last_to;

static void on_transition(int event, int from, int to, void *context) {
    (void)context;
    callback_invoked = 1;
    last_event = event;
    last_from = from;
    last_to = to;
}

int main(void) {
    zenit_state_transition_t table[] = {
        { ST_IDLE,   EV_START, ST_ACTIVE, on_transition },
        { ST_ACTIVE, EV_STOP,  ST_IDLE,   on_transition },
        { ST_ACTIVE, EV_FAIL,  ST_ERROR,  on_transition },
    };
    size_t count = sizeof(table) / sizeof(table[0]);

    zenit_state_t *state = zenit_state_allocate(table, count, ST_IDLE);
    if (state == NULL) {
        fprintf(stderr, "FAIL: allocate returned NULL\n");
        return 1;
    }

    if (zenit_get_last_state(state) != ST_IDLE) {
        fprintf(stderr, "FAIL: expected ST_IDLE after init\n");
        return 1;
    }

    if (zenit_state_process_event(state, EV_START, NULL).error != ZENIT_OK) {
        fprintf(stderr, "FAIL: EV_START should be valid\n");
        return 1;
    }
    if (!callback_invoked) {
        fprintf(stderr, "FAIL: callback was not invoked\n");
        return 1;
    }
    if (last_event != EV_START || last_from != ST_IDLE || last_to != ST_ACTIVE) {
        fprintf(stderr, "FAIL: callback args mismatch\n");
        return 1;
    }
    if (zenit_get_last_state(state) != ST_ACTIVE) {
        fprintf(stderr, "FAIL: expected ST_ACTIVE after EV_START\n");
        return 1;
    }

    callback_invoked = 0;
    if (zenit_state_process_event(state, EV_FAIL, NULL).error != ZENIT_OK) {
        fprintf(stderr, "FAIL: EV_FAIL should be valid from ACTIVE\n");
        return 1;
    }
    if (zenit_get_last_state(state) != ST_ERROR) {
        fprintf(stderr, "FAIL: expected ST_ERROR after EV_FAIL\n");
        return 1;
    }

    callback_invoked = 0;
    if (zenit_state_process_event(state, EV_STOP, NULL).error != ZENIT_ERROR_NOT_FOUND) {
        fprintf(stderr, "FAIL: EV_STOP from ERROR should be invalid\n");
        return 1;
    }
    if (zenit_get_last_state(state) != ST_ERROR) {
        fprintf(stderr, "FAIL: state should remain ST_ERROR after invalid event\n");
        return 1;
    }

    zenit_state_deallocate(state);

    /* NULL deallocate is safe */
    zenit_state_deallocate(NULL);

    printf("PASS: state machine\n");
    return 0;
}
