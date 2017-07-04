/*
 * linux/arch/arm/mach-mmp/include/mach/timex.h
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

<<<<<<< HEAD
#if defined(CONFIG_CPU_MMP2) || defined(CONFIG_CPU_MMP3)
=======
#ifdef CONFIG_CPU_MMP2
>>>>>>> v3.4.6
#define CLOCK_TICK_RATE		6500000
#else
#define CLOCK_TICK_RATE		3250000
#endif
