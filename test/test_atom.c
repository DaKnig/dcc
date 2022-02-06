#include "atom.h"
#include "log.h"
#include "util.h"

#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void testcase1(void) {
    const char *strtab[] = {
        "deadbeef",  "hello world", "assdffdasf", "123422111",
        "abcdefghi", "foo",         "bar",
    };
    const char *tmp[COUNT_OF(strtab)] = {NULL};

    const size_t n = COUNT_OF(strtab);
    const size_t niter = 4 * n;

    for (size_t i = 0; i < niter; i++) {
        const char *atom = atom_fromstr(strtab[i % n]);
        if (i >= n) {
            log_debug("atom = '%s', strtab[i] = '%s'\n", atom, strtab[i % n]);
            assert(atom == tmp[i % n]);
        } else {
            assert(tmp[i] == NULL);
            tmp[i] = atom;
        }
    }
}

int main(void) {
    testcase1();
    return 0;
}
