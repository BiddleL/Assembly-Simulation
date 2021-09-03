#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>

#include "decode.h"

// this function is given the instruction and return what type of instruction it is
int find_instruction (uint32_t instruction) {
    uint32_t first = instruction & ((uint32_t)0x3F << 26);
    if (first == 0) {
        uint32_t second = instruction & 0x3FF;
        if (second == 0x20) {
            return ADD;
        } else if (second == 0x22) {
            return SUB;
        } else if (second == 0x2A) {
            return SLT; // TODO doesn't SLT must fix
        } else if (instruction == 0xC) {
           return SYS;
        } else return ERROR;
    } else {
        first = first >> 26;
        if (first == 0x1C) {
            return MUL;
        } else if (first == 0x4) {
            return BEQ;
        } else if (first == 0x5) {
            return BNE;
        } else if (first == 0x8) {
            return ADDI;
        } else if (first == 0xD) {
            return ORI;
        } else if (first == 0xF) {
            return LUI;
        } else {
            return ERROR;
        }
    }
}

// this function is used for printing what instruction it is
void printing_instruction_type (uint32_t instruction) {
    int type = find_instruction(instruction);
    if(type == ADD) printf("add ");
    else if(type == SUB) printf("sub ");
    else if(type == SLT) printf("slt ");
    else if(type == MUL) printf("mul ");
    else if(type == BEQ) printf("beq ");
    else if(type == BNE) printf("bne ");
    else if(type == ADDI) printf("addi ");
    else if(type == ORI) printf("ori ");
    else if(type == LUI) printf("lui ");
    else if(type == SYS) printf("syscall ");
    else printf("ERROR ");
}

// this function takes the instruction and returns an array with the values
// of each register: s = array[0] : t = array[1] : d or I = array[2]
int16_t *registers (uint32_t instruction) {
    int type = find_instruction(instruction);
    uint32_t s_register = (instruction >> 21) & ((uint32_t)0x1F);
    uint32_t t_register = (instruction >> 16) & ((uint32_t)0x1F);
    uint32_t d_register = (instruction >> 11) & ((uint32_t)0x1F);
    uint32_t i_register = instruction & 0xFFFF;

    int16_t *pointer = malloc(3*sizeof(int16_t));
    pointer[0] = (uint16_t)s_register;
    pointer[1] = (uint16_t)t_register;
    if(type >= ADD && type <= MUL) {
       pointer[2] = (int16_t)d_register;
   } else if (type >= BEQ && type <= LUI) { 
        pointer[2] = (int16_t)i_register;
   }
   return pointer;
}

// this function prints the registers of an instruction
void printing_registers (uint32_t instruction) {
    int type = find_instruction(instruction);
    int16_t *register_values = registers(instruction);

    if (type >= ADD && type <= MUL) {
        printf(" $%d, $%d, $%d\n", register_values[2], register_values[0], register_values[1]);
    } else if (type == BEQ || type == BNE) {
        printf(" $%d, $%d, %d\n", register_values[0], register_values[1], (int16_t)register_values[2]);
    } else if (type == ADDI || type == ORI) {
        if (type == ORI) printf(" "); // fixs the spacing 
        printf("$%d, $%d, %d\n", register_values[1], register_values[0], (int16_t)register_values[2]);
    } else if (type == LUI) {
        printf(" $%d, %d\n", register_values[1], (int16_t)register_values[2]);
    } else if (type == SYS) {
        printf("\n");
    } else printf("ERROR\n");
    free(register_values);
}

// this function gives the value of what register must be set for the value
// can't use this function if the instruction 
int register_to_set (uint32_t instruction) {
    int type = find_instruction(instruction);
    int16_t *register_values = registers(instruction);
    int value_1 = (int)register_values[1];
    int value_2 = (int)register_values[2];
    free(register_values);
    if (type >= ADD && type <= MUL) {
        return value_2;
    } else if (type >= ADDI && type <= LUI) {
        return value_1;
    } else if (type == BEQ || type == BNE) {
        return 0;
    } else if (type == SYS) return ERROR;  
    else return ERROR;  
}

// this program runs the operations for the instructions
// and returns the result of those operations
int32_t operations (uint32_t instruction, int32_t reg[]) {
    int type = find_instruction(instruction);
    int16_t *register_values = registers(instruction);
    int32_t value = 0;

    if (type == ADD) {
        value = reg[register_values[0]] + reg[register_values[1]];
    } else if (type == SUB) {
        value = reg[register_values[0]] - reg[register_values[1]];
    } else if (type == SLT) {
        if (reg[register_values[0]] < reg[register_values[1]]) {
            value = 1;
        } else value = 0;
    } else if (type == MUL) {
        value = reg[register_values[0]] * reg[register_values[1]];
    } else if (type == ADDI) {
        value = reg[register_values[0]] + register_values[2];
    } else if (type == ORI) {
        value = reg[register_values[0]] | register_values[2];
    } else if (type == LUI) {
        value = register_values[2] << 16;
    } else if (type == BEQ) {
        if(reg[register_values[0]] == reg[register_values[1]]) {
            value = register_values[2] - 1;
        } else {
            value = 0;
        }
    } else if (type == BNE) {
        if(reg[register_values[0]] != reg[register_values[1]]) {
            value = register_values[2] - 1;
        } else {
            value = 0;
        }
    }
    free(register_values);
    return value;
}

// this function does the printing for the MIPS syscall function
int syscall_print (uint32_t instruction, int32_t reg[]) {    
    printf(">>> syscall %d\n", reg[2]);

    if (reg[2] == INTEGER) {
        printf("<<< %d\n", reg[4]);
        return SUCCESS;
    } else if (reg[2] == CHAR) {
        printf("<<< %c\n", reg[4]);
        return SUCCESS;
    } else if (reg[2] == EXIT) {
        return ERROR;
    } else {
        printf("Unknown system call: %d\n", reg[2]);
        return ERROR;
    } 
}

// this function prints for bne and beq 
void print_equals (uint32_t instruction, int32_t regist[], int i) {
    int type = find_instruction(instruction);
    int16_t *regs = registers(instruction);
    int counter = 0;
    if (type == BEQ) {
        if (regist[regs[0]] == regist[regs[1]]) {
            counter = i + regs[2];
            printf(">>> branch taken to PC = %d\n", counter);
        } else {
            printf(">>> branch not taken\n");
        }
    } else if (type == BNE) {
        if (regist[regs[0]] != regist[regs[1]]) {
            counter = i + regs[2];
            printf(">>> branch taken to PC = %d\n", counter);
        } else{
            printf(">>> branch not taken\n");
        }
    } else printf("ERROR\n");
    free(regs);
}

// this function does the syscall prints for the -r function
int syscall_print_r (uint32_t instruction, int32_t reg[]) {    
    if (reg[2] == INTEGER) {
        printf("%d", reg[4]);
        return SUCCESS;
    } else if (reg[2] == CHAR) {
        printf("%c", reg[4]);
        return SUCCESS;
    } else if (reg[2] == EXIT) {
        return ERROR;
    } else {
        printf("Unknown system call: %d\n", reg[2]);
        return ERROR;
    } 
}




/*int main (void) {
    int32_t reg[32] = {0};
    reg[8] = 9;
    int pc = 8;
    uint32_t test = 0x1500FFF9;
    printf("%d\n", operations(test, reg) + 1);
    print_equals (test, reg, pc);
    
    
    


    return 0;
}*/

// this function changes pc and deals with bne and beq
// this function was moved into the operations function
// therefore isn't used anymore
/*int movement_equal (uint32_t instruction, int32_t reg[], int i) {
    int type = find_instruction(instruction);
    int16_t *regs = registers(instruction);
    int counter = 0;
    if (type == BEQ) {
        if (regs[0] == regs[1]) {
            counter = i + 1;
        } else {
            counter = i + regs[2];
        }
    } else if (type == BNE) {
        if (regs[0] != regs[1]) {
            counter = i + 1;
        } else {
            counter = i + regs[2];
        }
    }
    ;
    return counter;
} */


