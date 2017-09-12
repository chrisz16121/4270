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
	//printf("Memory content [0x%08x..0x%08x] \n", start, stop);
	printf("-------------------------------------------------------------\n");
	printf("\t[Address in Hex (Dec) ]\t[Value]\n");
	for (address = start; address <= stop; address += 4) {
		//printf("\t0x%08x (%d) \t0x%08x\n", address, address,
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
//	printf("# Instructions Executed\t %u\n", INSTRUCTION_COUNT);
//	printf("PC\t 0x%08x\n", CURRENT_STATE.PC);
	printf("-------------------------------------\n");
	printf("[Register]\t[Value]\n");
	printf("-------------------------------------\n");
	for (i = 0; i < MIPS_REGS; i++) {
//		printf("[R%d]\t 0x%08x\n", i, CURRENT_STATE.REGS[i]);
	}
	printf("-------------------------------------\n");
//	printf("[HI]\t 0x%08x\n", CURRENT_STATE.HI);
//	printf("[LO]\t 0x%08x\n", CURRENT_STATE.LO);
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
	uint32_t intVal1, intVal2, intVal3, mchnCode;

	sscanf(instruction, "%s %s, %s, %s", instruct, val1, val2, val3);

	if( strcmp( *instruct,  "addi") == 0 ){ //ADDI
		
	} else if( strcmp( *instruct,  "addiu") == 0 ){ //ADDIU
		
	} else if( strcmp( *instruct,  "andi") == 0 ){ //ANDI
		
	} else if( strcmp( *instruct,  "beq") == 0 ){ //BEQ
		
	} else if( strcmp( *instruct,  "bne") == 0 ){ //BNE
		
	} else if( strcmp( *instruct,  "ori") == 0 ){ //ORI
		
	} else if( strcmp( *instruct,  "bgtz") == 0 ){ //BGTZ
		
	} else if( strcmp( *instruct,  "blez") == 0 ){ //BLEZ
		
	} else if( strcmp( *instruct,  "xori") == 0 ){ //XORI
		
	} else if( strcmp( *instruct,  "slti") == 0 ){ //SLTI
	
	} else if( strcmp( *instruct,  "j") == 0 ){ //J
		
	} else if( strcmp( *instruct,  "jal") == 0 ){ //JAL
		
	} else if( strcmp( *instruct,  "lb") == 0 ){ //LB
		
	} else if( strcmp( *instruct,  "lh") == 0 ){ //LH
		
	} else if( strcmp( *instruct,  "lui") == 0 ){ //LUI
		
	} else if( strcmp( *instruct,  "lw") == 0 ){ //LW
		
	} else if( strcmp( *instruct,  "sb") == 0 ){ //SB
	
	} else if( strcmp( *instruct,  "sh") == 0 ){ //SH
	
	} else if( strcmp( *instruct,  "sw") == 0 ){ //SW
		
	} else if( strcmp( *instruct,  "bltz") == 0 ){ //BLTZ
		
	} else if( strcmp( *instruct,  "bgez") == 0 ){ // BGEZ
				
	//START OF "SPECIAL" OPCODE INSTRUCTIONS
	} else if( strcmp( *instruct,  "add") == 0 ){ //ADD
		
	} else if( strcmp( *instruct,  "addu") == 0 ){ //ADDU
		
	} else if( strcmp( *instruct,  "and") == 0 ){ //AND
		
	} else if( strcmp( *instruct,  "sub") == 0 ){ //SUB
		
	} else if( strcmp( *instruct,  "subu") == 0 ){ //SUBU
		
	} else if( strcmp( *instruct,  "mult") == 0 ){ //MULT
		
	} else if( strcmp( *instruct,  "multu") == 0 ){ //MULTU
		
	} else if( strcmp( *instruct,  "div") == 0 ){ //DIV
		
	} else if( strcmp( *instruct,  "divu") == 0 ){ //DIVU
		
	} else if( strcmp( *instruct,  "or") == 0 ){ //OR
		
	} else if( strcmp( *instruct,  "xor") == 0 ){ //XOR
		
	} else if( strcmp( *instruct,  "nor") == 0 ){ //NOR
		
	} else if( strcmp( *instruct,  "slt") == 0 ){ //SLT
	
	} else if( strcmp( *instruct,  "sll") == 0 ){ //SLL !!!It is supposed to be all zeroes!!! Logical means add 0's
		
	} else if( strcmp( *instruct,  "sra") == 0 ){ //SRA Arithmetic means 
	
	} else if( strcmp( *instruct,  "srl") == 0 ){ //SRL DOUBLE CHECK RESULT BC IT MAY INSERT 1's INSTEAD OF 0's
		
	} else if( strcmp( *instruct,  "jalr") == 0 ){ //JALR
		
	} else if( strcmp( *instruct,  "jr") == 0 ){ //JR
		
	} else if( strcmp( *instruct,  "mfhi") == 0 ){ //MFHI
		
	} else if( strcmp( *instruct,  "mflo") == 0 ){ //MFLO
		
	} else if( strcmp( *instruct,  "mthi") == 0 ){ //MTHI
		
	} else if( strcmp( *instruct,  "mtlo") == 0 ){ //MTLO
		
	} else if( strcmp( *instruct,  "syscall") == 0 ){ 
	
	} else{ 
		printf("\n\nInstruction Not Found\n\n");
	} 
	return mchnCode;
}

/* main**************************************************/
int main(int argc, char *argv[]) {
	printf("\n**************************\n");
	printf("Welcome to MU-MIPS Assembler...\n");
	printf("**************************\n\n");

	if (argc < 2) {
		printf(
				"Error) == 0 ){ You should provide input file.\nUsage) == 0 ){ %s <input program> \n\n",
				argv[0]);
		exit(1);
	}

	strcpy(prog_file, argv[1]);

	//while( line of input file != EOF ) {
		//prase individual line from input file
		//send line to translateInstruction
			//return translated machine code
		//print line of machine code into output file
	//}
	//save output file

	return 0;
}

