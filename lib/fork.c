// implement fork from user space

#include <inc/string.h>
#include <inc/lib.h>

// PTE_COW marks copy-on-write page table entries.
// It is one of the bits explicitly allocated to user processes (PTE_AVAIL).
#define PTE_COW		0x800

extern void (*_pgfault_handler)(struct UTrapframe *utf);
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

	// Check that the faulting access was (1) a write, and (2) to a
	// copy-on-write page.  If not, panic.
	// Hint:
	//   Use the read-only page table mappings at uvpt
	//   (see <inc/memlayout.h>).

	// LAB 4: Your code here.
    if((uvpt[PGNUM(addr)] & (PTE_W|PTE_COW)) <= 0)
        panic("fault_va was not a write and copy on write page %08x ,eip:%08x",addr,utf->utf_eip);

	// Allocate a new page, map it at a temporary location (PFTEMP),
	// copy the data from the old page to the new page, then move the new
	// page to the old page's address.
	// Hint:
	//   You should make three system calls.
    cprintf("[%08x] line:%d pgfault va:%08x\n", thisenv->env_id,__LINE__,addr);

	// LAB 4: Your code here.
	if ((r = sys_page_alloc(thisenv->env_id, PFTEMP, PTE_P|PTE_U|PTE_W)) < 0)
		panic("sys_page_alloc: %e", r);
	memmove(PFTEMP, ROUNDDOWN(addr,PGSIZE), PGSIZE);
	if ((r = sys_page_map(thisenv->env_id, ROUNDDOWN(addr,PGSIZE), thisenv->env_id,PFTEMP , PTE_P|PTE_U|PTE_W)) < 0)
		panic("sys_page_map: %e", r);
	if ((r = sys_page_unmap(thisenv->env_id, PFTEMP)) < 0)
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
	envid_t envid;
    int r;
	uint8_t *addr;
    set_pgfault_handler(pgfault);
	envid = sys_exofork();
	if (envid < 0)
		panic("sys_exofork: %e", envid);
	if (envid == 0) {
		// We're the child.
		// The copied value of the global variable 'thisenv'
		// is no longer valid (it refers to the parent!).
		// Fix it and return 0.
        _pgfault_handler = 0;
		thisenv = &envs[ENVX(sys_getenvid())];
        set_pgfault_handler(pgfault);
		return 0;
	}

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
