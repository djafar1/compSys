#include "simulate.h"
#include <stdbool.h>

int32_t sign_extend(int bit_width, int32_t value) {
    int sign_bit = bit_width - 1;

    // Check the sign bit
    if (value & (1 << sign_bit)) {
        // Perform sign extension by filling with 1s
        int32_t sign_extension = (int32_t)(-1) << sign_bit;
        return value | sign_extension;
    } else {
        // Positive value, no sign extension needed
        return value;
    }
}


int32_t MAX_INSTRUCTIONS = 20000; // For debugging

long int simulate(struct memory *mem, struct assembly *as, int start_addr, FILE *log_file) {
    int32_t reg[32];
    int32_t pc = start_addr;
    reg[0] = 0x00;
    int32_t instructions = 0;
    int32_t numberofinstruction = 0;
    while(1){
        instructions = memory_rd_w(mem, pc); 
        if (instructions == 0){
            printf("Null What are we printing: %x \n", instructions);
            return numberofinstruction;        
            }
        numberofinstruction++;
        printf("Instruction at address %08x: %08x\n", pc, instructions);
        int32_t  opcode = instructions & 0x7f; 
        int32_t  funct3 = (instructions >> 12) & 0x7;
        int32_t  funct7 = (instructions >> 25) & 0x7f;
        int32_t  immediate;
        int32_t  imm12, imm10_5, imm4_1, imm11, imm4_0, imm11_5, imm20, imm10_1, imm19_12;

        int32_t  rd = (instructions >> 7) & 0x1F;
        int32_t  rs1 = (instructions >> 15) & 0x1F;
        int32_t  rs2 = (instructions >> 20) & 0x1F;

        switch (opcode){
            case (JAL):
                imm20 = (instructions >> 31) & 0x1;
                imm10_1 = (instructions >> 21) & 0x3FF;
                imm11 = (instructions >> 20) & 0x1;
                imm19_12 = (instructions >> 12) & 0xFF;
                immediate = (imm20 << 20) | (imm19_12 << 12) | (imm11 << 11) | (imm10_1 << 1);
                immediate = sign_extend(20, immediate);
                printf("JAL rd=%d, imm=%d\n", rd, immediate);
                reg[rd] = pc + 4;
                pc = pc + immediate;
                continue;
            case(LUI):
                immediate = (instructions >> 12) & 0xFFFFF;
                reg[rd] = immediate << 12;
                printf("LUI rd=%d, imm=%d\n", rd, immediate);
                break;
            case(AUIPC):
                immediate = (instructions >> 12) & 0xFFFFF;
                reg[rd] = pc + (immediate << 12);
                printf("AUIPC rd=%d, imm=%d\n", rd, immediate);
                break;
            case (JALR):
                immediate = (instructions >> 20) & 0xFFF;
                printf("Before sign extension: %d (0x%x)\n", immediate, immediate); // debugging
                immediate = sign_extend(12, immediate);
                printf("After sign extension: %d (0x%x)\n", immediate, immediate); // debugging
                printf("JALR rd=%d, rs1=%d, imm=%d\n", rd, rs1, immediate);
                reg[rd] = pc + 4;
                int target_address = (reg[rs1] + immediate) & (~1); 
                pc = target_address;
                continue;
            case (OPIMM):
                immediate = (instructions >> 20) & 0xFFF;
                switch (funct3){
                    case ADDI:
                        printf("ADDI rd=%d, rs1=%d, imm=%d\n", rd, rs1, immediate);
                        reg[rd] = reg[rs1] + immediate;
                        printf("Updated reg[%d] = %d\n", rd, reg[rd]);
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
                        reg[rd] = (uint32_t)reg[rs1] < (uint32_t)immediate ? 1 
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
                printf("rs1: %d \n", rs1);
                printf("Store: reg[rs1]=%d, immediate=%d, reg[rs2]=%d\n", reg[rs1], immediate, reg[rs2]);
                printf(" target adress to %d \n ", reg[rs1] + immediate);
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
                printf("Load from address: %d\n", reg[rs1] + immediate);

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
                        printf("LW \n");
                        reg[rd] = memory_rd_w(mem, reg[rs1] + immediate);
                        break;
                    case LBU: 
                        printf("LBU \n");
                        reg[rd] = (uint32_t)(memory_rd_b(mem, reg[rs1] + immediate) & 0xFF);; // zero extend
                        break;
                    case LHU: 
                        printf("LHU \n");
                        reg[rd] = (uint32_t)(memory_rd_h(mem, reg[rs1] + immediate) & 0xFFFF);; // zero extend
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
                printf("Before sign extension: %d (0x%x)\n", immediate, immediate);

                immediate = sign_extend(12, immediate);
                printf("After sign extension: %d (0x%x)\n", immediate, immediate);

                // Bool to whether skip the pc + 4 in the end , then we continue to next iteration
                bool branchinstruct = false;
                // In every case we take pc = pc + (immediate * 4) 
                // 4 * 32-bit instructions == 16 bytes.
                switch (funct3){
                    case BEQ:
                        printf("BEQ \n");
                        if (reg[rs1] == reg[rs2]){pc = pc + (immediate); branchinstruct = true;};
                        break;
                    case BNE:
                        printf("BNE \n");
                         printf("BNE: reg[rs1]=%d, reg[rs2]=%d\n", reg[rs1], reg[rs2]);
                        if (reg[rs1] != reg[rs2]){
                            pc = pc + (immediate); 
                            branchinstruct = true;
                        };
                        break;
                    case BLT:
                        printf("BLT \n");
                        if(reg[rs1] >= reg[rs2]){pc = pc + (immediate); branchinstruct = true;};
                        break;
                    case BGE:
                        printf("BGE \n");
                        if(reg[rs1] <= reg[rs2]){pc = pc + (immediate); branchinstruct = true;};
                        break;
                    case BLTU:
                        printf("BLTU \n");
                        if((uint32_t)reg[rs1] >= (uint32_t)reg[rs2]){
                            pc = pc + immediate; 
                            branchinstruct = true;
                            };
                        break;
                    case BGEU:
                        printf("BGEU \n");
                        if((uint32_t)reg[rs1] <= (uint32_t)reg[rs2]){pc = pc + (immediate); branchinstruct = true;};
                        break;
                    default:
                        break;
                }
                if (branchinstruct == true){
                    continue;
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
                            reg[rd] = (uint32_t)reg[rs1]/(uint32_t)reg[rs2];;
                            break;
                        case REM:
                            printf("REM rd=%d, rs1=%d, is2=%d\n", rd, rs1, rs2);
                            reg[rd] = reg[rs1]%reg[rs2];
                            break;
                        case REMU:
                            printf("REMU rd=%d, rs1=%d, is2=%d\n", rd, rs1, rs2);
                            reg[rd] = (uint32_t)reg[rs1]%(uint32_t)reg[rs2];;
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
                                    printf("ADD rd=%d, rs1=%d, rs2=%d\n", rd, rs1, rs2);
                                    reg[rd] = reg[rs1] + reg[rs2];
                                    printf("Updated reg[%d] = %d\n", rd, reg[rd]);
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
                            reg[rd] = (uint32_t)reg[rs1] < (uint32_t)reg[rs2] ? 1 : 0; 
                            break;
                        case XOR:
                            printf("XOR rd=%d, rs1=%d, is2=%d\n", rd, rs1, rs2);
                            reg[rd] = reg[rs1] ^ reg[rs2];
                            break;
                        case SRLA:
                            switch (funct7){
                                case SRL:
                                    printf("SRL rd=%d, rs1=%d, is2=%d\n", rd, rs1, rs2);
                                    reg[rd] = reg[rs1] >> reg[rs2];
                                    break;
                                case SRA:
                                    printf("SRA rd=%d, rs1=%d, is2=%d\n", rd, rs1, rs2);
                                    reg[rd] = (int32_t) reg[rs1] >> reg[rs2];
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

        if (numberofinstruction > MAX_INSTRUCTIONS) {
            fprintf(stderr, "Issieus with infinite loop\n");
            return -1;
        }
        //printf("Opcode value at address %x: %x\n", pc, opcode);
        pc = pc + 4;
    }
}
