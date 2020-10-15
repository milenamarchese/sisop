// implement fork from user space

#include <inc/string.h>
#include <inc/lib.h>

// PTE_COW marks copy-on-write page table entries.
// It is one of the bits explicitly allocated to user processes (PTE_AVAIL).
#define PTE_COW 0x800

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
	uint32_t pte = uvpt[PGNUM(addr)];
	if (!(err & FEC_PR))
		panic("error page not mapped!");
	if (!(err & FEC_WR))
		panic("error on read!");
	if (!(pte & PTE_COW))
		panic("error not cow!");
	// Allocate a new page, map it at a temporary location (PFTEMP),
	// copy the data from the old page to the new page, then move the new
	// page to the old page's address.
	// Hint:
	//   You should make three system calls.
	addr = ROUNDDOWN(addr, PGSIZE);
	if (sys_page_alloc(0, PFTEMP, PTE_W|PTE_U|PTE_P) < 0)
		panic("sys_page_alloc");
	memcpy(PFTEMP, addr, PGSIZE);
	if (sys_page_map(0, PFTEMP, 0, addr, PTE_W|PTE_U|PTE_P) < 0)
		panic("sys_page_map");
	if (sys_page_unmap(0, PFTEMP) < 0)
		panic("sys_page_unmap");
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
	uint32_t pte = uvpt[pn];
	void* addr = (void*)(pn * PGSIZE); // VA

	if ((pte & PTE_W) || (pte & PTE_COW)) {
		if ((r = sys_page_map(0, addr, envid, addr, PTE_P|PTE_U|PTE_COW)) < 0)
			panic("sys_page_map: %e", r);
		if ((r = sys_page_map(0, addr, 0, addr, PTE_P|PTE_U|PTE_COW)) < 0)
			panic("sys_page_map: %e", r);
	} else {
		if ((r = sys_page_map(0, addr, envid, addr, PTE_P|PTE_U)) < 0)
			panic("sys_page_map: %e", r);
	}

	return 0;
}

//
static void
dup_or_share(envid_t dstenv, void *va, int perm) {
	int r;

	if ((r = sys_page_alloc(dstenv, va, PTE_P|PTE_U|PTE_W)) < 0)
		panic("sys_page_alloc: %e", r);

	if (!(perm & PTE_W)) { // Read only
		if ((r = sys_page_map(dstenv, va, 0, UTEMP, PTE_P|PTE_U)) < 0)
			panic("sys_page_map: %e", r);
	} else { // R&W
		if ((r = sys_page_map(dstenv, va, 0, UTEMP, PTE_P|PTE_U|PTE_W)) < 0)
			panic("sys_page_map: %e", r);
		memmove(UTEMP, va, PGSIZE);
		if ((r = sys_page_unmap(0, UTEMP)) < 0)
			panic("sys_page_unmap: %e", r);
	}
}

//
static envid_t 
fork_v0(void) {
	envid_t envid;
	uint8_t *addr;
	int r;
	envid = sys_exofork();
	if (envid < 0)
		panic("sys_exofork: %e", envid);
	if (envid == 0) {
		// We're the child.
		thisenv = &envs[ENVX(sys_getenvid())];
		return 0;
	}

	for (addr = (uint8_t*) 0; addr < (uint8_t*)UTOP; addr += PGSIZE) {
		if ((uvpd[PDX(addr)] & PTE_P) && (uvpt[PGNUM(addr)] & PTE_P)) { // Check if page exists, then if page is present
			pte_t pte = uvpt[PGNUM(addr)];
			dup_or_share(envid, addr, pte & PTE_SYSCALL);	
		}			
	}

	if ((r = sys_env_set_status(envid, ENV_RUNNABLE)) < 0)
		panic("sys_env_set_status: %e", r);

	return envid;
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

    envid_t child_id = sys_exofork();

	if (child_id < 0)
		panic("fork: %e", child_id);
    if (child_id == 0) {
		thisenv = &envs[ENVX(sys_getenvid())];
		return 0;
    }

    extern void _pgfault_upcall();
	sys_env_set_pgfault_upcall(child_id, _pgfault_upcall);
	sys_page_alloc(child_id, (void *)(UXSTACKTOP - PGSIZE), PTE_W|PTE_U|PTE_P);

    size_t pdx, ptx;
    for (pdx = 0; pdx < PDX(UTOP); ++pdx) {
        if (!(uvpd[pdx] & PTE_P))
            continue;
        for (ptx = 0; ptx < NPTENTRIES; ++ptx) {
            uintptr_t addr = (uintptr_t)PGADDR(pdx, ptx, 0);
            uintptr_t pte_num = PGNUM(addr);
            if (addr == (UXSTACKTOP - PGSIZE) || !(uvpt[pte_num] & PTE_P))
                continue;
            duppage(child_id, pte_num);
        }
    }

	sys_env_set_status(child_id, ENV_RUNNABLE);
	return child_id;
}

// Challenge!
int
sfork(void)
{
	panic("sfork not implemented");
	return -E_INVAL;
}


