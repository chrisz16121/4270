#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#include "mu-mips.h"

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
	/*IMPLEMENT THIS*/
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
			EX_MEM.ALUOutput = do_instruction(ID_EX.A,ID_EX.imm, operation); 
		} else { //register-register
			EX_MEM.ALUOutput = do_instruction(ID_EX.A,ID_EX.B, operation); 
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
	parse_instruction();	//Parse the IF_ID.IR
	uint32_t rs, rt, immediate; 
	ID_EX.IR = IF_ID.IR;
	rs = 0x03E00000 & IF_ID.IR;
	rt = 0x001F0000 & IF_ID.IR;
	immediate = 0x0000FFFF & IF_ID.IR;
	immediate = 0xFFFFFFFF ^ immediate;
	ID_EX.A = CURRENT_STATE.REGS[rs];
	ID_EX.B = CURRENT_STATE.REGS[rt];
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
/* parse instruction. This is from working simulator from Akturk, but modified         */ 
/************************************************************/
void parse_instruction()
{
	/*IMPLEMENT THIS*/
	/* execute one instruction at a time. Use/update CURRENT_STATE and and NEXT_STATE, as necessary.*/
	
	
	uint32_t instruction, opcode, function, rs, rt, rd, sa, immediate, target;
	uint64_t product, p1, p2;
	
	uint32_t addr, data;
	
	int branch_jump = FALSE;
	
	printf("[0x%x]\t", CURRENT_STATE.PC);
	
	instruction = mem_read_32(IF_ID.IR);
	
	opcode = (instruction & 0xFC000000) >> 26;
	function = instruction & 0x0000003F;
	rs = (instruction & 0x03E00000) >> 21;
	rt = (instruction & 0x001F0000) >> 16;
	rd = (instruction & 0x0000F800) >> 11;
	sa = (instruction & 0x000007C0) >> 6;
	immediate = instruction & 0x0000FFFF;
	target = instruction & 0x03FFFFFF;
	
	
	//If ALU Instruction, then assign RT, RS to A and B | REG TO REG need RD | REG TO MEM don't need RD
	

	
	//used to assign register values, not to execute logic
	if(opcode == 0x00){
		switch(function){
			case 0x00: //SLL --ALU

				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x02: //SRL --ALU

				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x03: //SRA --ALU
				
				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x08: //JR
				
				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x09: //JALR
				
				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x0C: //SYSCALL
			
				break;
			case 0x10: //MFHI --Load/Store

				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x11: //MTHI --Load/Store

				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x12: //MFLO --Load/Store

				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x13: //MTLO --Load/Store

				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x18: //MULT --ALU
			
				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x19: //MULTU --ALU
				
				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x1A: //DIV --ALU
				
				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x1B: //DIVU --ALU
			
				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x20: //ADD --ALU
				
				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x21: //ADDU --ALU

				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x22: //SUB --ALU

				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x23: //SUBU --ALU

				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x24: //AND --ALU

				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x25: //OR --ALU

				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x26: //XOR --ALU

				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x27: //NOR --ALU

				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x2A: //SLT --ALU
			
				print_instruction(CURRENT_STATE.PC);
				break;
			default:
				printf("Instruction at 0x%x is not implemented!\n", CURRENT_STATE.PC);
				break;
		}
	}
	else{
		switch(opcode){
			case 0x01:
				if(rt == 0x00000){ //BLTZ
					if((CURRENT_STATE.REGS[rs] & 0x80000000) > 0){
						NEXT_STATE.PC = CURRENT_STATE.PC + ( (immediate & 0x8000) > 0 ? (immediate | 0xFFFF0000)<<2 : (immediate & 0x0000FFFF)<<2);
						branch_jump = TRUE;
					}
					print_instruction(CURRENT_STATE.PC);
				}
				else if(rt == 0x00001){ //BGEZ
					if((CURRENT_STATE.REGS[rs] & 0x80000000) == 0x0){
						NEXT_STATE.PC = CURRENT_STATE.PC + ( (immediate & 0x8000) > 0 ? (immediate | 0xFFFF0000)<<2 : (immediate & 0x0000FFFF)<<2);
						branch_jump = TRUE;
					}
					print_instruction(CURRENT_STATE.PC);
				}
				break;
			case 0x02: //J
				
				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x03: //JAL
				
				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x04: //BEQ
				
				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x05: //BNE
				
				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x06: //BLEZ
			
				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x07: //BGTZ
			
				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x08: //ADDI --ALU

				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x09: //ADDIU --ALU

				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x0A: //SLTI --ALU
				
				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x0C: //ANDI --ALU

				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x0D: //ORI --ALU

				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x0E: //XORI --ALU

				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x0F: //LUI --Load/Store

				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x20: //LB --Load/Store
				
				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x21: //LH --Load/Store
		
				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x23: //LW --Load/Store

				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x28: //SB --Load/Store
			
				print_instruction(CURRENT_STATE.PC);				
				break;
			case 0x29: //SH --Load/Store
				
				print_instruction(CURRENT_STATE.PC);
				break;
			case 0x2B: //SW --Load/Store
		
				print_instruction(CURRENT_STATE.PC);
				break;
			default:
				// put more things here
				printf("Instruction at 0x%x is not implemented!\n", CURRENT_STATE.PC);
				break;
		}
	}
	
	if(!branch_jump){
		NEXT_STATE.PC = CURRENT_STATE.PC + 4;
	}
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
	/*IMPLEMENT THIS*/
}

/************************************************************/
/* Print the current pipeline                                                                                    */ 
/************************************************************/
void show_pipeline(){
	/*IMPLEMENT THIS*/
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
