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
#include <stdlib.h>

struct zenit_state_t {
    const zenit_state_transition_t *table;
    size_t count;
    int current;
};

zenit_state_t *zenit_state_allocate(
    const zenit_state_transition_t *table,
    size_t count,
    int initial_state
) {
    zenit_state_t *state = malloc(sizeof(zenit_state_t));
    if (state == NULL) {
        return NULL;
    }
    state->table = table;
    state->count = count;
    state->current = initial_state;
    return state;
}

int zenit_state_process_event(zenit_state_t *state, int event, void *context) {
    int from = state->current;

    for (size_t i = 0; i < state->count; i++) {
        const zenit_state_transition_t *t = &state->table[i];
        if (t->from_state == from && t->event == event) {
            state->current = t->to_state;
            if (t->on_transition != NULL) {
                t->on_transition(event, from, t->to_state, context);
            }
            return 0;
        }
    }
    return -1;
}

int zenit_get_last_state(const zenit_state_t *state) {
    return state->current;
}

void zenit_state_deallocate(zenit_state_t *state) {
    free(state);
}
