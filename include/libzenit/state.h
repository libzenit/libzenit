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

#ifndef LIBZENIT_STATE_H
#define LIBZENIT_STATE_H

#include <stddef.h>

typedef void (*zenit_state_callback_t)(int event, int from_state, int to_state, void *context);

typedef struct {
    int from_state;
    int event;
    int to_state;
    zenit_state_callback_t on_transition;
} zenit_state_transition_t;

typedef struct zenit_state_t zenit_state_t;

zenit_state_t *zenit_state_allocate(
    const zenit_state_transition_t *table,
    size_t count,
    int initial_state
);

int  zenit_state_process_event(zenit_state_t *state, int event, void *context);
int  zenit_get_last_state(const zenit_state_t *state);
void zenit_state_deallocate(zenit_state_t *state);

#endif
