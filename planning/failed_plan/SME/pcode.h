


/*
    PCODE
    
    A hack to get "assembly" working but cross platform.
    
    Your Saved metaprogramming environment won't be portable, but, well,
    it doesn't actually need to be. You can do a full rebuild from
    source when you move platforms.
    
    It works by using real pointers on real memory and keeping some 64 bit
    registers.
*/


/*
    PCODE ast... sort of.
*/
typedef struct {
    char* name;
    u64 var_value; //the uint that this variable's value is.
    //arguments
    char* arg1;
    char* arg2;
    char* arg3;
    //values of the arguments.
    u64 arg1_val;
    u64 arg2_val;
    u64 arg3_val;
    unsigned char is_insn;
    unsigned char is_label;
    unsigned char is_var;
    unsigned char insn_id;
} pcode_insn;

typedef struct{
    char* name;
    u64 val;
} pcode_var;

typedef struct {
    char* name;
    sme_ast_mod_root* linked_ast;
    sme_module* out_mod;
    pcode_insn* insns;
    pcode_var* vars;
    size_t ninsns;
    size_t nvars;
} pcode_build;


//is sixteen megs enough?
#define PCODE_STACK_SZ 0x1000000

#define EAT_BYTE() mread_u8(pc++)
#define EAT_SHORT() mread_u16((pc=pc+2)-2)
#define EAT_LONG() mread_u32((pc=pc+4)-4)
#define EAT_QWORD() mread_u64((pc=pc+8)-8)


static inline u8 mread_u8(u8* addr){return addr[0];}
static inline void mwrite_u8(u8* addr, u8 val){
    addr[0] = val;
}


//multi-byte reads and writes
static inline u16 mread_u16(u8* addr){
	u16 rv;
	memcpy(&rv, addr, 2);
	return rv;
}

static inline void mwrite_u16(u8* addr, u16 val){
	memcpy(addr, &val, 2);
}

static inline u32 mread_u32(u8* addr){
	u32 rv;
	memcpy(&rv,(addr), 4);
	return (rv);
}

static inline void mwrite_u32(u8* addr, u32 val){
	memcpy((addr), &val, 4);
}

static inline u64 mread_u64(u8* addr){
	u64 rv;
	memcpy(&rv, (addr), 8);
	return (rv);
}

static inline void mwrite_u64(u8* addr, u64 val){
	memcpy((addr), &val, 8);
}




#define READ8(addr) mread_u8((u8*)(addr))
#define READ16(addr) mread_u16((u8*)(addr))
#define READ32(addr) mread_u32((u8*)(addr))
#define READ64(addr) mread_u64((u8*)(addr))

#define WRITE8(addr,val)  mwrite_u8(((u8*)(addr)), val)
#define WRITE16(addr,val) mwrite_u16(((u8*)(addr)), val)
#define WRITE32(addr,val) mwrite_u32(((u8*)(addr)), val)
#define WRITE64(addr,val) mwrite_u64(((u8*)(addr)), val)


u8* pcode_stack = 0;
u8* pcode_stackptr;

sme_module* get_mod_by_id(u64 id){
    if(nmods >= id) return 0;
    return mods + id;
}

char* opcodes[98] = {
    "hlt",
    "nop",
    "mov",
    "syscall",
    "ld8",
    "ld16",
    "ld32",
    "ld64",
    "st8",
    "st16",
    "st32",
    "st64",
    "ild8",
    "ild16",
    "ild32",
    "ild64",
    "ist8",
    "ist16",
    "ist32",
    "ist64",
    "lsh",
    "rsh",
    "and",
    "or",
    "xor",
    "compl",
    "neg",
    "bool",
    "not",
    
    "iadd",
    "isub",
    "imul",
    "fadd",
    "fsub",
    "fmul",
    
    "dadd",
    "dsub",
    "dmul",
    "idiv",
    "imod",
    "udiv",
    "umod",
    "fdiv",
    "ddiv",
    "ze32",
    "ze16",
    "ze8",    
    
    "se32",
    "se16",
    "se8",
    "ucmp",
    "icmp",
    "fcmp",
    "dcmp",
    "jmp",
    "jnz",
    "jiz",
    "jlt",
    "jgt",
    "call",
    "ret",
    "getstp",
    "setstp",
    "push64",
    "push32",
    "push16",
    "push8",
    "pop64",
    "pop32",
    "pop16",
    "pop8",
    "itod",
    "itof",
    "dtoi",
    "ftoi",
    "dtof",
    "ftod",
    "inc",
    "dec",
    "ldsp64",
    "ldsp32",
    "ldsp16",
    "ldsp8",
    "stsp64",
    "stsp32",
    "stsp16",
    "stsp8",
    "bswap64",
    "bswap32",
    "bswap16",
    "mnz",
    "imm64",
    "imm32",
    "imm16",
    "imm8",
    //needed exts
    "callfnptr",
    "getglobalptr",
    "getglobalptrptr",
};

typedef struct {
    char* name;
    char is_label;
    u64 arg1;
    u64 arg2;
    u64 arg3;
} pcode_ins;



int pcode(
    char* callsite,
    sme_module* m
){
    u64 gpregs[256];
    char* pc;
    pc = callsite;
    u8* stp;
    sme_module* active_mod;
    stp = pcode_stack;
    #define DISPATCH {pc++;goto *opcodes[(pc-1)[0]];}
    void* opcodes[256] = {
        &&P_HLT,&&P_NOP,&&P_MOV,&&P_SYSCALL,
        &&P_LD8,&&P_LD16,&&P_LD32,&&P_LD64, 
        &&P_ST8,&&P_ST16,&&P_ST32,&&P_ST64,
        &&P_ILD8,&&P_ILD16,&&P_ILD32,&&P_ILD64, 
        
        &&P_IST8,&&P_IST16,&&P_IST32,&&P_IST64,
        &&P_LSH,&&P_RSH,&&P_AND,&&P_OR, 
        &&P_XOR,&&P_COMPL,&&P_NEG,&&P_BOOL,
        &&P_NOT,&&P_IADD,&&P_ISUB,&&P_IMUL, 
        
        &&P_FADD,&&P_FSUB,&&P_FMUL,&&P_DADD,
        &&P_DSUB,&&P_DMUL,&&P_IDIV,&&P_IMOD, 
        &&P_UDIV,&&P_UMOD,&&P_FDIV,&&P_DDIV,
        &&P_ZE32,&&P_ZE16,&&P_ZE8,&&P_SE32, 
        
        &&P_SE16,&&P_SE8,&&P_UCMP,&&P_ICMP,
        &&P_FCMP,&&P_DCMP,&&P_JMP,&&P_JNZ, 
        &&P_JIZ,&&P_JLT,&&P_JGT,&&P_CALL,
        &&P_RET,&&P_GETSTP,&&P_SETSTP,&&P_PUSH64, 
        
        &&P_PUSH32,&&P_PUSH16,&&P_PUSH8,&&P_POP64,
        &&P_POP32,&&P_POP16,&&P_POP8,&&P_ITOD, 
        &&P_ITOF,&&P_DTOI,&&P_FTOI,&&P_DTOF,
        &&P_FTOD,&&P_INC,&&P_DEC,&&P_LDSP64, 
        
        &&P_LDSP32,&&P_LDSP16,&&P_LDSP8,&&P_STSP64,
        &&P_STSP32,&&P_STSP16,&&P_STSP8,&&P_BSWAP64, 
        &&P_BSWAP32,&&P_BSWAP16,&&P_MNZ,&&P_LITERAL64,
        &&P_LITERAL32,&&P_LITERAL16,&&P_LITERAL8,
        &&P_CALLFNPTR, 
        &&P_GETGLOBALPTR,
        &&P_GETGLOBALPTRPTR,
        &&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT, 
        &&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT, 
        &&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT, 
        &&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT, 
        &&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT, 
        &&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT, 
        &&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT, 
        &&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT, 
        &&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT, 
        &&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT, 
        &&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT, 
        &&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT, 
        &&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT, 
        &&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT, 
        &&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT, 
        &&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT, 
        &&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT, 
        &&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT, 
        &&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT, 
        &&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT,&&P_HLT, 
    };
    P_NOP:
        DISPATCH
    ;
    P_HLT:return 0;
    P_MOV:
	{
		uint8_t regid_src;
		uint8_t regid_dest;
		regid_dest = EAT_BYTE();
		regid_src = EAT_BYTE();
		gpregs[regid_dest] = gpregs[regid_src];
	}
    DISPATCH;
    P_SYSCALL:
        syscall(gpregs);
        DISPATCH
    ;
    P_LD8:{
    		uint8_t regid;
    		regid = EAT_BYTE();
    		gpregs[regid] = READ8(EAT_QWORD());
    	} DISPATCH;
    P_LD16:{
    		uint8_t regid;
    		regid = EAT_BYTE();
    		gpregs[regid] = READ16(EAT_QWORD());
    	}DISPATCH
    P_LD32:{
    		uint8_t regid;
    		regid = EAT_BYTE();
    		gpregs[regid] = READ32(EAT_QWORD());
    	}DISPATCH
    P_LD64:{
    		uint8_t regid;
    		regid = EAT_BYTE();
    		gpregs[regid] = READ64(EAT_QWORD());
    	}DISPATCH
    P_ST8:{
    		uint8_t regid;
    		regid = EAT_BYTE();
    		WRITE8(EAT_QWORD(),gpregs[regid]);
	}DISPATCH
    P_ST16:{
    		uint8_t regid;
    		regid = EAT_BYTE();
    		WRITE16(EAT_QWORD(),gpregs[regid]);
	}DISPATCH
    P_ST32:{
    		uint8_t regid;
    		regid = EAT_BYTE();
    		WRITE32(EAT_QWORD(),gpregs[regid]);
	}DISPATCH
    P_ST64:{
    		uint8_t regid;
    		regid = EAT_BYTE();
    		WRITE64(EAT_QWORD(),gpregs[regid]);
	}DISPATCH
    P_ILD8:{
    		uint8_t regid_src;
    		uint8_t regid_dest;
    		regid_dest = EAT_BYTE();
    		regid_src = EAT_BYTE();
    		gpregs[regid_dest] = READ8(gpregs[regid_src]);
	}DISPATCH
    P_ILD16:{
    		uint8_t regid_src;
    		uint8_t regid_dest;
    		regid_dest = EAT_BYTE();
    		regid_src = EAT_BYTE();
    		gpregs[regid_dest] = READ16(gpregs[regid_src]);
	}DISPATCH
    P_ILD32:{
    		uint8_t regid_src;
    		uint8_t regid_dest;
    		regid_dest = EAT_BYTE();
    		regid_src = EAT_BYTE();
    		gpregs[regid_dest] = READ32(gpregs[regid_src]);
	}DISPATCH
    P_ILD64:{
    		uint8_t regid_src;
    		uint8_t regid_dest;
    		regid_dest = EAT_BYTE();
    		regid_src = EAT_BYTE();
    		gpregs[regid_dest] = READ64(gpregs[regid_src]);
	}DISPATCH
    P_IST8:{
    		uint8_t regid_src;
    		uint8_t regid_dest;
    		regid_dest = EAT_BYTE();
    		regid_src = EAT_BYTE();
    		WRITE8(gpregs[regid_dest],gpregs[regid_src]);
	}DISPATCH
    P_IST16:{
    		uint8_t regid_src;
    		uint8_t regid_dest;
    		regid_dest = EAT_BYTE();
    		regid_src = EAT_BYTE();
    		WRITE16(gpregs[regid_dest],gpregs[regid_src]);
	}DISPATCH
    P_IST32:{
    		uint8_t regid_src;
    		uint8_t regid_dest;
    		regid_dest = EAT_BYTE();
    		regid_src = EAT_BYTE();
    		WRITE32(gpregs[regid_dest],gpregs[regid_src]);
	}DISPATCH
    P_IST64:{
    		uint8_t regid_src;
    		uint8_t regid_dest;
    		regid_dest = EAT_BYTE();
    		regid_src = EAT_BYTE();
    		WRITE64(gpregs[regid_dest],gpregs[regid_src]);
	}DISPATCH
    P_LSH:
    	{
		uint8_t regid_src;
		uint8_t regid_dest;
		regid_dest = EAT_BYTE();
		regid_src = EAT_BYTE();
		gpregs[regid_dest] <<= gpregs[regid_src];
	}
    DISPATCH;
    P_RSH:
	{
		uint8_t regid_src;
		uint8_t regid_dest;
		regid_dest = EAT_BYTE();
		regid_src = EAT_BYTE();
		gpregs[regid_dest] >>= gpregs[regid_src];
	}
    DISPATCH;
    P_AND:
	{
		uint8_t regid_src;
		uint8_t regid_dest;
		regid_dest = EAT_BYTE();
		regid_src = EAT_BYTE();
		gpregs[regid_dest] &= gpregs[regid_src];
	}
    DISPATCH;
    P_OR:
	{
		uint8_t regid_src;
		uint8_t regid_dest;
		regid_dest = EAT_BYTE();
		regid_src = EAT_BYTE();
		gpregs[regid_dest] |= gpregs[regid_src];
	}
    DISPATCH;
    P_XOR:
	{
		uint8_t regid_src;
		uint8_t regid_dest;
		regid_dest = EAT_BYTE();
		regid_src = EAT_BYTE();
		gpregs[regid_dest] ^= gpregs[regid_src];
	}
    DISPATCH;
    
    P_COMPL:	{
		uint8_t regid_dest;
		regid_dest = EAT_BYTE();
		gpregs[regid_dest] = ~(uint64_t)gpregs[regid_dest];
    }DISPATCH;
    P_NEG:{
    		uint8_t regid_dest;
    		regid_dest = EAT_BYTE();
    		gpregs[regid_dest] = -1 * (int64_t)gpregs[regid_dest];
	}DISPATCH;
    P_BOOL:{
    		uint8_t regid_dest;
    		regid_dest = EAT_BYTE();
    		gpregs[regid_dest] = !!gpregs[regid_dest];
	}DISPATCH;
    P_NOT:{
    		uint8_t regid_dest;
    		regid_dest = EAT_BYTE();
    		gpregs[regid_dest] = !gpregs[regid_dest];
	}DISPATCH
    
    P_IADD:	{
		uint8_t regid_src;
		uint8_t regid_dest;
		regid_dest = EAT_BYTE();
		regid_src = EAT_BYTE();
		gpregs[regid_dest] += gpregs[regid_src];
	}DISPATCH
    P_ISUB:
	{
		uint8_t regid_src;
		uint8_t regid_dest;
		regid_dest = EAT_BYTE();
		regid_src = EAT_BYTE();
		gpregs[regid_dest] -= gpregs[regid_src];
	}DISPATCH
    P_IMUL:
    	{
		uint8_t regid_src;
		uint8_t regid_dest;
		regid_dest = EAT_BYTE();
		regid_src = EAT_BYTE();
		gpregs[regid_dest] *= gpregs[regid_src];
	}DISPATCH    
    P_FADD:{
    		f32 a;
    		f32 b;
    		uint8_t regid_dest;
    		uint8_t regid_src;
    		regid_dest = EAT_BYTE();
    		regid_src = EAT_BYTE();
    		a = u32_rto_f32(gpregs[regid_dest]);
    		b = u32_rto_f32(gpregs[regid_src]);
    		a = a + b;
    		gpregs[regid_dest] = (int64_t)f32_rto_i32(a);
	}DISPATCH
    P_FSUB:{
    		f32 a;
    		f32 b;
    		uint8_t regid_dest;
    		uint8_t regid_src;
    		regid_dest = EAT_BYTE();
    		regid_src = EAT_BYTE();
    		a = u32_rto_f32(gpregs[regid_dest]);
    		b = u32_rto_f32(gpregs[regid_src]);
    		a = a - b;
    		gpregs[regid_dest] = (int64_t)f32_rto_i32(a);
	}DISPATCH
    P_FMUL:{
    		f32 a;
    		f32 b;
    		uint8_t regid_dest;
    		uint8_t regid_src;
    		regid_dest = EAT_BYTE();
    		regid_src = EAT_BYTE();
    		a = u32_rto_f32(gpregs[regid_dest]);
    		b = u32_rto_f32(gpregs[regid_src]);
    		a = a * b;
    		gpregs[regid_dest] = (int64_t)f32_rto_i32(a);
	}DISPATCH
    P_DADD:{
    		f64 a;
    		f64 b;
    		uint8_t regid_dest;
    		uint8_t regid_src;
    		regid_dest = EAT_BYTE();
    		regid_src = EAT_BYTE();
    		a = u64_rto_f64(gpregs[regid_dest]);
    		b = u64_rto_f64(gpregs[regid_src]);
    		a = a + b;
    		gpregs[regid_dest] = f64_rto_i64(a);
	}DISPATCH
    P_DSUB:{
    		f64 a;
    		f64 b;
    		uint8_t regid_dest;
    		uint8_t regid_src;
    		regid_dest = EAT_BYTE();
    		regid_src = EAT_BYTE();
    		a = u64_rto_f64(gpregs[regid_dest]);
    		b = u64_rto_f64(gpregs[regid_src]);
    		a = a - b;
    		gpregs[regid_dest] = f64_rto_i64(a);
    	}DISPATCH
    P_DMUL:{
    		f64 a;
    		f64 b;
    		uint8_t regid_dest;
    		uint8_t regid_src;
    		regid_dest = EAT_BYTE();
    		regid_src = EAT_BYTE();
    		a = u64_rto_f64(gpregs[regid_dest]);
    		b = u64_rto_f64(gpregs[regid_src]);
    		a = a * b;
    		gpregs[regid_dest] = f64_rto_i64(a);
    	}DISPATCH
    
    P_IDIV:	{
		uint8_t regid_src;
		uint8_t regid_dest;
		int64_t divisee;
		int64_t divisor;
		regid_dest = EAT_BYTE();
		regid_src = EAT_BYTE();

		divisee = gpregs[regid_dest];
		divisor = gpregs[regid_src];
		gpregs[regid_dest] = divisee / divisor;
	}DISPATCH
    P_IMOD:{
    		uint8_t regid_src;
    		uint8_t regid_dest;
    		int64_t divisee;
    		int64_t divisor;
    		regid_dest = EAT_BYTE();
    		regid_src = EAT_BYTE();
    
    		divisee = gpregs[regid_dest];
    		divisor = gpregs[regid_src];
    		gpregs[regid_dest] = divisee % divisor;
	}DISPATCH
    
    P_UDIV:{
    		uint8_t regid_src;
    		uint8_t regid_dest;
    		uint64_t divisee;
    		uint64_t divisor;
    		regid_dest = EAT_BYTE();
    		regid_src = EAT_BYTE();
    
    		divisee = gpregs[regid_dest];
    		divisor = gpregs[regid_src];
    		gpregs[regid_dest] = divisee / divisor;
	}DISPATCH
    P_UMOD:{
    		uint8_t regid_src;
    		uint8_t regid_dest;
    		uint64_t divisee;
    		uint64_t divisor;
    		regid_dest = EAT_BYTE();
    		regid_src = EAT_BYTE();
    
    		divisee = gpregs[regid_dest];
    		divisor = gpregs[regid_src];
    		gpregs[regid_dest] = divisee % divisor;
	}DISPATCH    
    
    P_FDIV:{
    		f32 a;
    		f32 b;
    		uint8_t regid_dest;
    		uint8_t regid_src;
    		regid_dest = EAT_BYTE();
    		regid_src = EAT_BYTE();
    		a = u32_rto_f32(gpregs[regid_dest]);
    		b = u32_rto_f32(gpregs[regid_src]);
    		a = a / b;
    		gpregs[regid_dest] = (int64_t)f32_rto_i32(a);
	}DISPATCH
    P_DDIV:{
    		f64 a;
    		f64 b;
    		uint8_t regid_dest;
    		uint8_t regid_src;
    		regid_dest = EAT_BYTE();
    		regid_src = EAT_BYTE();
    		a = u64_rto_f64(gpregs[regid_dest]);
    		b = u64_rto_f64(gpregs[regid_src]);
    		a = a / b;
    		gpregs[regid_dest] = f64_rto_i64(a);
	}DISPATCH
    P_ZE32:{
    		gpregs[EAT_BYTE()] &= (uint64_t)0xffFFffFF;
	}DISPATCH
    P_ZE16:{
    		gpregs[EAT_BYTE()] &= (uint64_t)0xffFF;
	}DISPATCH
    P_ZE8:{
    		gpregs[EAT_BYTE()] &= (uint64_t)0xff;
	}DISPATCH    
    
    P_SE32:{
    		uint8_t regid;
    		int32_t val;
    		int64_t val64;
    		regid = EAT_BYTE();
    		val = gpregs[regid];
    		val64 = val;
    		gpregs[regid] = val64;
	}DISPATCH
    P_SE16:{
    		uint8_t regid;
    		int16_t val;
    		int64_t val64;
    		regid = EAT_BYTE();
    		val = gpregs[regid];
    		val64 = val;
    		gpregs[regid] = val64;
	}DISPATCH
    P_SE8:{
    		uint8_t regid;
    		int8_t val;
    		int64_t val64;
    		regid = EAT_BYTE();
    		val = gpregs[regid];
    		val64 = val;
    		gpregs[regid] = val64;
	}DISPATCH
    P_UCMP:{
    		uint8_t targ_reg;
    		uint64_t val1;
    		uint64_t val2;
    		targ_reg = EAT_BYTE();
    
    		val1 = gpregs[EAT_BYTE()];
    		val2 = gpregs[EAT_BYTE()];
    		gpregs[targ_reg] = (val1 > val2) * 1 +
    							(val1 < val2) * (int64_t)-1;
	}DISPATCH
    P_ICMP:{
    		uint8_t targ_reg;
    		int64_t val1;
    		int64_t val2;
    		targ_reg = EAT_BYTE();
    		val1 = gpregs[EAT_BYTE()];
    		val2 = gpregs[EAT_BYTE()];
    		gpregs[targ_reg] = (val1 > val2) * 1 +
    							(val1 < val2) * (int64_t)-1;
	}DISPATCH
    P_FCMP:{
    		f32 val1;
    		f32 val2;
    		uint8_t regid_a;
    		uint8_t regid_b;
    		uint8_t regid_d;
    		regid_d = EAT_BYTE();
    		regid_a = EAT_BYTE();
    		regid_b = EAT_BYTE();
    		val1 = u32_rto_f32(gpregs[regid_a]);
    		val2 = u32_rto_f32(gpregs[regid_b]);
    		
    		gpregs[regid_d] = (val1 > val2) * 1 +
    									(val1 < val2) * (int64_t)-1;
	}DISPATCH
    P_DCMP:{
    		f64 val1;
    		f64 val2;
    		uint8_t regid_a;
    		uint8_t regid_b;
    		uint8_t regid_d;
    		regid_d = EAT_BYTE();
    		regid_a = EAT_BYTE();
    		regid_b = EAT_BYTE();
    		val1 = u64_rto_f64(gpregs[regid_a]);
    		val2 = u64_rto_f64(gpregs[regid_b]);
    		
    		gpregs[regid_d] = (val1 > val2) * 1 +
    									(val1 < val2) * (int64_t)-1;
	}DISPATCH
    P_JMP:{
    		uint64_t val;
    		val = EAT_QWORD();
    		pc = active_mod->bytecode + val;
	}DISPATCH
    P_JNZ:{
    		char* dest;
    		uint8_t regid;
    		uint8_t test;
    		regid = EAT_BYTE();
    		dest = active_mod->bytecode + EAT_QWORD();
    		test = (gpregs[regid] != 0);
    		
    		if(test)
    			pc = dest;
    		/*pc = (test * dest) | ((!test) * pc);*/
	}DISPATCH
    P_JIZ:{
        		char* dest;
        		uint8_t regid;
        		uint8_t test;
        		regid = EAT_BYTE();
        		dest = active_mod->bytecode + EAT_QWORD();
        		test = (gpregs[regid] == 0);
        		
        		if(test)
        			pc = dest;
        		/*pc = (test * dest) | ((!test) * pc);*/
	}DISPATCH
    P_JLT:{
        		char* dest;
        		uint8_t regid;
        		uint8_t test;
        		regid = EAT_BYTE();
        		dest = active_mod->bytecode + EAT_QWORD();
        		test = (gpregs[regid] == (i64)-1);
        		
        		if(test)
        			pc = dest;
        		/*pc = (test * dest) | ((!test) * pc);*/
	}DISPATCH
    P_JGT:{
        		char* dest;
        		uint8_t regid;
        		uint8_t test;
        		regid = EAT_BYTE();
        		dest = active_mod->bytecode + EAT_QWORD();
        		test = (gpregs[regid] == 1);
        		
        		if(test)
        			pc = dest;
        		/*pc = (test * dest) | ((!test) * pc);*/
    	}DISPATCH
    //TODO
    P_CALL:{
        //push active_mod
		WRITE64(stp,(u64)active_mod);
		stp += 8;
		//push pc
		WRITE64(stp,(u64)(pc+8));
		stp += 8;
		//grab the new mod.
		u32 new_mod;
		u32 new_pc;
		new_mod = READ32(pc); pc+=4;
		new_pc = READ32(pc); 
		//now, do the call.
        pc = (mods + new_mod)->bytecode + new_pc;
    }DISPATCH
    P_CALLFNPTR:
    {
		u32 new_mod;
		u32 new_pc;
        //push active_mod
		WRITE64(stp,(u64)active_mod);
		stp += 8;
		//push pc
		WRITE64(stp,(u64)(pc+8));
		stp += 8;
		new_mod = gpregs[READ8(pc)];pc++;
		new_pc = gpregs[READ8(pc)];
		pc = (mods + new_mod)->bytecode + new_pc;
    }
    DISPATCH
    P_GETGLOBALPTR:
    {
        u8 treg;
        u32 gmod;
        u32 goff;
        treg = READ8(pc); pc++;
        gmod = READ32(pc);pc += 4;
        goff = READ32(pc);pc += 4;
        gpregs[treg] = (u64)((mods+gmod)->gdata + goff);
    }
    DISPATCH
    P_GETGLOBALPTRPTR:
    {
        u8 treg;
        u32 gmod;
        u32 goff;
        treg = READ8(pc); pc++;
        gmod = (gpregs[treg] >> 32);
        goff = gpregs[treg];
        gpregs[treg] = (u64)((mods+gmod)->gdata + goff);
    }
    DISPATCH
    P_RET:{
        //pop pc and active_mod
        u64 _pc;
        u64 _active_mod;
        _pc = READ64(stp-8);
        _active_mod = READ64(stp-16);
        stp -=16;
        pc = (char*)_pc;
    }DISPATCH
    //TODO ^ 
    P_GETSTP:{
    		uint8_t regid;
    		regid = EAT_BYTE();
    		gpregs[regid] = (u64)stp;
	}DISPATCH
    P_SETSTP:{
    		uint8_t regid;
    		regid = EAT_BYTE();
    		stp = (char*)gpregs[regid];
	}DISPATCH
    P_PUSH64:{
    		uint8_t regid;
    		regid = EAT_BYTE();
    		WRITE64(stp,gpregs[regid]);
    		stp += 8;
	}DISPATCH    
    P_PUSH32:{
    		uint8_t regid;
    		regid = EAT_BYTE();
    		WRITE32(stp,gpregs[regid]);
    		stp += 4;
	}DISPATCH
    P_PUSH16:{
    		uint8_t regid;
    		regid = EAT_BYTE();
    		WRITE16(stp,gpregs[regid]);
    		stp += 2;
	}DISPATCH
    P_PUSH8:{
    		uint8_t regid;
    		regid = EAT_BYTE();
    		WRITE8(stp,gpregs[regid]);
    		stp += 1;
	}DISPATCH
    P_POP64:{
    		uint8_t regid;
    		regid = EAT_BYTE();
    		stp -= 8;
    		gpregs[regid] = READ64(stp);
	}DISPATCH
    P_POP32:{
    		uint8_t regid;
    		regid = EAT_BYTE();
    		stp -= 4;
    		gpregs[regid] = READ32(stp);
	}DISPATCH
    P_POP16:{
    		uint8_t regid;
    		regid = EAT_BYTE();
    		stp -= 2;
    		gpregs[regid] = READ16(stp);
	}DISPATCH
    P_POP8:{
    		uint8_t regid;
    		regid = EAT_BYTE();
    		stp -= 1;
    		gpregs[regid] = READ8(stp);
	}DISPATCH
    P_ITOD:{
    		uint8_t regid;
    		regid = EAT_BYTE();
    		gpregs[regid] = f64_rto_i64( /*pun the f64 to an i64*/
    			(double) /*do the actual conversion from integer to f64*/
    			((int64_t)gpregs[regid]) /*reinterpret the u64 to an i64*/
    
    		);
	}DISPATCH
    P_ITOF:{
    		uint8_t regid;
    		uint32_t temp; /*Need this to avoid a sign extension.*/
    		regid = EAT_BYTE();
    		
    		temp = f32_rto_i32( /*pun the float to an i32*/
    			(float)  /*do the actual conversion from integer to float*/
    			((int32_t)gpregs[regid]) /*reinterpret the bottom 32 bits of the u64 as an i32.*/
    		);
    		gpregs[regid] = temp; /*no sign extension occurs.*/
	}DISPATCH
    P_DTOI:{
    		uint8_t regid;
    		regid = EAT_BYTE();
    		gpregs[regid] = (int64_t)u64_rto_f64(gpregs[regid]);
	}DISPATCH
    P_FTOI:{ /*FTOI sign extends...*/
    		uint8_t regid;
    		regid = EAT_BYTE();
    		gpregs[regid] = (int64_t)(int32_t)u32_rto_f32(gpregs[regid]);
	}DISPATCH
    P_DTOF:{
    		uint8_t regid;
    		f32 f;
    		f64 d;
    		uint32_t temp; //this is needed to prevent a sign extension.
    		d = u64_rto_f64(gpregs[regid]); //convert th
    		f = d;
    		temp = f32_rto_i32(f); //no sign conversion
    		gpregs[regid] = temp; //unsigned to unsigned- no sign conversion.
    		regid = EAT_BYTE();
	}DISPATCH
    P_FTOD:{
    		uint8_t regid;
    		f32 f;
    		f64 d;
    		f = u32_rto_f32(gpregs[regid]);
    		d = f;
    		gpregs[regid] = (uint64_t)f64_rto_i64(d);
    		//u64_rto_f64
    		regid = EAT_BYTE();
	}DISPATCH
    P_INC:{
    		uint8_t regid_d;
    		regid_d = EAT_BYTE();
    		gpregs[regid_d]++;
	}DISPATCH
    P_DEC:{
    		uint8_t regid_d;
    		regid_d = EAT_BYTE();
    		gpregs[regid_d]--;
	}DISPATCH
    
    P_LDSP64:{
    		uint8_t regid;
    		uint64_t off;
    		regid = EAT_BYTE();
    		off = EAT_LONG();
    		gpregs[regid] = READ64(stp - off);
	}DISPATCH    
    P_LDSP32:{
    		uint8_t regid;
    		uint64_t off;
    		regid = EAT_BYTE();
    		off = EAT_LONG();
    		gpregs[regid] = READ32(stp - off);
	}DISPATCH
    P_LDSP16:{
    		uint8_t regid;
    		uint64_t off;
    		regid = EAT_BYTE();
    		off = EAT_LONG();
    		gpregs[regid] = READ16(stp - off);
	}DISPATCH
    P_LDSP8:{
    		uint8_t regid;
    		uint64_t off;
    		regid = EAT_BYTE();
    		off = EAT_LONG();
    		gpregs[regid] = READ8(stp - off);
	}DISPATCH
    
    P_STSP64:{
    		uint8_t regid;
    		uint64_t off;
    		regid = EAT_BYTE();
    		off = EAT_LONG();
    		WRITE64(stp - off, gpregs[regid]);
	}DISPATCH
    P_STSP32:{
    		uint8_t regid;
    		uint64_t off;
    		regid = EAT_BYTE();
    		off = EAT_LONG();
    		WRITE32(stp - off, gpregs[regid]);
	}DISPATCH
    P_STSP16:{
    		uint8_t regid;
    		uint64_t off;
    		regid = EAT_BYTE();
    		off = EAT_LONG();
    		WRITE16(stp - off, gpregs[regid]);
	}DISPATCH
    P_STSP8:{
    		uint8_t regid;
    		uint64_t off;
    		regid = EAT_BYTE();
    		off = EAT_LONG();
    		WRITE8(stp - off, gpregs[regid]);
	}DISPATCH
    
    P_BSWAP64:{
    		uint8_t regid;
    		regid = EAT_BYTE();
    		gpregs[regid] = u64_bswap(gpregs[regid]);
	}DISPATCH
    P_BSWAP32:{
    		uint8_t regid;
    		regid = EAT_BYTE();
    		gpregs[regid] = u32_bswap(gpregs[regid]);
	}DISPATCH
    P_BSWAP16:{
    		uint8_t regid;
    		regid = EAT_BYTE();
    		gpregs[regid] = u16_bswap(gpregs[regid]);
	}DISPATCH

    P_MNZ:	
    {
		uint8_t regid_dest;
		uint8_t regid_src;
		uint8_t regid_testme;
		uint64_t test;
		regid_testme = EAT_BYTE();
		regid_dest = EAT_BYTE();
		regid_src = EAT_BYTE();
		test = (gpregs[regid_testme] != 0) ;
		gpregs[regid_dest] = 
			(test * 
            	gpregs[regid_src]
            )
            +
            ((!test) * 
            	gpregs[regid_dest]
			)
 		;
	}DISPATCH
    P_LITERAL64:;
	{
		uint8_t regid;
		regid = EAT_BYTE();
		gpregs[regid] = EAT_QWORD();
	}
	DISPATCH;

	P_LITERAL32:;
	{
		uint8_t regid;
		regid = EAT_BYTE();
		gpregs[regid] = EAT_LONG();
	}
	DISPATCH;

	P_LITERAL16:;
	{
		uint8_t regid;
		regid = EAT_BYTE();
		gpregs[regid] = EAT_SHORT();
	}
	DISPATCH;

	P_LITERAL8:;
	{
		uint8_t regid;
		regid = EAT_BYTE();
		gpregs[regid] = EAT_BYTE();
	}
	DISPATCH;
}
