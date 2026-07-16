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

#include <libzenit/benchmark.h>
#include <libzenit/state.h>
#include <stdlib.h>

/* ---------------------------------------------------------------------------
 * Context for the cyclic sequential-transition benchmark.
 *
 * Builds a ring of N states: state i transitions to (i+1)%N on event i.
 * Processing events 0..N-1 in order walks the full cycle and returns to
 * the initial state.
 * -------------------------------------------------------------------------*/

typedef struct {
    zenit_state_t* state;
    zenit_state_transition_t* table; /* owned — freed in teardown */
    int* events;
    int num_events;
} cycle_ctx_t;

/* Create a cyclical state machine and its event sequence. */
static cycle_ctx_t* cycle_setup(int num_states) {
    cycle_ctx_t* ctx = malloc(sizeof(cycle_ctx_t));
    if (ctx == NULL) return NULL;

    /* Build a cyclic transition table */
    ctx->table = malloc(sizeof(zenit_state_transition_t) * (size_t)num_states);
    if (ctx->table == NULL) {
        free(ctx);
        return NULL;
    }

    for (int i = 0; i < num_states; i++) {
        ctx->table[i] = (zenit_state_transition_t){
            .from_state = i,
            .event = i,
            .to_state = (i + 1) % num_states,
            .on_transition = NULL
        };
    }

    /* Allocate the state machine. The table pointer is stashed inside;
     * the caller must keep the table alive until after deallocation. */
    ctx->state = zenit_state_allocate(ctx->table, (size_t)num_states, 0);
    if (ctx->state == NULL) {
        free(ctx->table);
        free(ctx);
        return NULL;
    }

    /* Pre-build event sequence for one full cycle */
    ctx->events = malloc(sizeof(int) * (size_t)num_states);
    if (ctx->events == NULL) {
        zenit_state_deallocate(ctx->state);
        free(ctx->table);
        free(ctx);
        return NULL;
    }

    for (int i = 0; i < num_states; i++) {
        ctx->events[i] = i;
    }

    ctx->num_events = num_states;
    return ctx;
}

static void cycle_teardown(cycle_ctx_t* ctx) {
    if (ctx == NULL) return;
    zenit_state_deallocate(ctx->state);
    free(ctx->table);
    free(ctx->events);
    free(ctx);
}

/* One call = one full cycle through all transitions */
static void bench_cycle_fn(void* ctx) {
    cycle_ctx_t* c = (cycle_ctx_t*)ctx;
    for (int i = 0; i < c->num_events; i++) {
        zenit_state_process_event(c->state, c->events[i], NULL);
    }
}

/* ---------------------------------------------------------------------------
 * Miss benchmark: send an invalid event that never matches.
 * The state never changes, so every call is identical.
 * -------------------------------------------------------------------------*/

typedef struct {
    zenit_state_t* state;
    zenit_state_transition_t* table; /* owned */
} miss_ctx_t;

static miss_ctx_t* miss_setup(void) {
    miss_ctx_t* ctx = malloc(sizeof(miss_ctx_t));
    if (ctx == NULL) return NULL;

    /* Single transition: state 0 on event 0 stays in state 0 */
    ctx->table = malloc(sizeof(zenit_state_transition_t));
    if (ctx->table == NULL) {
        free(ctx);
        return NULL;
    }
    ctx->table[0] = (zenit_state_transition_t){ 0, 0, 0, NULL };

    ctx->state = zenit_state_allocate(ctx->table, 1, 0);
    if (ctx->state == NULL) {
        free(ctx->table);
        free(ctx);
        return NULL;
    }

    return ctx;
}

static void miss_teardown(miss_ctx_t* ctx) {
    if (ctx == NULL) return;
    zenit_state_deallocate(ctx->state);
    free(ctx->table);
    free(ctx);
}

static void bench_miss_fn(void* ctx) {
    zenit_state_process_event(((miss_ctx_t*)ctx)->state, -999, NULL);
}

/* ---------------------------------------------------------------------------
 * Main — run all state-machine benchmarks.
 * -------------------------------------------------------------------------*/

int main(void) {
    /* Sequential: small table (8 states → 8 transitions per call) */
    {
        cycle_ctx_t* ctx = cycle_setup(8);
        if (ctx != NULL) {
            /* 1M iterations × 8 transitions = 8M process_event calls */
            zenit_bench_result_t r = zenit_bench_run(
                "state_seq_8", bench_cycle_fn, ctx, 1000000
            );
            zenit_bench_print(&r);
            cycle_teardown(ctx);
        }
    }

    /* Sequential: large table (1024 states → 1024 transitions per call) */
    {
        cycle_ctx_t* ctx = cycle_setup(1024);
        if (ctx != NULL) {
            zenit_bench_result_t r = zenit_bench_run(
                "state_seq_1024", bench_cycle_fn, ctx, 10000
            );
            zenit_bench_print(&r);
            cycle_teardown(ctx);
        }
    }

    /* Misses: always-invalid event, one process_event per call */
    {
        miss_ctx_t* ctx = miss_setup();
        if (ctx != NULL) {
            zenit_bench_result_t r = zenit_bench_run(
                "state_miss", bench_miss_fn, ctx, 10000000
            );
            zenit_bench_print(&r);
            miss_teardown(ctx);
        }
    }

    return 0;
}
