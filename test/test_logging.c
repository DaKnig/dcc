#include "log.h"
#include "util.h"

#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    log_print("testing info message\n");
    log_warn("testing warning message\n");
    log_error("testing warning message\n");
    log_errorwloc("testing errorwloc macro\n");
    log_warn("warn with errno:");
    return 0;
}
