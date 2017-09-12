#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#include "mu-mips.h"

uint32_t createMask(uint32_t,uint32_t);
/***************************************************************/
/* Read a 32-bit word from memory                                                                            */
/***************************************************************/
uint32_t mem_read_32(uint32_t address) {
	int i;
	for (i = 0; i < NUM_MEM_REGION; i++) {
		if ((address >= MEM_REGIONS[i].begin)
				&& (address <= MEM_REGIONS[i].end)) {
			uint32_t offset = address - MEM_REGIONS[i].begin;
			return (MEM_REGIONS[i].mem[offset + 3] << 24)
					| (MEM_REGIONS[i].mem[offset + 2] << 16)
					| (MEM_REGIONS[i].mem[offset + 1] << 8)
					| (MEM_REGIONS[i].mem[offset + 0] << 0);
		}
	}
	return 0;
}

/***************************************************************/
/* Write a 32-bit word to memory                                                                                */
/***************************************************************/
void mem_write_32(uint32_t address, uint32_t value) {
	int i;
	uint32_t offset;
	for (i = 0; i < NUM_MEM_REGION; i++) {
		if ((address >= MEM_REGIONS[i].begin)
				&& (address <= MEM_REGIONS[i].end)) {
			offset = address - MEM_REGIONS[i].begin;

			MEM_REGIONS[i].mem[offset + 3] = (value >> 24) & 0xFF;
			MEM_REGIONS[i].mem[offset + 2] = (value >> 16) & 0xFF;
			MEM_REGIONS[i].mem[offset + 1] = (value >> 8) & 0xFF;
			MEM_REGIONS[i].mem[offset + 0] = (value >> 0) & 0xFF;
		}
	}
}



/***************************************************************/
/* Dump a word-aligned region of memory to the terminal                              */
/***************************************************************/
void mdump(uint32_t start, uint32_t stop) {
	uint32_t address;

	printf("-------------------------------------------------------------\n");
	printf("Memory content [0x%08x..0x%08x] :\n", start, stop);
	printf("-------------------------------------------------------------\n");
	printf("\t[Address in Hex (Dec) ]\t[Value]\n");
	for (address = start; address <= stop; address += 4) {
		printf("\t0x%08x (%d) :\t0x%08x\n", address, address,
				mem_read_32(address));
	}
	printf("\n");
}

/***************************************************************/
/* Dump current values of registers to the teminal                                              */
/***************************************************************/
void rdump() {
	int i;
	printf("-------------------------------------\n");
	printf("Dumping Register Content\n");
	printf("-------------------------------------\n");
	printf("# Instructions Executed\t: %u\n", INSTRUCTION_COUNT);
	printf("PC\t: 0x%08x\n", CURRENT_STATE.PC);
	printf("-------------------------------------\n");
	printf("[Register]\t[Value]\n");
	printf("-------------------------------------\n");
	for (i = 0; i < MIPS_REGS; i++) {
		printf("[R%d]\t: 0x%08x\n", i, CURRENT_STATE.REGS[i]);
	}
	printf("-------------------------------------\n");
	printf("[HI]\t: 0x%08x\n", CURRENT_STATE.HI);
	printf("[LO]\t: 0x%08x\n", CURRENT_STATE.LO);
	printf("-------------------------------------\n");
}


/***************************************************************/
/* Allocate and set memory to zero                                                                            */
/***************************************************************/
void init_memory() {
	int i;
	for (i = 0; i < NUM_MEM_REGION; i++) {
		uint32_t region_size = MEM_REGIONS[i].end - MEM_REGIONS[i].begin + 1;
		MEM_REGIONS[i].mem = malloc(region_size);
		memset(MEM_REGIONS[i].mem, 0, region_size);
	}
}

uint32_t createMask(uint32_t a, uint32_t b) { //a needs to be smaller than b
	uint32_t r = 0;
	for (int32_t i = a; i <= b; i++) {
		r |= 1 << i;
	}
	return r;
}

unint32_t translateInstruction( char *instruction ){
	char *instruct, *val1, *val2, *val3;
	uint32_t intVal1, intVal2, intVal3;

	sscanf(instruction, "%s %s, %s, %s", instruct, val1, val2, val3);
	switch(*instruct){ //do if elses
		case "addi": //ADDI
			
			break;
		case "addiu": //ADDIU
			
			break;
		case "andi": //ANDI
			
		  	break;
		case "beq": //BEQ
			
			break;
		case "bne": //BNE
			
			break;
		case "ori": //ORI
			
		  break;
		case "bgtz": //BGTZ
			
			break;
		case "blez": //BLEZ
			
			break;
		case "xori": //XORI
			
			break;
		case "slti": //SLTI
			break;
		case "j": //J
			
			break;
		case "jal": //JAL
			
			break;
		case "lb": //LB
			break;
		case "lh": //LH
			break;
		case "lui": //LUI
			break;
		case "lw": //LW
			break;
		case "sb": //SB
			break;
		case "sh": //SH
			break;
		case "sw": //SW
			break;
		case "bltz": //BLTZ
			
			break;
		case "bgez": // BGEZ
			break;
		//
		//START OF "SPECIAL" OPCODE INSTRUCTIONS
		//
		case "add": //ADD
			
			break;
		case "addu": //ADDU
			
			break;
		case "and": //AND
			
			break;
		case "sub": //SUB
			
			break;		
		case "subu": //SUBU
			
			break;
		case "mult": //MULT
			
			break;
		case "multu": //MULTU
			
			break;
		case "div": //DIV
			
			break;	
		case "divu": //DIVU
			
			break;
		case "or": //OR
			
			break;
		case "xor": //XOR
			
			break;
		case "nor": //NOR
			
			break;
		case "slt": //SLT
			break;
		case "sll": //SLL !!!It is supposed to be all zeroes!!! Logical means add 0's
			
			break;
		case "sra": //SRA Arithmetic means 
		
			break;
		case "srl": //SRL DOUBLE CHECK RESULT BC IT MAY INSERT 1's INSTEAD OF 0's
			
			break;
		case "jalr": //JALR
			break;		
		case "jr": //JR
			break;
		case "mfhi": //MFHI
			break;
		case "mflo": //MFLO
			break;
		case "mthi": //MTHI
			break;	
		case "mtlo": //MTLO
			break;
		case "syscall": 
		
			break;
		default: 
			printf("\n\nInstruction Not Found\n\n");
			break;
	}
}

/* main**************************************************/
int main(int argc, char *argv[]) {
	printf("\n**************************\n");
	printf("Welcome to MU-MIPS Assembler...\n");
	printf("**************************\n\n");

	if (argc < 2) {
		printf(
				"Error: You should provide input file.\nUsage: %s <input program> \n\n",
				argv[0]);
		exit(1);
	}

	strcpy(prog_file, argv[1]);
	// initialize();
	// load_program();
	// help();
	// while (1) {
	// 	handle_command();
	// }
	return 0;
}

