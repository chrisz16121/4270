#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#include "mu-mips.h"

uint32_t do_instruction( uint32_t X, uint32_t Y, uint32_t opcode);

/***************************************************************/
/* Print out a list of commands available                                                                  */
/***************************************************************/
void help() {        
	printf("------------------------------------------------------------------\n\n");
	printf("\t**********MU-MIPS Help MENU**********\n\n");
	printf("sim\t-- simulate program to completion \n");
	printf("run <n>\t-- simulate program for <n> instructions\n");
	printf("rdump\t-- dump register values\n");
	printf("reset\t-- clears all registers/memory and re-loads the program\n");
	printf("input <reg> <val>\t-- set GPR <reg> to <val>\n");
	printf("mdump <start> <stop>\t-- dump memory from <start> to <stop> address\n");
	printf("high <val>\t-- set the HI register to <val>\n");
	printf("low <val>\t-- set the LO register to <val>\n");
	printf("print\t-- print the program loaded into memory\n");
	printf("show\t-- print the current content of the pipeline registers\n");
	printf("?\t-- display help menu\n");
	printf("quit\t-- exit the simulator\n\n");
	printf("------------------------------------------------------------------\n\n");
}

/***************************************************************/
/* Read a 32-bit word from memory                                                                            */
/***************************************************************/
uint32_t mem_read_32(uint32_t address)
{
	int i;
	for (i = 0; i < NUM_MEM_REGION; i++) {
		if ( (address >= MEM_REGIONS[i].begin) &&  ( address <= MEM_REGIONS[i].end) ) {
			uint32_t offset = address - MEM_REGIONS[i].begin;
			return (MEM_REGIONS[i].mem[offset+3] << 24) |
					(MEM_REGIONS[i].mem[offset+2] << 16) |
					(MEM_REGIONS[i].mem[offset+1] <<  8) |
					(MEM_REGIONS[i].mem[offset+0] <<  0);
		}
	}
	return 0;
}

/***************************************************************/
/* Write a 32-bit word to memory                                                                                */
/***************************************************************/
void mem_write_32(uint32_t address, uint32_t value)
{
	int i;
	uint32_t offset;
	for (i = 0; i < NUM_MEM_REGION; i++) {
		if ( (address >= MEM_REGIONS[i].begin) && (address <= MEM_REGIONS[i].end) ) {
			offset = address - MEM_REGIONS[i].begin;

			MEM_REGIONS[i].mem[offset+3] = (value >> 24) & 0xFF;
			MEM_REGIONS[i].mem[offset+2] = (value >> 16) & 0xFF;
			MEM_REGIONS[i].mem[offset+1] = (value >>  8) & 0xFF;
			MEM_REGIONS[i].mem[offset+0] = (value >>  0) & 0xFF;
		}
	}
}

/***************************************************************/
/* Execute one cycle                                                                                                              */
/***************************************************************/
void cycle() {                                                
	handle_pipeline();
	CURRENT_STATE = NEXT_STATE;
	CYCLE_COUNT++;
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
	while (RUN_FLAG){
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
	for (address = start; address <= stop; address += 4){
		printf("\t0x%08x (%d) :\t0x%08x\n", address, address, mem_read_32(address));
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
	printf("# Cycles Executed\t: %u\n", CYCLE_COUNT);
	printf("PC\t: 0x%08x\n", CURRENT_STATE.PC);
	printf("-------------------------------------\n");
	printf("[Register]\t[Value]\n");
	printf("-------------------------------------\n");
	for (i = 0; i < MIPS_REGS; i++){
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

	if (scanf("%s", buffer) == EOF){
		exit(0);
	}

	switch(buffer[0]) {
		case 'S':
		case 's':
			if (buffer[1] == 'h' || buffer[1] == 'H'){
				show_pipeline();
			}else {
				runAll(); 
			}
			break;
		case 'M':
		case 'm':
			if (scanf("%x %x", &start, &stop) != 2){
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
			if (buffer[1] == 'd' || buffer[1] == 'D'){
				rdump();
			}else if(buffer[1] == 'e' || buffer[1] == 'E'){
				reset();
			}
			else {
				if (scanf("%d", &cycles) != 1) {
					break;
				}
				run(cycles);
			}
			break;
		case 'I':
		case 'i':
			if (scanf("%u %i", &register_no, &register_value) != 2){
				break;
			}
			CURRENT_STATE.REGS[register_no] = register_value;
			NEXT_STATE.REGS[register_no] = register_value;
			break;
		case 'H':
		case 'h':
			if (scanf("%i", &hi_reg_value) != 1){
				break;
			}
			CURRENT_STATE.HI = hi_reg_value; 
			NEXT_STATE.HI = hi_reg_value; 
			break;
		case 'L':
		case 'l':
			if (scanf("%i", &lo_reg_value) != 1){
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
	for (i = 0; i < MIPS_REGS; i++){
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
	CURRENT_STATE.PC =  MEM_TEXT_BEGIN;
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
	while( fscanf(fp, "%x\n", &word) != EOF ) {
		address = MEM_TEXT_BEGIN + i;
		mem_write_32(address, word);
		printf("writing 0x%08x into address 0x%08x (%d)\n", word, address, address);
		i += 4;
	}
	PROGRAM_SIZE = i/4;
	printf("Program loaded into memory.\n%d words written into memory.\n\n", PROGRAM_SIZE);
	fclose(fp);
}

/************************************************************/
/* maintain the pipeline                                                                                           */ 
/************************************************************/
void handle_pipeline()
{
	/*INSTRUCTION_COUNT should be incremented when instruction is done*/
	/*Since we do not have branch/jump instructions, INSTRUCTION_COUNT should be incremented in WB stage */
	
	WB();
	MEM();
	EX();
	ID();
	IF();
}

/************************************************************/
/* writeback (WB) pipeline stage:                                                                          */ 
/************************************************************/
void WB()
{
	INSTRUCTION_COUNT++;
	//This is essentually pseudocode, if will not work!!
	if( 1 ){ /*ALU*/
		if( 1 ) {/*register-immediate*/
			NEXT_STATE.REGS[ MEM_WB.IR[rt] ] = MEM_WB.ALUOutput;//!!!Might be wrong!!!
		} else {/*register-register*/
			NEXT_STATE.REGS[ MEM_WB.IR[rd] ] = MEM_WB.ALUOutput;//!!!Might be wrong!!!
		}	
	}else { /*Load/Store*/
		if( 1 ){ /*Load*/
			NEXT_STATE.REGS[ MEM_WB.IR[rt] ] = MEM_WB.LMD;//!!!Might be wrong!!!
		}
	}
}

/************************************************************/
/* memory access (MEM) pipeline stage:                                                          */ 
/************************************************************/
void MEM()
{
	//This is essentually pseudocode, it will not work!!!
	uint32_t temp;
	if( 1 ){ /*ALU*/ 
		MEM_WB.IR = EX_MEM.IR;
		MEM_WB.ALUOutput = EX_MEM.ALUOutput;
	} else { /*Load/Store*/
		MEM_WB.IR = EX_MEM.IR;
		if( 1 ){ //Load
			MEM_WB.LMD = mem_read_32( EX_MEM.ALUOutput );
		} else { //Store
			temp = mem_read_32( EX_MEM.ALUOutput );
			mem_write_32( temp, EX_MEM.B );
		}
		EX_MEM.ALUOutput = ID_EX.A + ID_EX.imm;
		EX_MEM.B = ID_EX.B;
	}
}

/************************************************************/
/* execution (EX) pipeline stage:                                                                          */ 
/************************************************************/
void EX()
{
	//This is essentually pseudocode, it will not work!!!
	if( 1 ){ /*ALU*/ 
		EX_MEM.IR = ID_EX.IR;
		if( 1 ){ //register-immediate
			EX_MEM.ALUOutput = do_instruction(ID_EX.A,ID_EX.imm, opcode); 
		} else { //register-register
			EX_MEM.ALUOutput = do_instruction(ID_EX.A,ID_EX.B, opcode); 
		}
	} else { /*Load/Store*/
		EX_MEM.IR = ID_EX.IR;
		EX_MEM.ALUOutput = ID_EX.A + ID_EX.imm;
		EX_MEM.B = ID_EX.B;
	}
}

/************************************************************/
/* instruction decode (ID) pipeline stage:                                                         */ 
/************************************************************/
void ID()
{
	uint32_t rs, rt, immediate; 
	ID_EX.IR = IF_ID.IR;
	rs = 0x03E00000 & IF_ID.IR;
	rt = 0x001F0000 & IF_ID.IR;
	immediate = 0x0000FFFF & IF_ID.IR;
	immediate = 0xFFFFFFFF ^ immediate;
	ID_EX.A = CURRENT_STATE.REGS[ IF_ID.IR[rs] ]; //!!!Might be wrong!!!
	ID_EX.B = CURRENT_STATE.REGS[ IF_ID.IR[rt] ]; //!!!Might be wrong!!!
	ID_EX.imm = immediate;
}

/************************************************************/
/* instruction fetch (IF) pipeline stage:                                                              */ 
/************************************************************/
void IF()
{
	IF_ID.IR = CURRENT_STATE.PC;
	CURRENT_STATE.PC = CURRENT_STATE.PC + 4;
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

uint32_t do_instruction( uint32_t X, uint32_t Y, uint32_t opcode, uint32_t function){
	//This is where we will have a large case statement 
	//to determine what operation to do on X and Y
	uint32_t answer;
	if(opcode == 0x00){
			switch(function){
				case 0x00: //SLL
					answer = X << Y;
					break;
				case 0x02: //SRL
					answer = X >> Y;
					break;
				case 0x03: //SRA 
					if ((X & 0x80000000) == 1)
					{
						answer =  ~(~X >> Y );
					}
					else{
						answer = X >> Y;
					}
					break;
				case 0x18: //MULT
					if (X & 0x80000000) == 0x80000000){
						p1 = 0xFFFFFFFF00000000 | X;
					}else{
						p1 = 0x00000000FFFFFFFF & X;
					}
					if ((Y & 0x80000000) == 0x80000000){
						p2 = 0xFFFFFFFF00000000 | Y;
					}else{
						p2 = 0x00000000FFFFFFFF & Y;
					}
					product = p1 * p2;
					NEXT_STATE.LO = (product & 0X00000000FFFFFFFF);
					NEXT_STATE.HI = (product & 0XFFFFFFFF00000000)>>32;
					answer = 0x1;
					break;
				case 0x19: //MULTU
					product = (uint64_t)X * (uint64_t)Y;
					NEXT_STATE.LO = (product & 0X00000000FFFFFFFF);
					NEXT_STATE.HI = (product & 0XFFFFFFFF00000000)>>32;
					answer = 0x1;
					break;
				case 0x1A: //DIV 
					if(Y != 0)
					{
						NEXT_STATE.LO = (int32_t)X / (int32_t)Y;
						NEXT_STATE.HI = (int32_t)X % (int32_t)Y;
					}
					answer = 0x1;
					break;
				case 0x1B: //DIVU
					if(Y != 0)
					{
						NEXT_STATE.LO = X / Y;
						NEXT_STATE.HI = X % Y;
					}
					answer = 0x1;
					break;
				case 0x20: //ADD
					answer = X + Y;
					break;
				case 0x21: //ADDU 
					answer = X + Y;
					break;
				case 0x22: //SUB
					answer = X - Y;
					break;
				case 0x23: //SUBU
					answer = X - Y;
					break;
				case 0x24: //AND
					answer = X & Y;
					break;
				case 0x25: //OR
					answer = X | Y;
					break;
				case 0x26: //XOR
					answer = X ^ Y;
					break;
				case 0x27: //NOR
					answer = ~(X | Y);
					break;
				case 0x2A: //SLT
					if(X < Y){
						answer = 0x1;
					}
					else{
						answer = 0x0;
					} 
					break;
				default:
					printf("Instruction at 0x%x is not implemented!\n", CURRENT_STATE.PC);
					break;
			}
		}
		else{
			switch(opcode){
				case 0x08: //ADDI
					answer = X + ( (Y & 0x8000) > 0 ? (Y | 0xFFFF0000) : (Y & 0x0000FFFF));
					break;
				case 0x09: //ADDIU
					answer = X + ( (Y & 0x8000) > 0 ? (Y | 0xFFFF0000) : (Y & 0x0000FFFF));
					break;
				case 0x0A: //SLTI
					if ( (  (int32_t)X - (int32_t)( (Y & 0x8000) > 0 ? (Y | 0xFFFF0000) : (Y & 0x0000FFFF))) < 0){
						answer = 0x1;
					}else{
						answer = 0x0;
					}
					break;
				case 0x0C: //ANDI
					answer = X & (Y & 0x0000FFFF);
					break;
				case 0x0D: //ORI
					answer = X | (Y & 0x0000FFFF);
					break;
				case 0x0E: //XORI
					answer = X ^ (Y & 0x0000FFFF);
					break;
				default:
					// put more things here
					printf("Instruction at 0x%x is not implemented!\n", CURRENT_STATE.PC);
					break;
			}
		}
	return answer;
}
/************************************************************/
/* Print the program loaded into memory (in MIPS assembly format)    */ 
/************************************************************/
void print_program(){
	/*IMPLEMENT THIS*/
}

/************************************************************/
/* Print the current pipeline                                                                                    */ 
/************************************************************/
void show_pipeline(){
	/*IMPLEMENT THIS*/
	printf("Current PC: %x\n", CURRENT_STATE.PC);
	printf("IF/ID.IR: %x\n", instruct);
	printf("IF/ID.PC: %x\n", IF_ID.PC);

	printf("ID/EX.IR: %x\n", ID_EX.IR);
	printf("ID/EX.A: %x\n", ID_EX.A);
	printf("ID/EX.B: %x\n", ID_EX.B);
	printf("ID/EX.imm: %x\n", ID_EX.imm);

	printf("EX/MEM.IR: %x\n", EX_MEM.IR);
	printf("EX/MEM.A: %x\n", EX_MEM.A);
	printf("EX/MEM.B: %x\n", EX_MEM.B);
	printf("EX/MEM.ALUOutput: %x\n", EX_MEM.ALUOutput);

	printf("MEM/WB.IR: %x\n", MEM_WB.IR);
	printf("MEM/WB.ALUOutout: %x\n", MEM_WB.ALUOutput);
	printf("MEM/WB.LMD: %x\n", MEM_WB.LMD);
}

/***************************************************************/
/* main                                                                                                                                   */
/***************************************************************/
int main(int argc, char *argv[]) {                              
	printf("\n**************************\n");
	printf("Welcome to MU-MIPS SIM...\n");
	printf("**************************\n\n");
	
	if (argc < 2) {
		printf("Error: You should provide input file.\nUsage: %s <input program> \n\n",  argv[0]);
		exit(1);
	}

	strcpy(prog_file, argv[1]);
	initialize();
	load_program();
	help();
	while (1){
		handle_command();
	}
	return 0;
}
