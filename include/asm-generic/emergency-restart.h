#ifndef _ASM_GENERIC_EMERGENCY_RESTART_H
#define _ASM_GENERIC_EMERGENCY_RESTART_H

static inline void machine_emergency_restart(void)
{
<<<<<<< HEAD
	machine_restart("panic");
=======
	machine_restart(NULL);
>>>>>>> v3.4.6
}

#endif /* _ASM_GENERIC_EMERGENCY_RESTART_H */
