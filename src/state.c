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

/**
 * @brief Internal (opaque) representation of a finite-state machine.
 *
 * Holds a pointer to the caller-owned transition table, the number of
 * entries in that table, and the current state value.
 */
struct zenit_state_t {
    const zenit_state_transition_t *table; /**< Transition rules — NOT owned here */
    size_t count;                          /**< Number of rules in @p table */
    int current;                           /**< Current state value */
};

/**
 * @brief Allocate and initialise a state machine.
 *
 * @param table         Caller-owned transition array (must outlive the machine).
 * @param count         Number of entries in @p table.
 * @param initial_state State the machine should start in.
 * @return Opaque handle or NULL if malloc fails.
 */
zenit_state_t *zenit_state_allocate(
    const zenit_state_transition_t *table,
    size_t count,
    int initial_state
) {
    /* Allocate heap memory for the opaque struct */
    zenit_state_t *state = malloc(sizeof(zenit_state_t));
    /* If malloc returned NULL there is nothing we can do — propagate the failure */
    if (state == NULL) {
        return NULL;
    }
    /* Stash the pointer to the caller's transition table (we do NOT copy it) */
    state->table = table;
    /* Remember how many rules we must scan on each event */
    state->count = count;
    /* Seed the machine at the caller-chosen initial state */
    state->current = initial_state;
    return state;
}

/**
 * @brief Process an event: walk the transition table and, on match, move
 *        to the target state and fire the optional callback.
 *
 * @param state   Machine handle.
 * @param event   Event identifier to match against table entries.
 * @param context Opaque value forwarded to the callback.
 * @return 0 on successful transition, -1 when no rule matches.
 */
int zenit_state_process_event(zenit_state_t *state, int event, void *context) {
    /* Snapshot the current state before we start matching */
    int from = state->current;

    /* Linear scan over every entry in the transition table — O(n) */
    for (size_t i = 0; i < state->count; i++) {
        /* Grab a pointer to the i-th rule (avoids copying the struct) */
        const zenit_state_transition_t *t = &state->table[i];
        /* Both from_state AND event must match for this rule to apply */
        if (t->from_state == from && t->event == event) {
            /* Rule matched — commit the transition */
            state->current = t->to_state;
            /* If the caller registered a callback, fire it now */
            if (t->on_transition != NULL) {
                /* Callback receives: event, previous state, new state, user context */
                t->on_transition(event, from, t->to_state, context);
            }
            return 0;
        }
    }
    /* No rule matched — return error code; machine state is unchanged */
    return -1;
}

/**
 * @brief Return the current state value. Pure accessor, no side effects.
 */
int zenit_get_last_state(const zenit_state_t *state) {
    return state->current;
}

/**
 * @brief Free the machine handle. Does NOT free the transition table.
 *
 * @param state Handle to free (NULL is accepted and ignored).
 */
void zenit_state_deallocate(zenit_state_t *state) {
    /* free() is specified to do nothing when passed NULL; the guard is implicit */
    free(state);
}
