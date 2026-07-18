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

#include <libzenit/allocator.h>
#include <libzenit/result.h>
#include <stddef.h>

/**
 * @brief Callback invoked when a state transition occurs.
 *
 * @param event      The event that triggered the transition.
 * @param from_state The state the machine was in before the transition.
 * @param to_state   The state the machine transitions to.
 * @param context    Opaque user pointer forwarded from the process_event call.
 */
typedef void (*zenit_state_callback_t)(int event, int from_state, int to_state, void *context);

/**
 * @brief A single entry in the state transition table.
 *
 * Defines a directed edge in the state machine: when the current state equals
 * @p from_state and the incoming event equals @p event, the machine moves to
 * @p to_state and (if set) fires @p on_transition.
 */
typedef struct {
    int from_state;                /**< Source state that triggers this rule */
    int event;                     /**< Event that triggers this rule */
    int to_state;                  /**< Target state after the transition */
    zenit_state_callback_t on_transition; /**< Optional callback (may be NULL) */
} zenit_state_transition_t;

/** @brief Opaque handle for a finite-state machine instance. */
typedef struct zenit_state_t zenit_state_t;

/**
 * @brief Allocate and initialise a state machine.
 *
 * The caller must keep @p table alive for the lifetime of the returned state
 * machine. The table is not copied internally.
 *
 * @param table         Pointer to an array of transition rules.
 * @param count         Number of elements in @p table.
 * @param initial_state State the machine starts in.
 * @return Opaque handle, or NULL on allocation failure.
 */
zenit_state_t *zenit_state_allocate(
    const zenit_state_transition_t *table,
    size_t count,
    int initial_state
);

/**
 * @brief Allocate and initialise a state machine with a custom allocator.
 *
 * The caller must keep @p table alive for the lifetime of the returned state
 * machine. The table is not copied internally.
 *
 * @param table         Pointer to an array of transition rules.
 * @param count         Number of elements in @p table.
 * @param initial_state State the machine starts in.
 * @param allocator     Custom allocator to use for all internal memory.
 * @return Opaque handle, or NULL on allocation failure.
 */
zenit_state_t *zenit_state_allocate_with_allocator(
    const zenit_state_transition_t *table,
    size_t count,
    int initial_state,
    zenit_allocator_t allocator
);

/**
 * @brief Feed an event into the state machine.
 *
 * Walks the transition table linearly looking for a rule where
 * @c from_state matches the current state and @c event matches the event.
 * On match the machine transitions and (if present) fires the callback.
 *
 * @param state   Opaque handle obtained from zenit_state_allocate().
 * @param event   Event to process.
 * @param context Opaque user pointer forwarded to the transition callback.
 * @return ZENIT_RESULT_OK on transition, or ZENIT_RESULT_ERROR(ZENIT_ERROR_NOT_FOUND)
 *         if no matching rule exists.
 */
zenit_result_t zenit_state_process_event(zenit_state_t *state, int event, void *context);

/**
 * @brief Read the current state without modifying the machine.
 *
 * @param state Opaque handle.
 * @return The current internal state value.
 */
int zenit_state_current(const zenit_state_t *state);

/**
 * @brief Reset the state machine to a given state without processing an event.
 *
 * Useful for returning to a known state without going through the transition
 * table (e.g. after error recovery).
 *
 * @param state State machine handle.
 * @param new_state State value to set as the current state.
 * @return ZENIT_RESULT_OK on success, or ZENIT_RESULT_ERROR(ZENIT_ERROR_NULL)
 *         if @p state is NULL.
 */
zenit_result_t zenit_state_reset(zenit_state_t *state, int new_state);

/**
 * @brief Release all memory owned by a state machine.
 *
 * Does NOT free the transition table — that is the caller's responsibility.
 * Passing NULL is safe and is a no-op.
 *
 * @param state Opaque handle, or NULL.
 */
void zenit_state_deallocate(zenit_state_t *state);

#endif
