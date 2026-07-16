#include <libzenit/state.h>
#include <stdio.h>
#include <stdlib.h>

void *__real_malloc(size_t size);
void *__wrap_malloc(size_t size) {
    (void)size;
    return NULL;
}

int main(void) {
    zenit_state_transition_t t = {0, 0, 0, NULL};
    zenit_state_t *state = zenit_state_allocate(&t, 1, 0);
    if (state == NULL) {
        printf("PASS: malloc failure returns NULL\n");
        return 0;
    }
    printf("FAIL: expected NULL\n");
    return 1;
}
