#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#include "mu-mips.h"


/***************************************************************/
int z = 1;
/* Print out a list of commands available                                                                  */
void help() {
	printf(
			"------------------------------------------------------------------\n\n");
	printf("\t**********MU-MIPS Help MENU**********\n\n");
	printf("sim\t-- simulate program to completion \n");
	printf("run <n>\t-- simulate program for <n> instructions\n");
	printf("rdump\t-- dump register values\n");
	printf("reset\t-- clears all registers/memory and re-loads the program\n");
	printf("input <reg> <val>\t-- set GPR <reg> to <val>\n");
	printf(
			"mdump <start> <stop>\t-- dump memory from <start> to <stop> address\n");
	printf("high <val>\t-- set the HI register to <val>\n");
	printf("low <val>\t-- set the LO register to <val>\n");
	printf("print\t-- print the program loaded into memory\n");
	printf("?\t-- display help menu\n");
	printf("quit\t-- exit the simulator\n\n");
	printf(
			"------------------------------------------------------------------\n\n");
}
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
/* Execute one cycle                                                                                                              */
/***************************************************************/
void cycle() {
	handle_instruction();
	CURRENT_STATE = NEXT_STATE;
	//printf("%d:\tREGISTERS:\n$zero:%d\t$v0:%d\t$a0:%d\t$a1:%d\t$a2:%d\t$a3:%d\t$t0:%d\tt1:%d\t\n",z,CURRENT_STATE.REGS[0],CURRENT_STATE.REGS[2],CURRENT_STATE.REGS[4],CURRENT_STATE.REGS[5],CURRENT_STATE.REGS[6],CURRENT_STATE.REGS[7],CURRENT_STATE.REGS[8],CURRENT_STATE.REGS[9]);
	INSTRUCTION_COUNT++;
	z++;
}

/***************************************************************/
/* Simulate MIPS for n cycles                                                                                       */
/***************************************************************/
void run(int num_cycles) {

	if (RUN_FLAG == FALSE) {
		printf("Simulation Stopped\n\n");
		return;
	}

	printf("Running simulator for %d cycles...\n\n", num_cycles);
	int i;
	for (i = 0; i < num_cycles; i++) {
		if (RUN_FLAG == FALSE) {
			printf("Simulation Stopped.\n\n");
			break;
		}
		cycle();
	}
}

/***************************************************************/
/* simulate to completion                                                                                               */
/***************************************************************/
void runAll() {
	if (RUN_FLAG == FALSE) {
		printf("Simulation Stopped.\n\n");
		return;
	}

	printf("Simulation Started...\n\n");
	while (RUN_FLAG) {
		cycle();
	}
	printf("Simulation Finished.\n\n");
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
/* Read a command from standard input.                                                               */
/***************************************************************/
void handle_command() {
	char buffer[20];
	uint32_t start, stop, cycles;
	uint32_t register_no;
	int register_value;
	int hi_reg_value, lo_reg_value;

	printf("MU-MIPS SIM:> ");

	if (scanf("%s", buffer) == EOF) {
		exit(0);
	}

	switch (buffer[0]) {
	case 'S':
	case 's':
		runAll();
		break;
	case 'M':
	case 'm':
		if (scanf("%x %x", &start, &stop) != 2) {
			break;
		}
		mdump(start, stop);
		break;
	case '?':
		help();
		break;
	case 'Q':
	case 'q':
		printf("**************************\n");
		printf("Exiting MU-MIPS! Good Bye...\n");
		printf("**************************\n");
		exit(0);
	case 'R':
	case 'r':
		if (buffer[1] == 'd' || buffer[1] == 'D') {
			rdump();
		} else if (buffer[1] == 'e' || buffer[1] == 'E') {
			reset();
		} else {
			if (scanf("%d", &cycles) != 1) {
				break;
			}
			run(cycles);
		}
		break;
	case 'I':
	case 'i':
		if (scanf("%u %i", &register_no, &register_value) != 2) {
			break;
		}
		CURRENT_STATE.REGS[register_no] = register_value;
		NEXT_STATE.REGS[register_no] = register_value;
		break;
	case 'H':
	case 'h':
		if (scanf("%i", &hi_reg_value) != 1) {
			break;
		}
		CURRENT_STATE.HI = hi_reg_value;
		NEXT_STATE.HI = hi_reg_value;
		break;
	case 'L':
	case 'l':
		if (scanf("%i", &lo_reg_value) != 1) {
			break;
		}
		CURRENT_STATE.LO = lo_reg_value;
		NEXT_STATE.LO = lo_reg_value;
		break;
	case 'P':
	case 'p':
		print_program();
		break;
	default:
		printf("Invalid Command.\n");
		break;
	}
}

/***************************************************************/
/* reset registers/memory and reload program                                                    */
/***************************************************************/
void reset() {
	int i;
	/*reset registers*/
	for (i = 0; i < MIPS_REGS; i++) {
		CURRENT_STATE.REGS[i] = 0;
	}
	CURRENT_STATE.HI = 0;
	CURRENT_STATE.LO = 0;

	for (i = 0; i < NUM_MEM_REGION; i++) {
		uint32_t region_size = MEM_REGIONS[i].end - MEM_REGIONS[i].begin + 1;
		memset(MEM_REGIONS[i].mem, 0, region_size);
	}

	/*load program*/
	load_program();

	/*reset PC*/
	INSTRUCTION_COUNT = 0;
	CURRENT_STATE.PC = MEM_TEXT_BEGIN;
	NEXT_STATE = CURRENT_STATE;
	RUN_FLAG = TRUE;
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

/**************************************************************/
/* load program into memory                                                                                      */
/**************************************************************/
void load_program() {
	FILE * fp;
	int i, word;
	uint32_t address;

	/* Open program file. */
	fp = fopen(prog_file, "r");
	if (fp == NULL) {
		printf("Error: Can't open program file %s\n", prog_file);
		exit(-1);
	}

	/* Read in the program. */

	i = 0;
	while (fscanf(fp, "%x\n", &word) != EOF) {
		address = MEM_TEXT_BEGIN + i;
		mem_write_32(address, word);
		printf("writing 0x%08x into address 0x%08x (%d)\n", word, address,
				address);
		i += 4;
	}
	PROGRAM_SIZE = i / 4;
	printf("Program loaded into memory.\n%d words written into memory.\n\n",
			PROGRAM_SIZE);
	fclose(fp);
}
uint32_t createMask(uint32_t a, uint32_t b) { //a needs to be smaller than b
	uint32_t r = 0;
	for (int32_t i = a; i <= b; i++) {
		r |= 1 << i;
	}
	return r;
}
/************************************************************/
/* decode and execute instruction                                                                     */
/************************************************************/
void handle_instruction()
{
	//print_program();
	int i;
	int binary_num[32];
	uint32_t temp_instruct;
	uint32_t instruct = mem_read_32(CURRENT_STATE.PC);
	uint32_t opcode,rs,rt,rd,result,remainder;
	uint64_t double_result;
	uint32_t delay_slot_instruct,target_address,offset;
	uint32_t immediate;
	uint32_t check,special,mask,a,b,sa,base,virtualAddress,temp;
	opcode = 0xFC000000 & instruct;
	printf("Instruction fetched: %x\n",instruct);
	temp_instruct = instruct;
	/*for(i = 0;i < 32;i++){
		int temp = 1 & temp_instruct;
		//printf("%d",temp);
		temp_instruct >> 1;
		binary_num[31-i] = temp;
	}
	for(i = 0;i < 32; i ++){
		printf("%d",binary_num[i]);
	}
	*/
	//printf("\n");
	//printf("\n");
	//begin else ifs 
	
	switch(opcode){
		case 0x20000000: //ADDI
			rs = (0x03E00000 & instruct) >> 21;
			rt = (0x001F0000 & instruct) >> 16;
			immediate = (0x0000FFFF & instruct);
			result = CURRENT_STATE.REGS[rs] + (int32_t)immediate;
			NEXT_STATE.REGS[rt] = result;
			break;
		case 0x24000000: //ADDIU
			rs = (0x03E00000 & instruct) >> 21;
			rt = (0x001F0000 & instruct) >> 16;
			immediate = (0x0000FFFF & instruct);
			result = CURRENT_STATE.REGS[rs] + immediate;
			NEXT_STATE.REGS[rt] = result;
			//printf("register %d now contains %d:%d\n",rt,result,NEXT_STATE.REGS[2]);
			break;
			case 0x30000000: //ANDI--Corrected?
			immediate = (0x0000FFFF & instruct);
	    	rs = (0x03E00000 & instruct) >> 21;
		  	rt = (0x001F0000 & instruct) >> 16;
		  	NEXT_STATE.REGS[rt] = immediate & CURRENT_STATE.REGS[rs]; // need to store value of rt into address
		  	break;
		case 0x10000000: //BEQ--Corrected?
			offset = (0x0000FFFF & instruct) << 2;
			rs = (0x03E00000 & instruct) >> 21;
			rt = (0x001F0000 & instruct) >> 16;
			if(CURRENT_STATE.REGS[rs] == CURRENT_STATE.REGS[rt]){
				NEXT_STATE.PC = CURRENT_STATE.PC + offset;
			}
			break;
		case 0x14000000: //BNE--Corrected?
			offset = (0x0000FFFF & instruct) << 2;
			offset = offset & 0x0000FFFF;
			offset = offset ^ 0xFFFF0000;
			rs = (0x03E00000 & instruct) >> 21;
			rt = (0x001F0000 & instruct) >> 16;
			if(CURRENT_STATE.REGS[rs] != CURRENT_STATE.REGS[rt]){
				//printf("taking branch! offset is %08x\n",offset);
				NEXT_STATE.PC = CURRENT_STATE.PC + offset;
				//printf("Current Instruction: %08x\n",CURRENT_STATE.PC);
				//printf("Next Instruction: %08x\n",NEXT_STATE.PC);
			}
			break;
		case 0x34000000: //ORI--Corrected?
			immediate = (0x0000FFFF & instruct);
			rs = (0x03E00000 & instruct) >> 21;
		 	rt = immediate | rs;
		  	NEXT_STATE.REGS[rt] = immediate | CURRENT_STATE.REGS[rs]; // store value of rt into address
		  	break;
		case 0x1C000000: //BGTZ--Corrected?
			offset = (0x0000FFFF & instruct) << 2;
			rs = (0x03E00000 & instruct) >> 21;
			rt = (0x001F0000 & instruct) >> 16;
			if(CURRENT_STATE.REGS[rs] > 0){
				NEXT_STATE.PC = CURRENT_STATE.PC + offset;
			}
			break;
		case 0x18000000: //BLEZ--Corrected?
			offset = (0x0000FFFF & instruct) << 2;
			delay_slot_instruct = mem_read_32(CURRENT_STATE.PC + 4);
			target_address = delay_slot_instruct + offset;
			rs = (0x03E00000 & instruct) >> 21;
			if(CURRENT_STATE.REGS[rs] <= 0){
				CURRENT_STATE.PC += offset;
			}
			break;
		case 0x38000000: //XORI
			rs = (0x03E00000 & instruct) >> 21;
	  		rt = (0x001F0000 & instruct) >> 16;
		  	rd = (0x00007C00 & instruct) >> 11;
		 	a = CURRENT_STATE.REGS[rs] & CURRENT_STATE.REGS[rt];
		  	b = ~CURRENT_STATE.REGS[rs] & ~CURRENT_STATE.REGS[rt];
		  	NEXT_STATE.REGS[rd] = ~a & ~b; //~ gets bitwise complement
			break;
		case 0x28000000: //SLTI
			rs = (0x03E00000 & instruct) >> 21;
			rt = (0x001F0000 & instruct) >> 16;
			immediate = (0x0000FFFF & instruct);
			if (CURRENT_STATE.REGS[rs] < immediate) {
				NEXT_STATE.REGS[rt] = 1;
			} else {
				NEXT_STATE.REGS[rt] = 0;
			}
			break;
		case 0x08000000: //J
			target_address = (0x03FFFFFF & instruct);
			mask = createMask(28,31);
			a = (CURRENT_STATE.PC & mask);
			NEXT_STATE.PC = (a | (target_address << 2)) & 0xFFFFFFFC;  
			break;
		case 0x0C000000: //JAL
			target_address = (0x03FFFFFF & instruct);
			mask = createMask(28,31);
			a = (CURRENT_STATE.PC & mask);
			NEXT_STATE.PC = (a | (target_address << 2)) & 0xFFFFFFFC;
			NEXT_STATE.REGS[31] = CURRENT_STATE.PC + 8;
			break;
		case 0x80000000: //LB*
			//only loading a byte, so do we need to get rid of the lower or upper 24 bits?
			offset = (0x0000FFFF & instruct);
			base = (0x03E00000 & instruct) >> 21;
			rt = (0x001F0000 & instruct) >> 16;
			virtualAddress = offset + base;
			//NEXT_STATE.REGS[rt] = CURRENT_STATE.REGS[virtualAddress] & 0x000000FF;
			//NEXT_STATE.REGS[rt] = mem_read_32(virtualAddress);
			NEXT_STATE.REGS[rt] = (mem_read_32(virtualAddress) & 0x000000FF);
			break;
		case 0x84000000: //LH*	
			//same question for the halfword
			offset = (0x0000FFFF & instruct);
			base = (0x03E00000 & instruct) >> 21;
			rt = (0x001F0000 & instruct) >> 16;
			virtualAddress = offset + base;
			//NEXT_STATE.REGS[rt] = CURRENT_STATE.REGS[virtualAddress] & 0x0000FFFF;
			//NEXT_STATE.REGS[rt] = mem_read_32(virtualAddress);
			NEXT_STATE.REGS[rt] = (mem_read_32(virtualAddress) & 0x0000FFFF);
			break;
		case 0x3C000000: //LUI
			immediate = (0x0000FFFF & instruct);
			rt = (0x001F0000 & instruct) >> 16;
			temp = (immediate << 16) | (0x0000);
			NEXT_STATE.REGS[rt] = temp;
			break;
		case 0x8C000000: //LW*
			//no bit masking here, read function returns a word
			offset = (0x0000FFFF & instruct);
			if(offset > 0x1000) break; // overflow condition
			base = (0x03E00000 & instruct) >> 21;
			rt = (0x001F0000 & instruct) >> 16;
			virtualAddress = offset + base;
			//NEXT_STATE.REGS[rt] = CURRENT_STATE.REGS[virtualAddress];
			NEXT_STATE.REGS[rt] = mem_read_32(virtualAddress);
			break;
		case 0xA0000000: //SB*
			//this one needs bit masking..
			offset = (0x0000FFFF & instruct);
			base = (0x03E00000 & instruct) >> 21;
			rt = (0x001F0000 & instruct) >> 16;
			virtualAddress = offset + base;
			//NEXT_STATE.REGS[virtualAddress] = CURRENT_STATE.REGS[rt] & 0x000000FF;
			mem_write_32(virtualAddress,(CURRENT_STATE.REGS[rt] & 0x000000FF));
			break;
		case 0xA4000000: //SH*
			//another one with bit masking
			offset = (0x0000FFFF & instruct);
			base = (0x03E00000 & instruct) >> 21;
			rt = (0x001F0000 & instruct) >> 16;
			virtualAddress = offset + base;
			if ((0x00000001 & virtualAddress) != 0) {
				printf("This instruction doesn't work");
			} else {
				//NEXT_STATE.REGS[virtualAddress] = CURRENT_STATE.REGS[rt] & 0x0000FFFF;
				mem_write_32(virtualAddress,(CURRENT_STATE.REGS[rt] & 0x0000FFFF));
			}
			break;
		case 0xAC000000: //SW
			offset = (0x0000FFFF & instruct);
			base = (0x03E00000 & instruct) >> 21;
			rt = (0x001F0000 & instruct) >> 16;
			if ((0x00000001 & instruct) != 0 || (0x00000002 & instruct) != 0) {
				printf("This instruction doesn't work");
			} else {
				virtualAddress = offset + base;
				//NEXT_STATE.REGS[virtualAddress] = CURRENT_STATE.REGS[rt];
				mem_write_32(virtualAddress,CURRENT_STATE.REGS[rt]);
			}
			break;
		case 0x04000000: //BLTZ, BGEZ CHECK THIS
			mask = createMask(16, 20);
			temp = CURRENT_STATE.PC + 4;
			rs = (0x03E00000 & instruct) >> 21;
			uint32_t check = instruct & mask;
			if (check == 0x00000000) {
				if(CURRENT_STATE.REGS[rs] == 1){
					NEXT_STATE.PC = (offset + temp) << 2; //BLTZ
				}else{
				
				}
			} else {
				if(CURRENT_STATE.REGS[rs] == 0){
					NEXT_STATE.PC = (offset + temp) << 2; //BGEZ
				}else{
					
				}
			}
			break;
			case 0x00000000: //Special
			mask = createMask(0,5);
				special = instruct & mask;
			switch( special ){
				case 0x00000020: //ADD
					rs = (0x03E00000 & instruct) >> 21;
					rt = (0x001F0000 & instruct) >> 16;
					rd = (0x00007C00 & instruct) >> 11;
					result = CURRENT_STATE.REGS[rs] + CURRENT_STATE.REGS[rt];
					NEXT_STATE.REGS[rd] = result;
					break;
				case 0x00000021: //ADDU
					rs = (0x03E00000 & instruct) >> 21;
					rt = (0x001F0000 & instruct) >> 16;
					rd = (0x0000F800 & instruct) >> 11;
					result = CURRENT_STATE.REGS[rs] + CURRENT_STATE.REGS[rt];
					NEXT_STATE.REGS[rd] = result;
					//printf("%d + %d was assigned to %d\n",rs,rt,rd);
					break;
				case 0x00000024: //AND
					immediate = (0x0000FFFF & instruct);
					rs = (0x03E00000 & instruct) >> 21;
					rt = (0x001F0000 & instruct) >> 16;
					NEXT_STATE.REGS[rt] = CURRENT_STATE.REGS[immediate] & CURRENT_STATE.REGS[rs];
					break;
				case 0x00000022: //SUB
					rs = (0x03E00000 & instruct) >> 21;
					rt = (0x001F0000 & instruct) >> 16;
					rd = (0x00007C00 & instruct) >> 11;
					result = CURRENT_STATE.REGS[rs] - CURRENT_STATE.REGS[rt];
					NEXT_STATE.REGS[rd] = result;
					break;		
				case 0x00000023: //SUBU
					rs = (0x03E00000 & instruct) >> 21;
					rt = (0x001F0000 & instruct) >> 16;
					rd = (0x00007C00 & instruct) >> 11;
					result = CURRENT_STATE.REGS[rs] - CURRENT_STATE.REGS[rt];
					NEXT_STATE.REGS[rd] = result;
					break;
				case 0x00000018: //MULT
					rs = (0x03E00000 & instruct) >> 21;
					rt = (0x001F0000 & instruct) >> 16;
					double_result = CURRENT_STATE.REGS[rs] * CURRENT_STATE.REGS[rt];
					NEXT_STATE.HI = (uint32_t)((double_result & 0xFFFFFFFF00000000) >> 32);
					NEXT_STATE.LO = (uint32_t)(double_result & 0x00000000FFFFFFFF);
					//implement 2's complement
					break;
				case 0x00000019: //MULTU
					rs = (0x03E00000 & instruct) >> 21;
					rt = (0x001F0000 & instruct) >> 16;
					double_result = CURRENT_STATE.REGS[rs] * CURRENT_STATE.REGS[rt];
					NEXT_STATE.HI = (uint32_t)((double_result & 0xFFFFFFFF00000000) >> 32);
					NEXT_STATE.LO = (uint32_t)(double_result & 0x00000000FFFFFFFF);
					break;
				case 0x0000001A: //DIV
					rs = (0x03E00000 & instruct) >> 21;
					rt = (0x001F0000 & instruct) >> 16;
					result = CURRENT_STATE.REGS[rs] / CURRENT_STATE.REGS[rt];
					remainder = CURRENT_STATE.REGS[rs] % CURRENT_STATE.REGS[rt];
					NEXT_STATE.HI = remainder;
					NEXT_STATE.LO = result;
					//implement 2's complement
					break;	
				case 0x0000001B: //DIVU
					rs = (0x03E00000 & instruct) >> 21;
					rt = (0x001F0000 & instruct) >> 16;
					result = CURRENT_STATE.REGS[rs] / CURRENT_STATE.REGS[rt];
					remainder = CURRENT_STATE.REGS[rs] % CURRENT_STATE.REGS[rt];
					NEXT_STATE.HI = remainder;
					NEXT_STATE.LO = result;
					break;
    				case 0x00000025: //OR
				  	rs = (0x03E00000 & instruct) >> 21;
					rt = (0x001F0000 & instruct) >> 16;
					rd = (0x00007C00 & instruct) >> 11;
					NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] | CURRENT_STATE.REGS[rt];
					break;
				case 0x00000026: //XOR
					rs = (0x03E00000 & instruct) >> 21;
					rt = (0x001F0000 & instruct) >> 16;
					rd = (0x00007C00 & instruct) >> 11;
					a = CURRENT_STATE.REGS[rs] & CURRENT_STATE.REGS[rt];
					b = ~CURRENT_STATE.REGS[rs] & ~CURRENT_STATE.REGS[rt];
					NEXT_STATE.REGS[rd] = ~a & ~b; //~ gets bitwise complement
					break;
				case 0x00000027: //NOR
					rs = (0x03E00000 & instruct) >> 21;
					rt = (0x001F0000 & instruct) >> 16;
					rd = (0x00007C00 & instruct) >> 11;
					NEXT_STATE.REGS[rd] = ~(CURRENT_STATE.REGS[rs] | CURRENT_STATE.REGS[rt]);
					break;
				case 0x0000002A: //SLT
					rs = (0x03E00000 & instruct) >> 21;
					rt = (0x001F0000 & instruct) >> 16;
					rd = (0x00007C00 & instruct) >> 11;
					if (CURRENT_STATE.REGS[rs] < CURRENT_STATE.REGS[rt]) {
						NEXT_STATE.REGS[rd] = 1;
					} else {
					NEXT_STATE.REGS[rd] = 0;
					}
					break;
				case 0x00000000: //SLL !!!It is supposed to be all zeroes!!! Logical means add 0's
					rt = (0x001F0000 & instruct) >> 16;
					sa = (0x000007C0 & instruct) >> 6;
					rd = (0x00007C00 & instruct) >> 11;
					NEXT_STATE.REGS[rd] = (CURRENT_STATE.REGS[rt] << CURRENT_STATE.REGS[sa]) & CURRENT_STATE.REGS[sa];
					break;
				case 0x00000003: //SRA Arithmetic means 
					rt = (0x001F0000 & instruct) >> 16;
					sa = (0x000007C0 & instruct) >> 6;
					rd = (0x00007C00 & instruct) >> 11;
					NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rt] >> CURRENT_STATE.REGS[sa];//I think this will add 1's, but not sure how to specify
					break;
				case 0x00000002: //SRL DOUBLE CHECK RESULT BC IT MAY INSERT 1's INSTEAD OF 0's
					rt = (0x001F0000 & instruct) >> 16;
					sa = (0x000007C0 & instruct) >> 6;
					rd = (0x00007C00 & instruct) >> 11;
					NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rt]
					>> CURRENT_STATE.REGS[sa]; // I think this adds 1's and idk how to specify to add 0's
					break;
				case 0x00000009: //JALR
					rs = (0x03E00000 & instruct) >> 21;
					rd = (0x00007C00 & instruct) >> 11;
					temp_instruct = CURRENT_STATE.REGS[rs];
					CURRENT_STATE.REGS[rs] = CURRENT_STATE.PC + 8;
					NEXT_STATE.PC = temp_instruct;
					break;		
				case 0x00000008: //JR
					rs = (0x03E00000 & instruct) >> 21;
					NEXT_STATE.PC = CURRENT_STATE.REGS[rs];
					break;
				case 0x00000010: //MFHI
					rd = (0x00007C00 & instruct) >> 11;
					NEXT_STATE.REGS[rd] = CURRENT_STATE.HI;
					break;
				case 0x00000012: //MFLO
					rd = (0x00007C00 & instruct) >> 11;
					NEXT_STATE.REGS[rd] = CURRENT_STATE.LO;
					break;
				case 0x00000011: //MTHI
					rs = (0x03E00000 & instruct) >> 21;
					NEXT_STATE.HI = CURRENT_STATE.REGS[rs];
					break;	
				case 0x00000013: //MTLO
					rs = (0x03E00000 & instruct) >> 21;
					NEXT_STATE.LO = CURRENT_STATE.REGS[rs];
					break;
				case 0x0000000C: 
					if(CURRENT_STATE.REGS[2] == 0xA){
						printf("We made it to the end!\nProgram has finished executing, exiting simulator.\n");
						exit(1);
					}
					else if(CURRENT_STATE.REGS[2] == 0x5){
						int dummy;
						printf("Please enter an integer\n");
						scanf("%d",&dummy);
						NEXT_STATE.REGS[2] = dummy;
					}
					else if(CURRENT_STATE.REGS[2] == 0x1){
						int dummy = CURRENT_STATE.REGS[4];
						printf("Program Result: %d\n",dummy);
					}
					break;
				default:
					printf("\n\nInstruction Not Found\n\n");
			}
			break;
		default: 
			printf("\n\nInstruction Not Found\n\n");
			break;
	}
	int dummy = 0;
	//scanf("%d",&dummy);
	/*IMPLEMENT THIS*/
	/* execute one instruction at a time. Use/update CURRENT_STATE and and NEXT_STATE, as necessary.*/
	NEXT_STATE.PC += 4;
	//Bump NEXT_STATE
}

/************************************************************/
/* Initialize Memory                                                                                                    */
/************************************************************/
void initialize() {
	init_memory();
	CURRENT_STATE.PC = MEM_TEXT_BEGIN;
	NEXT_STATE = CURRENT_STATE;
	RUN_FLAG = TRUE;
}

/************************************************************/
/* Print the program loaded into memory (in MIPS assembly format)    */
/************************************************************/

void print_program(){
	int i;
	uint32_t instruct = mem_read_32(CURRENT_STATE.PC);
	uint32_t mask = createMask(26,32);
	uint32_t opcode = instruct & mask;
	uint32_t rs,rt,rd,base,sa;	
	uint32_t special,check;
	uint32_t immediate,offset,target;
	/*opcode = opcode >> 25;
	for(i = 0;i < 6;i++){
		int temp = 1 & opcode;
		printf("%d",temp);
	}
	printf("\n");
	*/
	switch (opcode) {
		case 0x20000000: //ADDI
			rs = (0x03E00000 & instruct) >> 21;
			rt = (0x001F0000 & instruct) >> 16;
			immediate = (0x0000FFFF & instruct);
			printf("ADDI %d, %d, %d\n", rs, rt, immediate);
			break;
		case 0x24000000: //ADDIU
			rs = (0x03E00000 & instruct) >> 21;
			rt = (0x001F0000 & instruct) >> 16;
			immediate = (0x0000FFFF & instruct);
			printf("ADDIU %d, %d, %d\n", rs, rt, immediate);
			break;
		case 0x30000000: //ANDI
			rs = (0x03E00000 & instruct) >> 21;
			rt = (0x001F0000 & instruct) >> 16;
			immediate = (0x0000FFFF & instruct);
			printf("ANDI %d, %d, %d\n", rs, rt, immediate);
			break;
		case 0x10000000: //BEQ
			rs = (0x03E00000 & instruct) >> 21;
			rt = (0x001F0000 & instruct) >> 16;
			offset = (0x0000FFFF & instruct);
			printf("BEQ %d, %d, %d\n", rs, rt, offset);
			break;
		case 0x14000000: //BNE
			rs = (0x03E00000 & instruct) >> 21;
			rt = (0x001F0000 & instruct) >> 16;
			offset = (0x0000FFFF & instruct);
			printf("BNE %d, %d, %d\n", rs, rt, offset);
			break;
		case 0x34000000: //ORI
			rs = (0x03E00000 & instruct) >> 21;
			rt = (0x001F0000 & instruct) >> 16;	
			immediate = (0x0000FFFF & instruct);
			printf("ORI %d, %d, %d\n", rs, rt, immediate);
			break;
		case 0x1C000000: //BGTZ
			rs = (0x03E00000 & instruct) >> 21;
			offset = (0x0000FFFF & instruct);
			printf("BGTZ %d, %d\n", rs, offset);
			break;
		case 0x18000000: //BLEZ
			rs = (0x03E00000 & instruct) >> 21;
			offset = (0x0000FFFF & instruct);
			printf("BLEZ %d, %d\n", rs, offset);
			break;
		case 0x38000000: //XORI
			rs = (0x03E00000 & instruct) >> 21;
			rt = (0x001F0000 & instruct) >> 16;
			immediate = (0x0000FFFF & instruct);
			printf("XORI %d, %d, %d\n", rt, rs, immediate);
			break;
		case 0x28000000: //SLTI
			rs = (0x03E00000 & instruct) >> 21;
			rt = (0x001F0000 & instruct) >> 16;
			immediate = (0x0000FFFF & instruct);
			printf("SLTI %d, %d, %d\n", rt, rs, immediate);
			break;
		case 0x08000000: //J
			target = (0x03FFFFFF & instruct);
			printf("J %d\n", target);
			break;
		case 0x0C000000: //JAL
			target = (0x03FFFFFF & instruct);
			printf("JAL %d\n", target);
			break;
		case 0x80000000: //LB
			base = (0x03E00000 & instruct) >> 21;
			rt = (0x001F0000 & instruct) >> 16;
			offset = (0x0000FFFF & instruct);
			printf("LB %d, %d(%d)\n", rt, offset, base);
			break;
		case 0x84000000: //LH
			base = (0x03E00000 & instruct) >> 21;
			rt = (0x001F0000 & instruct) >> 16;
			offset = (0x0000FFFF & instruct);
			printf("LH %d, %d(%d)\n", rt, offset, base);
			break;
		case 0x3C000000: //LUI
			rt = (0x001F0000 & instruct) >> 16;
			immediate = (0x0000FFFF & instruct);
			printf("LUI %d, %d\n", rt, immediate);
			break;
		case 0x8C000000: //LW
			base = (0x03E00000 & instruct) >> 21;
			rt = (0x001F0000 & instruct) >> 16;
			offset = (0x0000FFFF & instruct);
			printf("LW %d, %d(%d)\n", rt, offset, base);
			break;
		case 0xA0000000: //SB
			base = (0x03E00000 & instruct) >> 21;
			rt = (0x001F0000 & instruct) >> 16;
			offset = (0x0000FFFF & instruct);
			printf("SB %d, %d(%d)\n", rt, offset, base);
			break;
		case 0xA4000000: //SH
			base = (0x03E00000 & instruct) >> 21;
			rt = (0x001F0000 & instruct) >> 16;
			offset = (0x0000FFFF & instruct);
			printf("SH %d, %d(%d)\n", rt, offset, base);
			break;
		case 0xAC000000: //SW
			base = (0x03E00000 & instruct) >> 21;
			rt = (0x001F0000 & instruct) >> 16;
			offset = (0x0000FFFF & instruct);
			printf("SH %d, %d(%d)\n", rt, offset, base);
			break;
		case 0x04000000: //BLTZ, BGEZ
			mask = createMask(16, 20);
			check = instruct & mask;
			if (check == 0x00000000) {
				//BLTZ
				rs = (0x03E00000 & instruct) >> 21;
				offset = (0x0000FFFF & instruct);
				printf("BLTZ %d, %d\n", rs, offset);
			}
			else{
				//BGEZ
				rs = (0x03E00000 & instruct) >> 21;
				offset = (0x0000FFFF & instruct);
				printf("BGEZ %d, %d\n", rs, offset);
			}
			break;
		case 0x00000000: //Special
			mask = createMask(0, 5);
			special = instruct & mask;
		switch (special) {
			case 0x00000020: //ADD
				rs = (0x03E00000 & instruct) >> 21;
				rt = (0x001F0000 & instruct) >> 16;
				rd = (0x00007C00 & instruct) >> 11;
				printf("ADD %d, %d, %d\n", rd, rs, rt);
				break;
			case 0x00000021: //ADDU
				rs = (0x03E00000 & instruct) >> 21;
				rt = (0x001F0000 & instruct) >> 16;
				rd = (0x00007C00 & instruct) >> 11;
				printf("ADDU %d, %d, %d\n", rd, rs, rt);
				break;
			case 0x00000024: //AND
				rs = (0x03E00000 & instruct) >> 21;
				rt = (0x001F0000 & instruct) >> 16;
				rd = (0x00007C00 & instruct) >> 11;
				printf("AND %d, %d, %d\n", rd, rs, rt);
				break;
			case 0x00000022: //SUB
				rs = (0x03E00000 & instruct) >> 21;
				rt = (0x001F0000 & instruct) >> 16;
				rd = (0x00007C00 & instruct) >> 11;
				printf("SUB %d, %d, %d\n", rd, rs, rt);
				break;
			case 0x00000023: //SUBU
				rs = (0x03E00000 & instruct) >> 21;
				rt = (0x001F0000 & instruct) >> 16;
				rd = (0x00007C00 & instruct) >> 11;
				printf("SUBU %d, %d, %d\n", rd, rs, rt);
				break;
			case 0x00000018: //MULT
				rs = (0x03E00000 & instruct) >> 21;
				rt = (0x001F0000 & instruct) >> 16;
				printf("MULT %d, %d\n", rs, rt);
				break;
			case 0x00000019: //MULTU
				rs = (0x03E00000 & instruct) >> 21;
				rt = (0x001F0000 & instruct) >> 16;
				printf("MULTU %d, %d\n", rs, rt);
				break;
			case 0x0000001A: //DIV
				rs = (0x03E00000 & instruct) >> 21;
				rt = (0x001F0000 & instruct) >> 16;
				printf("DIVU %d, %d\n", rs, rt);
				break;
			case 0x0000001B: //DIVU
				rs = (0x03E00000 & instruct) >> 21;
				rt = (0x001F0000 & instruct) >> 16;
				printf("DIVU %d, %d\n", rs, rt);
				break;
			case 0x00000025: //OR
				rs = (0x03E00000 & instruct) >> 21;	
				rt = (0x001F0000 & instruct) >> 16;
				rd = (0x00007C00 & instruct) >> 11;
				printf("OR %d, %d, %d\n", rd, rs, rt);
				break;
			case 0x00000026: //XOR
				rs = (0x03E00000 & instruct) >> 21;
				rt = (0x001F0000 & instruct) >> 16;
				rd = (0x00007C00 & instruct) >> 11;
				printf("XOR %d, %d, %d\n", rd, rs, rt);
				break;
			case 0x00000027: //NOR
				rs = (0x03E00000 & instruct) >> 21;
				rt = (0x001F0000 & instruct) >> 16;
				rd = (0x00007C00 & instruct) >> 11;
				printf("NOR %d, %d, %d\n", rd, rs, rt);
				break;
			case 0x0000002A: //SLT
				rs = (0x03E00000 & instruct) >> 21;
				rt = (0x001F0000 & instruct) >> 16;
				rd = (0x00007C00 & instruct) >> 11;
				printf("NOR %d, %d, %d\n", rd, rs, rt);
				break;
			case 0x00000000: //SLL !!!It is supposed to be all zeroes!!!
				sa = (0x000003E0 & instruct) >> 6;
				rt = (0x001F0000 & instruct) >> 16;
				rd = (0x00007C00 & instruct) >> 11;
				printf("SLL %d, %d, %d\n", rd, rt, sa);
				break;
			case 0x00000003: //SRA
				sa = (0x000003E0 & instruct) >> 6;
				rt = (0x001F0000 & instruct) >> 16;
				rd = (0x00007C00 & instruct) >> 11;	
				printf("SRA %d, %d, %d\n", rd, rt, sa);
				break;
			case 0x00000002: //SRL
				sa = (0x000003E0 & instruct) >> 6;
				rt = (0x001F0000 & instruct) >> 16;
				rd = (0x00007C00 & instruct) >> 11;
				printf("SRL %d, %d, %d\n", rd, rt, sa);
				break;
			case 0x00000009: //JALR
				rs = (0x03E00000 & instruct) >> 21;
				rd = (0x00007C00 & instruct) >> 11;
				printf("JALR %d\n", rs);
				printf("JALR %d, %d\n", rd, rs);
				break;
			case 0x00000008: //JR
				rs = (0x03E00000 & instruct) >> 21;
				printf("JR %d\n", rs);
				break;
			case 0x00000010: //MFHI
				rd = (0x00007C00 & instruct) >> 11;
				printf("MFHI %d\n", rd);
				break;
			case 0x00000012: //MFLO
				rd = (0x00007C00 & instruct) >> 11;
				printf("MFLO %d\n", rd);
				break;
			case 0x00000011: //MTHI
				rs = (0x03E00000 & instruct) >> 21;
				printf("MTHI %d\n", rs);
				break;
			case 0x00000013: //MTLO
				rs = (0x03E00000 & instruct) >> 21;
				printf("MTHI %d\n", rs);
				break;
			case 0x0000000C: 
				if(CURRENT_STATE.REGS[2] == 0xA){
					printf("SYSCALL instruction, exiting program\n");
				}
				//printf("We out this thaaaang\n");
				//NEXT_STATE.REGS[2] = 0xA;
				break;
			default:
				printf("\n\nInstruction Not Found\n\n");
			}
			break;
		default:
			printf("\n\nInstruction Not Found\n\n");
			break;
	}	

	/*IMPLEMENT THIS*/
}
/* main**************************************************/
int main(int argc, char *argv[]) {
	printf("\n**************************\n");
	printf("Welcome to MU-MIPS SIM...\n");
	printf("**************************\n\n");
	printf("Test_5");
	if (argc < 2) {
		printf(
				"Error: You should provide input file.\nUsage: %s <input program> \n\n",
				argv[0]);
		exit(1);
	}
	printf("Test_1");
	strcpy(prog_file, argv[1]);
	initialize();
	printf("Test_2");
	load_program();
	printf("Test_3");
	help();
	printf("Test_4");
	while (1) {
		handle_command();
	}
	return 0;
}

