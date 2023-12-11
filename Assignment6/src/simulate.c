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
        switch (opcode){
            case (OPIMM):
                switch (funct3){
                    case :
                        printf("ADDI \n");
                        break;
                    case 0x1:
                        printf("SLLI \n");
                        break;
                    case 0x2:
                        printf("SLTI \n");
                        break;
                    case 0x3:
                        printf("SLTIU \n");
                        break;
                    case 0x4:
                        printf("XORI \n");
                        break;  
                default:
                    break;
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
