#include "simulate.h"






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
        __int32_t immediate = 0;

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
                int imm11_5 = (instructions >> 25) & 0x7F;
                int imm4_0 = (instructions >> 7) & 0x1F;
                immediate = (imm11_5 << 5) | imm4_0;
                switch (funct3){
                    case SB:
                        printf("SB \n");
                        memory_wr_b(mem, rs1+immediate, reg[rs2]);
                        break;
                    case SH:
                        printf("SH \n");
                        memory_wr_b(mem, rs1+immediate, reg[rs2]);
                        break;
                    case SW:
                        printf("SW \n");
                        memory_wr_b(mem, rs1+immediate, reg[rs2]);
                        break;
                    default:
                        break;
                }
            case(LOAD):
                switch (funct3){
                    case LB:
                        printf("LB \n");
                        break;
                    case LH:
                        printf("LH \n");
                        break;
                    case LW:
                        printf("LW \n");
                        break;
                    case LBU:
                        printf("LBU \n");
                        break;
                    case LHU:
                        printf("LHU \n");
                        break;
                    default:
                        break;
                }
            case(BRANCH):
                switch (funct3){
                    case BEQ:
                        printf("BEQ \n");
                        if (reg[rs1] == reg[rs2]){pc = pc+4;};
                        break;
                    case BNE:
                        printf("BNE \n");
                        if (reg[rs1] /= reg[rs2]){pc = pc + (immediate*4);};
                        break;
                    case BLT:
                        printf("BLT \n");
                        if(reg[rs1] >= reg[rs2]){pc = pc+4;};
                        break;
                    case BGE:
                        printf("BGE \n");
                        if(reg[rs1] <= reg[rs2]){pc = pc + (immediate*4);};
                        break;
                    case BLTU:
                        printf("BLTU \n");
                        if((unsigned int)reg[rs1] >= (unsigned int)reg[rs2]){pc = pc+4;};
                        break;
                    case BGEU:
                        printf("BGEU \n");
                        if((unsigned int)reg[rs1] <= (unsigned int)reg[rs2]){pc = pc + (immediate*4);};
                        break;
                    default:
                        break;
                }
            case(0X33): // CHecking WHETHER IT IS OP or EXTENTIOn?
                if (funct7 == MULDIVREM){
                    switch (funct3){
                        case MUL:
                            printf("MUL \n");
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
                            printf("DIV \n");
                            break;
                        case DIVU:
                            printf("DIVU \n");
                            break;
                        case REM:
                            printf("REM \n");
                            break;
                        case REMU:
                            printf("REMU \n");
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
            default:
                break;
        }
    
        //printf("Opcode value at address %x: %x\n", pc, opcode);
        
        pc = pc + 4;
    }
    return;
}
