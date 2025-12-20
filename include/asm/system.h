#define move_to_user_mode() \
__asm__ ("movl %%esp,%%eax\n\t" \
	"pushl $0x17\n\t" \
	"pushl %%eax\n\t" \
	"pushfl\n\t" \
	"pushl $0x0f\n\t" \
	"pushl $1f\n\t" \
	"iret\n" \
	"1:\tmovl $0x17,%%eax\n\t" \
	"movw %%ax,%%ds\n\t" \
	"movw %%ax,%%es\n\t" \
	"movw %%ax,%%fs\n\t" \
	"movw %%ax,%%gs" \
	:::"ax")

#define sti() __asm__ ("sti"::)
#define cli() __asm__ ("cli"::)
#define nop() __asm__ ("nop"::)

#define iret() __asm__ ("iret"::)

#define _set_gate(gate_addr,type,dpl,addr) \
__asm__ ("movw %%dx,%%ax\n\t" \
	"movw %0,%%dx\n\t" \
	"movl %%eax,%1\n\t" \
	"movl %%edx,%2" \
	: \
	: "i" ((short) (0x8000+(dpl<<13)+(type<<8))), \
	"o" (*((char *) (gate_addr))), \
	"o" (*(4+(char *) (gate_addr))), \
	"d" ((char *) (addr)),"a" (0x00080000))

#define set_intr_gate(n,addr) \
	_set_gate(&idt[n],14,0,addr)

#define set_trap_gate(n,addr) \
	_set_gate(&idt[n],15,0,addr)

#define set_system_gate(n,addr) \
	_set_gate(&idt[n],15,3,addr)

#define _set_seg_desc(gate_addr,type,dpl,base,limit) {\
	*(gate_addr) = ((base) & 0xff000000) | \
		(((base) & 0x00ff0000)>>16) | \
		((limit) & 0xf0000) | \
		((dpl)<<13) | \
		(0x00408000) | \
		((type)<<8); \
	*((gate_addr)+1) = (((base) & 0x0000ffff)<<16) | \
		((limit) & 0x0ffff); }

/* GCC 15+ friendly: build TSS/LDT descriptor in C (8 bytes). */
static inline void _set_tssldt_desc(char *n, unsigned long addr, unsigned char type)
{
    /* limit = 104 bytes */
    unsigned short limit = 104;

    n[0] = (unsigned char)(limit & 0xff);
    n[1] = (unsigned char)((limit >> 8) & 0xff);

    n[2] = (unsigned char)(addr & 0xff);
    n[3] = (unsigned char)((addr >> 8) & 0xff);
    n[4] = (unsigned char)((addr >> 16) & 0xff);

    n[5] = type;  /* 0x89 for TSS, 0x82 for LDT */
    n[6] = 0x00;
    n[7] = (unsigned char)((addr >> 24) & 0xff);
}

#define set_tss_desc(n, addr) _set_tssldt_desc((char *)(n), (unsigned long)(addr), 0x89)
#define set_ldt_desc(n, addr) _set_tssldt_desc((char *)(n), (unsigned long)(addr), 0x82)

