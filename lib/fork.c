// implement fork from user space

#include <inc/string.h>
#include <inc/lib.h>

// PTE_COW marks copy-on-write page table entries.
// It is one of the bits explicitly allocated to user processes (PTE_AVAIL).
#define PTE_COW		0x800

extern void _pgfault_upcall(void);
//
// Custom page fault handler - if faulting page is copy-on-write,
// map in our own private writable copy.
//
static void
pgfault(struct UTrapframe *utf)
{
	void *addr = (void *) utf->utf_fault_va;
	uint32_t err = utf->utf_err;
	int r;

	//const volatile struct Env *thisenv = &envs[ENVX(sys_getenvid())];
    envid_t env_id = sys_getenvid();
	if ((err & FEC_WR) == 0 ){
        panic("[%08x]fault_va was not a write %08x ,eip:%08x",env_id,addr,utf->utf_eip);
	}
	// Check that the faulting access was (1) a write, and (2) to a
	// copy-on-write page.  If not, panic.
	// Hint:
	//   Use the read-only page table mappings at uvpt
	//   (see <inc/memlayout.h>).

	// LAB 4: Your code here.
    if((uvpt[PGNUM(addr)] & (PTE_COW)) == 0)
        panic("fault_va is not a copy on write page %08x ,eip:%08x",addr,utf->utf_eip);

	// Allocate a new page, map it at a temporary location (PFTEMP),
	// copy the data from the old page to the new page, then move the new
	// page to the old page's address.
	// Hint:
	//   You should make three system calls.
    //cprintf("[%08x] line:%d pgfault va:%08x\n", thisenv->env_id,__LINE__,addr);

	// LAB 4: Your code here.
	if ((r = sys_page_alloc(env_id, PFTEMP, PTE_P|PTE_U|PTE_W)) < 0)
		panic("sys_page_alloc: %e", r);
	memcpy(PFTEMP, ROUNDDOWN(addr,PGSIZE), PGSIZE);
	if ((r = sys_page_map(env_id, PFTEMP, env_id,ROUNDDOWN(addr,PGSIZE), PTE_P|PTE_U|PTE_W)) < 0)
		panic("sys_page_map: %e", r);
	if ((r = sys_page_unmap(env_id, PFTEMP)) < 0)
		panic("sys_page_unmap: %e", r);
    
	//panic("pgfault not implemented");
}

//
// Map our virtual page pn (address pn*PGSIZE) into the target envid
// at the same virtual address.  If the page is writable or copy-on-write,
// the new mapping must be created copy-on-write, and then our mapping must be
// marked copy-on-write as well.  (Exercise: Why do we need to mark ours
// copy-on-write again if it was already copy-on-write at the beginning of
// this function?)
// anwser: it can be cleared by another child before we mark child's copy-on-write

//
// Returns: 0 on success, < 0 on error.
// It is also OK to panic on error.
//
static int
duppage(envid_t envid, unsigned pn)
{
	int r;

	// LAB 4: Your code here.
    int perm = PTE_P|PTE_U;
    // pde_t not present
    if (uvpt[pn] & PTE_W)
        perm |= PTE_COW;
    else if (uvpt[pn] & PTE_COW)
        perm |= PTE_COW;
	if ((r = sys_page_map(thisenv->env_id, (void *)(pn*PGSIZE), envid, (void *)(pn*PGSIZE), perm)) < 0)
		panic("sys_page_map: %e", r);
	if ((r = sys_page_map(envid, (void *)(pn*PGSIZE),thisenv->env_id, (void *)(pn*PGSIZE), perm)) < 0)
		panic("sys_page_map: %e", r);
	//panic("duppage not implemented");
	return 0;
}

//
// User-level fork with copy-on-write.
// Set up our page fault handler appropriately.
// Create a child.
// Copy our address space and page fault handler setup to the child.
// Then mark the child as runnable and return.
//
// Returns: child's envid to the parent, 0 to the child, < 0 on error.
// It is also OK to panic on error.
//
// Hint:
//   Use uvpd, uvpt, and duppage.
//   Remember to fix "thisenv" in the child process.
//   Neither user exception stack should ever be marked copy-on-write,
//   so you must allocate a new page for the child's user exception stack.
//
envid_t
fork(void)
{
    set_pgfault_handler(pgfault);
	envid_t envid;
	int r;
	uint8_t *addr;
	envid = sys_exofork();
	if (envid < 0)
		panic("sys_exofork: %e", envid);
	else if (envid == 0) {
		// We're the child.
		// The copied value of the global variable 'thisenv'
		// is no longer valid (it refers to the parent!).
		// Fix it and return 0.
		thisenv = &envs[ENVX(sys_getenvid())];
	        cprintf("[%08x] end reset thisenv\n", thisenv->env_id);
		//set_pgfault_handler(pgfault);
		return 0;
	}
	//set child's pgfault handler
	if ((r = sys_page_alloc(envid, (void*) (UXSTACKTOP - PGSIZE), PTE_P|PTE_U|PTE_W)) < 0)
		panic("sys_page_alloc: %e", r);
	if ((r = sys_env_set_pgfault_upcall(envid, (void*) _pgfault_upcall)) < 0)
		panic("sys_env_set_pgfault_upcall: %e", r);

	// We're the parent.
	for (addr =  0; addr < (uint8_t*)USTACKTOP; addr += PGSIZE){
	//for (;pn < PGNUM(MMIOLIM); pn += 4){
        //only copy present page
        if ((uvpd[PDX(addr)] & PTE_P) == 0 || (uvpt[PGNUM(addr)] & PTE_P) == 0)
            continue;
		duppage(envid, PGNUM(addr));
    }
	// LAB 4: Your code here.
	// Start the child environment running
	if ((r = sys_env_set_status(envid, ENV_RUNNABLE)) < 0)
		panic("sys_env_set_status: %e", r);
	return envid;
	//panic("fork not implemented");
}

// Challenge!
int
sfork(void)
{
	panic("sfork not implemented");
	return -E_INVAL;
}
