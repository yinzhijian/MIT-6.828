
#include <inc/lib.h>

void
exit(void)
{
    cprintf("[%08x] start exit\n", thisenv->env_id);
	sys_env_destroy(0);
    cprintf("[%08x] end exit\n", thisenv->env_id);
}

