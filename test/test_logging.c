#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdarg.h>

#include "util.h"
#include "log.h"

int main(void)
{
	log_print("testing info message\n");
	log_warn("testing warning message\n");
	log_error("testing warning message\n");
	log_errorwloc("testing errorwloc macro\n");
	log_warn("warn with errno:");
	log_fatal("testing fatal message\n");
}
