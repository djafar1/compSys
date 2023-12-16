#include "simulate.h"

__int32_t sign_extend(int bit_width, __int32_t value) {
    int sign_bit = bit_width - 1;
    __int32_t sign_mask = 1 << sign_bit;

    // Check the sign bit
    if (value & sign_mask) {
        // Perform sign extension by filling with 1s
        __int32_t sign_extension = 0xFFFFFFFF << bit_width;
        return value | sign_extension;
    } else {
        // Positive value, no sign extension needed
        return value;
    }
}


long int simulate(struct memory *mem, struct assembly *as, int start_addr, FILE *log_file) {
    int reg[32];
    int pc = start_addr;
    reg[0] = 0x00;
    int instructions = 0;
    while(1){
        instructions = memory_rd_w(mem, pc); 
        if (instructions == NULL){
            printf("Null What are we printing: %x \n", instructions);
            return;
        }
        __int32_t opcode = instructions & 0x7f; 
        __int32_t funct3 = (instructions >> 12) & 0x7;
        __int32_t funct7 = (instructions >> 25) & 0x7f;
        __int32_t immediate;
        __int32_t imm12, imm10_5, imm4_1, imm11, imm4_0, imm11_5;

        __int32_t rd = (instructions >> 7) & 0x1F;
        __int32_t rs1 = (instructions >> 15) & 0x1F;
        __int32_t rs2 = (instructions >> 20) & 0x1F;
        switch (opcode){
            case (OPIMM):
                immediate = (instructions >> 20) & 0xFFF;
                switch (funct3){
                    case ADDI:
                        printf("ADDI rd=%d, rs1=%d, imm=%d\n", rd, rs1, immediate);
                        reg[rd] = reg[rs1] + immediate;
                        break;
                    case SLLI:
                        // Shift left logical immediate
                        printf("SLLI rd=%d, rs1=%d, imm=%d\n", rd, rs1, immediate);
                        reg[rd] = reg[rs1] << immediate;
                        break;
                    case SLTI:
                        // Set less than immediate
                        printf("SLTI rd=%d, rs1=%d, imm=%d\n", rd, rs1, immediate);
                        reg[rd] = reg[rs1] < immediate ? 1 : 0; 
                        break;
                    case SLTIU:
                        // Set less than immediate unsigned
                        printf("SLTIU rd=%d, rs1=%d, imm=%d\n", rd, rs1, immediate);
                        reg[rd] = (unsigned int)reg[rs1] < (unsigned int)immediate ? 1 
                        : 0;
                        break;
                    case XORI:
                        // XOR immediate
                        printf("XORI rd=%d, rs1=%d, imm=%d\n", rd, rs1, immediate);
                        reg[rd] = reg[rs1] ^ immediate;
                        break;
                    case SRI:
                        switch (funct7){
                            case SRLI:
                                // Shift right logical immediate
                                printf("SRLI rd=%d, rs1=%d, imm=%d\n", rd, rs1, immediate);
                                reg[rd] = reg[rs1] >> immediate;
                                break;
                            case SRAI:
                                // Shift right arithmetic immediate
                                printf("SRAI rd=%d, rs1=%d, imm=%d\n", rd, rs1, immediate);
                                reg[rd] = reg[rs1] >> immediate; // Note: Arithmetic shift right
                                break;
                            default:
                                break;
                        }
                        break;
                    case ORI:
                        // OR immediate
                        printf("ORI rd=%d, rs1=%d, imm=%d\n", rd, rs1, immediate);
                        reg[rd] = reg[rs1] | immediate;
                        break;
                    case ANDI:
                        // AND immediate
                        printf("ANDI rd=%d, rs1=%d, imm=%d\n", rd, rs1, immediate);
                        reg[rd] = reg[rs1] & immediate;
                        break;
                    default:
                        // Handle unknown funct3 for OPIMM instructions
                        break;
                }

                break;
            case(STORE):
                imm11_5 = (instructions >> 25) & 0x7F;
                imm4_0 = (instructions >> 7) & 0x1F;
                immediate = (imm11_5 << 5) | imm4_0;
                switch (funct3){
                    case SB:
                        printf("SB \n");
                        memory_wr_b(mem, reg[rs1]+immediate, reg[rs2]);
                        break;
                    case SH:
                        printf("SH \n");
                        memory_wr_h(mem, reg[rs1]+immediate, reg[rs2]);
                        break;
                    case SW:
                        printf("SW \n");
                        memory_wr_w(mem, reg[rs1]+immediate, reg[rs2]);
                        break;
                    default:
                        break;
                }
                break;
            case(LOAD):
                immediate = (instructions >> 20) & 0xFFF;
                switch (funct3){
                    case LB:
                        printf("LB \n");
                        reg[rd] = sign_extend(8, memory_rd_b(mem, reg[rs1] + immediate));
                        break;
                    case LH:
                        printf("LH \n");
                        reg[rd] = sign_extend(16, memory_rd_h(mem, reg[rs1] + immediate));
                        break;
                    case LW:
                        printf("Yes : %d \n", reg[rd]);
                        printf("LW \n");
                        reg[rd] = memory_rd_w(mem, reg[rs1] + immediate);
                        break;
                    case LBU: 
                        printf("LBU \n");
                        reg[rd] = memory_rd_b(mem, reg[rs1] + immediate) & 0xFFFF; // zero extend
                        break;
                    case LHU: 
                        printf("LHU \n");
                        reg[rd] = memory_rd_h(mem, reg[rs1] + immediate >> 1) & 0xFFFF; // zero extend
                        break;
                    default:
                        break;
                }
                break;
            case(BRANCH):
                imm12 = (instructions >> 31) & 0x1; // immediate value of 31?
                imm10_5 = (instructions >> 25) & 0x3F; // immediate value of 30:25
                imm4_1 = (instructions >> 8) & 0xF; // immediate value of 11:8
                imm11 = (instructions >> 7) & 0x1; // immediate value of 7
                // Immediate in the correct order. We left shift to make space for the others.
                immediate = (imm12 << 12) | (imm11 << 11) | (imm10_5 << 5) | (imm4_1 << 1) ;

                immediate = sign_extend(12, immediate);
                // In every case we take pc = pc + (immediate * 4) 
                // 4 * 32-bit instructions == 16 bytes.
                switch (funct3){
                    case BEQ:
                        printf("BEQ \n");
                        if (reg[rs1] == reg[rs2]){pc = pc + (immediate*4);};
                        break;
                    case BNE:
                        printf("BNE \n");
                        if (reg[rs1] != reg[rs2]){pc = pc + (immediate*4);};
                        break;
                    case BLT:
                        printf("BLT \n");
                        if(reg[rs1] >= reg[rs2]){pc = pc + (immediate*4);};
                        break;
                    case BGE:
                        printf("BGE \n");
                        if(reg[rs1] <= reg[rs2]){pc = pc + (immediate*4);};
                        break;
                    case BLTU:
                        printf("BLTU \n");
                        if((unsigned int)reg[rs1] >= (unsigned int)reg[rs2]){pc = pc + (immediate*4);};
                        break;
                    case BGEU:
                        printf("BGEU \n");
                        if((unsigned int)reg[rs1] <= (unsigned int)reg[rs2]){pc = pc + (immediate*4);};
                        break;
                    default:
                        break;
                }
                break;
            case(0X33): // Checking whether it is an OP or EXTENTION
                if (funct7 == MULDIVREM){
                    switch (funct3){
                        case MUL:
                            printf("MUL rd=%d, rs1=%d, is2=%d\n", rd, rs1, rs2);
                            reg[rd] = reg[rs1]*reg[rs2];
                            break;
                        case MULH:
                            printf("MULH \n");
                            break;
                        case MULHSU:
                            printf("MULHSU \n");
                            break;
                        case MULHU:
                            printf("MULHU \n");
                            break;
                        case DIV:
                            printf("DIV rd=%d, rs1=%d, is2=%d\n", rd, rs1, rs2);
                            reg[rd] = reg[rs1]/reg[rs2];
                            break;
                        case DIVU:
                            printf("DIVU rd=%d, rs1=%d, is2=%d\n", rd, rs1, rs2);
                            reg[rd] = (unsigned int)reg[rs1]/(unsigned int)reg[rs2];;
                            break;
                        case REM:
                            printf("REM rd=%d, rs1=%d, is2=%d\n", rd, rs1, rs2);
                            reg[rd] = reg[rs1]%reg[rs2];
                            break;
                        case REMU:
                            printf("REMU rd=%d, rs1=%d, is2=%d\n", rd, rs1, rs2);
                            reg[rd] = (unsigned int)reg[rs1]%(unsigned int)reg[rs2];;
                            break;
                        default:
                            break;
                    }
                }
                else{ 
                    switch (funct3){
                        case ADDorSUB:
                            switch (funct7){
                                case ADD:
                                    printf("ADD rd=%d, rs1=%d, is2=%d\n", rd, rs1, rs2);
                                    reg[rd] = reg[rs1] + reg[rs2];
                                    break;
                                case SUB:
                                    printf("sub rd=%d, rs1=%d, is2=%d\n", rd, rs1, rs2);
                                    reg[rd] = reg[rs1] - reg[rs2];
                                    break;
                                default:
                                    break;
                            }
                            break;
                        case SLL:
                            printf("SLL rd=%d, rs1=%d, is2=%d\n", rd, rs1, rs2);
                            reg[rd] = reg[rs1] << reg[rs2];
                            break;
                        case SLT:
                            printf("SLT rd=%d, rs1=%d, is2=%d\n", rd, rs1, rs2);
                            reg[rd] = reg[rs1] < reg[rs2] ? 1 : 0; 
                            break;
                        case SLTU:
                            printf("SLTU rd=%d, rs1=%d, is2=%d\n", rd, rs1, rs2);
                            reg[rd] = (unsigned int)reg[rs1] < (unsigned int)reg[rs2] ? 1 : 0; 
                            break;
                        case XOR:
                            printf("XOR rd=%d, rs1=%d, is2=%d\n", rd, rs1, rs2);
                            reg[rd] = reg[rs1] ^ reg[rs2];
                            break;
                        case SRLA:
                            switch (funct7){
                                case SRL:
                                    printf("SRL rd=%d, rs1=%d, is2=%d\n", rd, rs1, rs2);
                                    reg[rd] = reg[rs1] << reg[rs2];
                                    break;
                                case SRA:
                                    printf("SRA rd=%d, rs1=%d, is2=%d\n", rd, rs1, rs2);
                                    reg[rd] = reg[rs1] >> reg[rs2];
                                    break;
                                default:
                                    break;
                            }
                            break;
                        case OR:
                            printf("OR rd=%d, rs1=%d, is2=%d\n", rd, rs1, rs2);
                            reg[rd] = reg[rs1] | reg[rs2];
                            break;
                        case AND:
                            printf("AND rd=%d, rs1=%d, is2=%d\n", rd, rs1, rs2);
                            reg[rd] = reg[rs1] & reg[rs2];
                            break;
                        default:
                            break;
                    }
                }
                break;
            default:
                break;
        }
    
        //printf("Opcode value at address %x: %x\n", pc, opcode);
        
        pc = pc + 4;
    }
    return;
}
