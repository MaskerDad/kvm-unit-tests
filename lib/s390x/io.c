/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * s390x io implementation
 *
 * Copyright (c) 2017 Red Hat Inc
 *
 * Authors:
 *  Thomas Huth <thuth@redhat.com>
 *  David Hildenbrand <david@redhat.com>
 */
#include <libcflat.h>
#include <argv.h>
#include <vmalloc.h>
#include <asm/spinlock.h>
#include <asm/facility.h>
#include <asm/sigp.h>
#include "sclp.h"
#include "uv.h"
#include "smp.h"

extern char ipl_args[];
uint64_t stfl_doublewords[NB_STFL_DOUBLEWORDS];

static struct spinlock lock;

void setup(void);

void puts(const char *s)
{
	spin_lock(&lock);
	sclp_print(s);
	spin_unlock(&lock);
}

void setup(void)
{
	struct cpu this_cpu_tmp = { 0 };

	/*
	 * Set a temporary empty struct cpu for the boot CPU, needed for
	 * correct interrupt handling in the setup process.
	 * smp_setup will allocate and set the permanent one.
	 */
	THIS_CPU = &this_cpu_tmp;

	setup_args_progname(ipl_args);
	setup_facilities();
	sclp_read_info();
	sclp_facilities_setup();
	sclp_console_setup();
	sclp_memory_setup();
	uv_setup();
	smp_setup();
}

void exit(int code)
{
	smp_teardown();
	printf("\nEXIT: STATUS=%d\n", ((code) << 1) | 1);
	while (1) {
		sigp(stap(), SIGP_STOP, 0, NULL);
	}
}
