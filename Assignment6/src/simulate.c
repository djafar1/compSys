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
        int opcode = instructions & 0x7f; 
        int funct3 = (instructions >> 12) & 0x7;
        int funct7 = (instructions >> 25) & 0x7f;

        switch (opcode){
            case (OPIMM):
                switch (funct3){
                    case ADDI:
                        printf("ADDI \n");
                        break;
                    case SLLI:
                        printf("SLLI \n");
                        break;
                    case SLTI:
                        printf("SLTI \n");
                        break;
                    case SLTIU:
                        printf("SLTIU \n");
                        break;
                    case XORI:
                        printf("XORI \n");
                        break;
                    case SRI:
                        switch (SRLI){
                            case SRLI:
                                printf("SRLI \n");
                                break;
                            case SRAI:
                                printf("SRAI \n");
                                break;
                            default:
                                break;
                        }
                    case ORI:
                        printf("ORI \n");
                        break;
                    case ANDI:
                        printf("ANDI \n");
                        break;   
                default:
                    break;
                }
                break;
            case(STORE):
                switch (funct3){
                    case SB:
                        printf("SB \n");
                        break;
                    case SH:
                        printf("SH \n");
                        break;
                    case SW:
                        printf("SW \n");
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
                        break;
                    case BNE:
                        printf("BNE \n");
                        break;
                    case BLT:
                        printf("BLT \n");
                        break;
                    case BGE:
                        printf("BGE \n");
                        break;
                    case BLTU:
                        printf("BLTU \n");
                        break;
                    case BGEU:
                        printf("BGEU \n");
                        break;
                    default:
                        break;
                }
            case(0X33):
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
                                    printf("REMU \n");
                                    break;
                                case SUB:
                                    printf("REMU \n");
                                    break;
                            default:
                                break;
                            }
                        case SLL:
                            printf("SLL \n");
                            break;
                        case SLT:
                            printf("SLT \n");
                            break;
                        case SLTU:
                            printf("SLTU \n");
                            break;
                        case XOR:
                            printf("XOR \n");
                            break;
                        case SRLA:
                            switch (funct7){
                                case SRL:
                                    printf("REMU \n");
                                    break;
                                case SRA:
                                    printf("REMU \n");
                                    break;
                            default:
                                break;
                            }
                        case OR:
                            printf("XOR \n");
                            break;
                        case AND:
                            printf("XOR \n");
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
