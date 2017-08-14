#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include <stdint.h>
#include "trampoline.h"

#if defined(__x86_64__) || defined(_M_X64)
typedef uint64_t pint;
typedef int64_t sint;
#define WORST_CASE 			40
#define JUMP_SIZE 			sizeof(JMP_ABS)
#elif defined(__i386) || defined(_M_IX86)
typedef uint32_t pint;
typedef int32_t sint;
#define WORST_CASE 			29
#define JUMP_SIZE 			sizeof(JMP_REL)
#endif

#define TRMPS_ARRAY_SIZE	30
const uint8_t NOP = 0x90;

static void* trmps;
static int last_trmp = 0; // trmp[TRMPS_ARRAY_SIZE]

static void initializeTrampolines(void) {
	trmps = mmap(NULL, (WORST_CASE * TRMPS_ARRAY_SIZE),
		        PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
}

int Hook(void* target, void* replacement, void** func_ptr) {
    TRAMPOLINE ct;
    int res, page_size;

    // Check if our trampoline pool has been initialized. If not, do so.
    if (!trmps) {
    	initializeTrampolines();
    }
    else { // TODO: Implement a way to add and remove hooks.
    	if (last_trmp + 1 > TRMPS_ARRAY_SIZE) return -3;
    }

    void* trmp = (void*)((pint)trmps + last_trmp * WORST_CASE);

    ct.pTarget     = target;
    ct.pDetour     = replacement;
    ct.pTrampoline = trmp;

    if (!CreateTrampolineFunction(&ct)) {
        return -11;
    }

    page_size = sysconf(_SC_PAGESIZE);
    if (page_size == -1) return errno;
    res = mprotect((void*)((pint)target & ~(page_size-1)), page_size, PROT_READ | PROT_WRITE | PROT_EXEC);
    if (res) return errno;

#if defined(__x86_64__) || defined(_M_X64)
    PJMP_ABS pJmp = (PJMP_ABS)target;
    pJmp->opcode0 = 0xFF;
    pJmp->opcode1 = 0x25;
    pJmp->dummy   = 0;
    pJmp->address = (pint)replacement;
#else
    PJMP_REL pJmp = (PJMP_REL)target;
    pJmp->opcode  = 0xE9;
    pJmp->operand = (pint)replacement - ( (pint)target + sizeof(JMP_REL) );
#endif

    int difference = ct.newIPs[ ct.nIP - 1 ];
    for (int i=JUMP_SIZE; i<difference; i++) {
        *((uint8_t*)target + i) = NOP;
    }

    *func_ptr = trmp;

    last_trmp++;
    return 0;
}

int seek_hook_slot( int offset ) {

    if ( (last_trmp + offset < 0) || (last_trmp + offset >= TRMPS_ARRAY_SIZE) )
        return 0;

    last_trmp += offset;
    return 1;
}
