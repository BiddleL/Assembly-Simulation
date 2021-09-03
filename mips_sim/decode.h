#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>


// these defines are for the type of MIPS instruction
#define ADD 0x1
#define SUB 0x2
#define SLT 0x3
#define MUL 0x4
#define BEQ 0x5
#define BNE 0x6
#define ADDI 0x7
#define ORI 0x8
#define LUI 0x9
#define SYS 0xA
#define ERROR 404
#define WRONG_PLACE -1

// defines for registers
#define S$ 0x0
#define T$ 0x1
#define D$ 0x2
#define I$ 0x2
#define a0$ 0x4
#define v0$ 0x2

// defines for syscall 
#define SUCCESS 0x0
#define EXIT 0xA
#define INTEGER 0x1
#define CHAR 0xB



// this function is given the instruction and return what type of instruction it is
int find_instruction (uint32_t instruction);

// this function takes the instruction and returns an array with the values
// of each register: s = array[0] : t = array[1] : d or I = array[2]
int16_t *registers (uint32_t instruction);

// this function is used for printing what instruction it is
void printing_instruction_type (uint32_t instruction);

// this function prints the registers of an instruction
void printing_registers (uint32_t instruction);

// this function gives the value of what register must be set for the value
int register_to_set (uint32_t instruction);

// this program runs the operations for the instructions
// and returns the result of those operations
int32_t operations (uint32_t instruction, int32_t reg[]);

// this function does the printing for the MIPS syscall function
int syscall_print (uint32_t instruction, int32_t reg[]);

// this function prints for bne and beq
void print_equals (uint32_t instruction, int32_t reg[], int i);

// this function does the syscall prints for the -r function
int syscall_print_r (uint32_t instruction, int32_t reg[]);