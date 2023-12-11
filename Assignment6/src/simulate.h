#ifndef __SIMULATE_H__
#define __SIMULATE_H__

#include "memory.h"
#include "assembly.h"
#include <stdio.h>

#define JALR 0x67

#define LUI 0x37

#define AUIPC 0x17

#define JAL 0x6F

#define ECALL 0x73 

#define OPIMM 0x13
    #define ADDI 0x0 
    #define SLLI 0x1
    #define SLTI 0x2
    #define SLTIU 0x3
    #define XORI 0x4
    #define SRI 0x5
        #define SRLI 0x00
        #define SRAI 0x20
    #define ORI 0x6
    #define ANDI 0x7

#define STORE 0x23
    #define SB 0x0
    #define SH 0X1
    #define SW 0x2

#define LOAD 0x3
    #define LB 0x0
    #define LH 0x1
    #define Lw 0x2
    #define LBU 0x3
    #define LHU 0x4

#define BRANCH 0x63
    #define BEQ 0x0
    #define BNE 0x1
    #define BLT 0x4
    #define BGE 0x5
    #define BLTU 0x6
    #define BGEU 0x7

#define OP 0x33
    #define ADDorSUB 0x0
        #define ADD 0x0 // lOOKING at funct7
        #define SUB 0x20
    #define SLL 0x1
    #define SLT 0x2
    #define SLTU 0x3
    #define XOR 0x4
    #define SRLA 0x5  
        #define SRL 0x0
        #define SRA 0x20
    #define OR 0x6
    #define AND 0x7

#define MULDIVREM 0x1 
// Here we need to look at the FUnct7 CAUSE it has same opcode as OP 0x33
    #define MUL 0x0
    #define MULH 0x1
    #define MULHSU 0x2
    #define MULHU 0x3
    #define DIV 0x4
    #define DIVU 0x5
    #define REM 0x6
    #define REMU 0x7

// Simuler RISC-V program i givet lager og fra given start adresse
long int simulate(struct memory *mem, struct assembly *as, int start_addr, FILE *log_file);

#endif
