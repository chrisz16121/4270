#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#include "mu-mips.h"


int cycle_count = 0;

int FF = 0;
int instruction_fetch_flag = 0;

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
	printf("forwarding <n>\t-- enable(<n>=1)/disable(<n>=2) forwarding (disabled by default)\n");
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
	//if(KillFlag == 1){
	//	CURRENT_STATE = NEXT_STATE;
	//}
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
		case 'f':
			if(scanf("%d",&ENABLE_FORWARDING)!=1){
				break;
			}
			ENABLE_FORWARDING==0?printf("Forwarding OFF\n"):printf("Forwarding ON\n");
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
	uint32_t rd,rt;
	if(cycle_count <4){		//DO nothing
		printf("WB is NULL, cycle %d\n",cycle_count);
	}
	else if( fetch_flag == 1 && count == 3 ){
		printf("killing...\n");
		RUN_FLAG = FALSE;
	}
	else{
		//uint32_t instruction = MEM_WB.IR
		printf("WB: ");
		print_instruction(MEM_WB.PC);
		INSTRUCTION_COUNT++;
		uint32_t destination = MEM_WB.dest;
		printf("destination of WB is: %d\n",destination);
		if(MEM_WB.type == 1){/*register-immediate*/
			NEXT_STATE.REGS[MEM_WB.dest] = MEM_WB.ALUOutput;
		} 
		else if(MEM_WB.type == 0) {/*register-register*/
			//rd = (MEM_WB.IR & 0x0000F800) >> 11;
			NEXT_STATE.REGS[MEM_WB.dest] = MEM_WB.ALUOutput;
		}	
		else if(MEM_WB.type == 2){ /*Load*/
			NEXT_STATE.REGS[MEM_WB.B] = MEM_WB.LMD;
		}
		if(FF == 1){
			if(ID_EX.type == 0 && ((destination == ID_EX.rt) || (destination == ID_EX.rs))){
				printf("Hazard eliminated\n");
				FF = 0;
				fetch_flag = 0;
			}
			else if(ID_EX.type == 1 && destination == ID_EX.rs){
				printf("Hazard eliminated\n");
				FF = 0;
				fetch_flag = 0;
			}
			else if(ID_EX.type == 2 && destination == ID_EX.rs){
				printf("Hazard eliminated\n");
				FF = 0;
				fetch_flag = 0;
			}
			else if(ID_EX.type == 3 && destination == ID_EX.rt){
				printf("Hazard eliminated\n");
				FF = 0;
				fetch_flag = 0;
			}
		}
	}
	
}

/************************************************************/
/* memory access (MEM) pipeline stage:                                                          */ 
/************************************************************/
void MEM()
{
	uint32_t instruction,rs,rt,rd;
	if(cycle_count <3){		//DO nothing
		printf("MEM is NULL, cycle %d\n",cycle_count);
	}
	else{
		MEM_WB = EX_MEM;
		printf("MEM: ");
		print_instruction(MEM_WB.PC);	

		if(MEM_WB.type == 0 || MEM_WB.type == 1){
			MEM_WB.ALUOutput = MEM_WB.ALUOutput;	//redundant
		}
		else{ /*Load/Store*/
			if(MEM_WB.type == 2){ //Load
				MEM_WB.LMD = mem_read_32(MEM_WB.ALUOutput);
				if(FF == 1 && ENABLE_FORWARDING == 1){
					uint32_t destination = MEM_WB.dest;
					if(ID_EX.type == 0 && ((destination == ID_EX.rt) || (destination == ID_EX.rs))){
						if(destination == ID_EX.rt){
							ID_EX.B = MEM_WB.LMD;
						}
						else{
							ID_EX.A = MEM_WB.LMD;
						}
						printf("Hazard eliminated in MEM\n");
						FF = 0;
						fetch_flag = 0;
					}
					else if(ID_EX.type == 1 && destination == ID_EX.rs){
						ID_EX.A = MEM_WB.LMD;
						printf("Hazard eliminated in MEM\n");
						FF = 0;
						fetch_flag = 0;
					}
					else if(ID_EX.type == 2 && destination == ID_EX.rs){
						ID_EX.A = MEM_WB.LMD;
						printf("Hazard eliminated in MEM\n");
						FF = 0;
						fetch_flag = 0;
					}
					else if(ID_EX.type == 3 && destination == ID_EX.rt){
						ID_EX.B = MEM_WB.LMD;
						printf("Hazard eliminated in MEM\n");
						FF = 0;
						fetch_flag = 0;
					}
				}
			} 
			else if(EX_MEM.type == 3) { //Store
				mem_write_32(MEM_WB.ALUOutput,MEM_WB.B);
			}
		}
	}
	instruction = MEM_WB.IR;
	rs = (instruction & 0x03E00000) >> 21;
	rt = (instruction & 0x001F0000) >> 16;
	rd = (instruction & 0x0000F800) >> 11;
	
	/*if( ENABLE_FORWARDING == 1 && MEM_WB.type == 2){
		if((MEM_WB.regWrite == 1) && (MEM_WB.dest != 0) && !((EX_MEM.regWrite && (EX_MEM.dest != 0)) && (EX_MEM.dest == ID_EX.rs) && (MEM_WB.dest == ID_EX.rs))){
			ID_EX.A = MEM_WB.A;
			printf("MEM Forwarding\n");
			printf("Hazard eliminated\n");
			FF = 0;
			fetch_flag = 0;
		}
		if((MEM_WB.regWrite == 1) && (MEM_WB.dest != 0) && !((EX_MEM.regWrite && (EX_MEM.dest != 0)) && (EX_MEM.dest == ID_EX.rt) && (MEM_WB.dest == ID_EX.rt))){
			ID_EX.B = MEM_WB.B;
			printf("MEM Forwarding\n");
			printf("Hazard eliminated\n");
			FF = 0;
			fetch_flag = 0;
		}
	}
	*/
}

/************************************************************/
/* execution (EX) pipeline stage:                                                                          */ 
/************************************************************/
void EX()
{
	uint32_t instruction,rs,rt,rd;
	instruction = ID_EX.IR;
	rs = (instruction & 0x03E00000) >> 21;
	rt = (instruction & 0x001F0000) >> 16;
	rd = (instruction & 0x0000F800) >> 11;
	if(cycle_count <2){		//DO nothing
		printf("EX is NULL, cycle %d\n",cycle_count);
	}
	//else if(ID_EX.regWrite == 1 && ID_EX.rd != 0 && rd == rt){
		//stall the pipeline for n cycles
		//involves clearing every control signal? dont want to mess with this yet
	//}
	//else if(ID_EX.regWrite == 1 && ID_EX.rd != 0 && rd == rs){
	//	//stall the pipeline for n cycles
	//}
	else if(FF == 1){
		EX_MEM.A = 0;
		EX_MEM.B = 0; 
		EX_MEM.imm = 0;
		EX_MEM.IR = 0;	
		EX_MEM.PC = 0;
	}
	else{
		EX_MEM = ID_EX;
		if(EX_MEM.type == 0){
			EX_MEM.dest = rd;
		}
		else if(EX_MEM.type == 1){
			EX_MEM.dest = rt;
		}
		else if(EX_MEM.type == 2){
			EX_MEM.dest = rt;			
		}
		if(EX_MEM.PC == 0){
			printf("EX: Execution is stalled\n");
		}
		else{
			printf("EX: ");
			print_instruction(EX_MEM.PC);	
			if(EX_MEM.type == 0){ /*ALU, register-register*/
				EX_MEM.regWrite = 1;
				printf("EX (reg-reg) destination reg: %d\n",EX_MEM.dest);
				EX_MEM.ALUOutput = do_instruction(EX_MEM.A,EX_MEM.B,EX_MEM.IR); 
			}
			else if(EX_MEM.type == 1){ //register-immediate
				EX_MEM.regWrite = 1;
				printf("EX  (reg-Imm) destination reg: %d\n",EX_MEM.dest);
				EX_MEM.ALUOutput = do_instruction(EX_MEM.A,EX_MEM.imm,EX_MEM.IR); 	
			}
			else if(EX_MEM.type == 2) { /*Load/Store*/
				EX_MEM.ALUOutput = EX_MEM.A + EX_MEM.imm;
				EX_MEM.regWrite = 1;
				//EX_MEM.B = ID_EX.B;
			}
			else if(EX_MEM.type == 3) { /*Load/Store*/
				EX_MEM.ALUOutput = EX_MEM.A + EX_MEM.imm;
			}
			instruction = EX_MEM.IR;
			rs = (instruction & 0x03E00000) >> 21;
			rt = (instruction & 0x001F0000) >> 16;
			rd = (instruction & 0x0000F800) >> 11;
		
			/*if( ENABLE_FORWARDING == 1 && (EX_MEM.type == 0 || EX_MEM.type == 1)){	
				if((EX_MEM.regWrite == 1) && (EX_MEM.dest != 0) && (EX_MEM.dest == ID_EX.rs)){
					ID_EX.rs = EX_MEM.ALUOutput;
					printf("EX Forwarding\n");
					printf("Hazard eliminated\n");
					FF = 0;
					fetch_flag = 0;
				}
				if((EX_MEM.regWrite == 1) && (EX_MEM.dest != 0) && (EX_MEM.dest == ID_EX.rt)){
					ID_EX.rt = EX_MEM.ALUOutput;
					printf("EX Forwarding\n");
					printf("Hazard eliminated\n");
					FF = 0;
					fetch_flag = 0;
				}
			}*/
		}
	}
}

/************************************************************/
/* instruction decode (ID) pipeline stage:                                                         */ 
/************************************************************/
void ID()
{
	int forwarded = 0;
	if(cycle_count <1){		//DO nothing
		printf("ID is NULL, cycle %d\n",cycle_count);
	}
	else if(FF == 1){

		printf("ID: Instruction decode is stalled\n");
		print_instruction(ID_EX.PC);
		printf("is stil in decode stage\n");
	}
	else{
		ID_EX = IF_ID;
		//printf("Sent to find_type: ");
		//print_instruction(ID_EX.PC);
		find_instruct_type();	//Parse the IF_ID.IR
		//ID_EX.type gets set in the find_instruct_type function!
		if( ID_EX.type == 4 ){
			fetch_flag = 1; //this kills the program
			count++;
		}
		else{
		ID_EX.rs = (0x03E00000 & ID_EX.IR) >> 21;
		ID_EX.rt = (0x001F0000 & ID_EX.IR) >> 16;
		ID_EX.imm = 0x0000FFFF & ID_EX.IR;	
		printf("ID: ");
		print_instruction(IF_ID.PC);
		//printf("rd in ex: %d\n",regDest);
		//printf("ID source registers: rs: %d rt: %d\nregDest: %d\nregWrite: %d\n\n",rs,rt,regDest_ex,EX_MEM.regWrite);
		if(ID_EX.type == 0){
			//printf("REG-REG in ID\n");
			//printf("EX_MEM.dest = %d\tID_EX.RS = %d\tID_EX.RT = %d\n",EX_MEM.dest,ID_EX.rs,ID_EX.rt);
			if((EX_MEM.regWrite == 1) && (EX_MEM.dest != 0) && ((EX_MEM.dest == ID_EX.rs) || (EX_MEM.dest == ID_EX.rt))){
				int dummy = EX_MEM.dest;
				printf("\n***Data hazard on $r%d in the EX_MEM stage (register type)***\n\n",dummy);
				if(ENABLE_FORWARDING == 1 && EX_MEM.dest == ID_EX.rs){
					if(EX_MEM.type != 2){
						ID_EX.A = EX_MEM.ALUOutput;
						printf("Hazard eliminated with forwarding\ncontents of EX_MEM $r%d were written to ID_EX.rs\n",EX_MEM.dest);
						forwarded = 1;
					}
					else{
						printf("Cannot forward data from EX stage, stalling\n");
						FF = 1;
					}
				}
				else if(ENABLE_FORWARDING == 1 && EX_MEM.dest == ID_EX.rt){
					if(EX_MEM.type != 2){
						ID_EX.B = EX_MEM.ALUOutput;
						printf("Hazard eliminated with forwarding\ncontents of EX_MEM $r%d were written to ID_EX.rt\n",EX_MEM.dest);
						forwarded = 1;
					}
					else{
						printf("Cannot forward data from EX stage, stalling\n");
						FF = 1;
					}
				}
				else{
					FF = 1; //stall
				}
			}
			else if((MEM_WB.regWrite == 1) && (MEM_WB.dest != 0) && ((MEM_WB.dest == ID_EX.rs) || (MEM_WB.dest == ID_EX.rt))){
				int dummy = MEM_WB.dest;
				printf("\n***Data hazard on $r%d in the MEM_WB stage (register type)***\n\n",dummy);
				if(ENABLE_FORWARDING == 1 && MEM_WB.dest == ID_EX.rs){
					ID_EX.A = MEM_WB.ALUOutput;
					printf("Hazard eliminated with forwarding\ncontents of MEM_WB $r%d were written to ID_EX.rs\n",MEM_WB.dest);
					forwarded = 1;
				}
				else if(ENABLE_FORWARDING == 1 && MEM_WB.dest == ID_EX.rt){
					ID_EX.B = MEM_WB.ALUOutput;
					printf("Hazard eliminated with forwarding\ncontents of MEM_WB $r%d were written to ID_EX.rt\n",MEM_WB.dest);
					forwarded = 1;
				}
				else{
					FF = 1;
				}
			}			
		}
		else if(ID_EX.type == 1){
			//printf("REG-IMM in ID\n");
			//printf("EX_MEM.dest: %d ID_EX.rt: %d\n",EX_MEM.dest,ID_EX.rs);
			if((EX_MEM.regWrite == 1) && (EX_MEM.dest != 0) && (EX_MEM.dest == ID_EX.rs)){
				printf("\n***Data hazard on $r%d in the EX_MEM stage (immediate instruction)***\n\n",ID_EX.rs);
				if(ENABLE_FORWARDING == 1 && EX_MEM.dest == ID_EX.rs){
					if(EX_MEM.type != 2){
						ID_EX.A = EX_MEM.ALUOutput;
						printf("Hazard eliminated with forwarding\ncontents of EX_MEM $r%d were written to ID_EX.rs\n",EX_MEM.dest);
						forwarded = 1;
					}
					else{
						printf("Cannot forward data from EX stage, stalling\n");
						FF = 1;
					}
				}
				else{
					FF = 1;
				}
			}
			else if((MEM_WB.regWrite == 1) && (MEM_WB.dest != 0) && (MEM_WB.dest == ID_EX.rs)){
				printf("\n***Data hazard on $r%d in the MEM_WB stage (immediate instruction)***\n\n",ID_EX.rs);
				if(ENABLE_FORWARDING == 1 && EX_MEM.dest == ID_EX.rs){
					ID_EX.A = MEM_WB.ALUOutput;
					printf("Hazard eliminated with forwarding\ncontents of MEM_WB $r%d were written to ID_EX.rs\n",MEM_WB.dest);
					forwarded = 1;
				}
				else{
					FF = 1;
				}
			}
		}
		else if(ID_EX.type == 2){
			//printf("EX_MEM.dest = %d\tID_EX.RS = %d\tID_EX.RT = %d\n",EX_MEM.dest,ID_EX.rs,ID_EX.rt);
			if((EX_MEM.regWrite == 1) && (EX_MEM.dest != 0) && ((EX_MEM.dest == ID_EX.rs) || (EX_MEM.dest == ID_EX.rt))){	
				//ID_EX.dest = regDest;
				int dummy = EX_MEM.dest;
				printf("\n***Data hazard on $r%d in the EX_MEM stage (load instruction)***\n\n",dummy);
				if(ENABLE_FORWARDING == 1 && EX_MEM.dest == ID_EX.rs){
					if(EX_MEM.type != 2){
						ID_EX.A = EX_MEM.ALUOutput;
						printf("Hazard eliminated with forwarding\ncontents of EX_MEM $r%d were written to ID_EX.rs\n",EX_MEM.dest);
						forwarded = 1;
					}
					else{
						printf("Cannot forward data from EX stage, stalling\n");
						FF = 1;
					}
				}
				else{
					FF = 1;
				}
			}
			else if((MEM_WB.regWrite == 1) && (MEM_WB.dest != 0) && ((MEM_WB.dest == ID_EX.rs) || (MEM_WB.dest == ID_EX.rt))){	
				//ID_EX.dest = regDest;
				int dummy = MEM_WB.dest;
				printf("\n***Data hazard on $r%d in the MEM_WB stage (load instruction)***\n\n",dummy);
				if(ENABLE_FORWARDING == 1 && EX_MEM.dest == ID_EX.rs){
					ID_EX.A = MEM_WB.ALUOutput;
					printf("Hazard eliminated with forwarding\ncontents of MEM_WB $r%d were written to ID_EX.rs\n",MEM_WB.dest);
					forwarded = 1;
				}
				else{
					FF = 1;
				}
			}
		}

		else if (ID_EX.type == 3){
			if((EX_MEM.regWrite == 1) && (EX_MEM.dest != 0) && (EX_MEM.dest == ID_EX.rt)){
				printf("\n***Data hazard on $r%d in the EX_MEM stage (store instruction)***\n\n",ID_EX.rt);
				if(ENABLE_FORWARDING == 1 && EX_MEM.dest == ID_EX.rt){
					if(EX_MEM.type != 2){
						ID_EX.B = EX_MEM.ALUOutput;
						printf("Hazard eliminated with forwarding\ncontents of EX_MEM $r%d were written to ID_EX.rt\n",EX_MEM.dest);
						forwarded = 1;
					}
					else{
						printf("Cannot forward data from EX stage, stalling\n");
						FF = 1;
					}
				}
				else{
					FF = 1;
				}
			}
			if((MEM_WB.regWrite == 1) && (MEM_WB.dest != 0) && (MEM_WB.dest == ID_EX.rt)){
				printf("\n***Data hazard on $r%d in the MEM_WB stage (store instruction)***\n\n",ID_EX.rt);
				if(ENABLE_FORWARDING == 1 && EX_MEM.dest == ID_EX.rt){
					ID_EX.B = MEM_WB.LMD;
					printf("Hazard eliminated with forwarding\ncontents of MEM_WB $r%d were written to ID_EX.rt\n",MEM_WB.dest);
					forwarded = 1;
				}
				else{
					FF = 1;
				}
			}
		}
		if(FF == 0){
			printf("Instruction progresses\n");
			if(forwarded == 0){
				ID_EX.A = CURRENT_STATE.REGS[ID_EX.rs];
				ID_EX.B = CURRENT_STATE.REGS[ID_EX.rt];
			}
			forwarded = 0; 
		}
		//printf("%x %x %x %x\n", ID_EX.A, ID_EX.B, ID_EX.imm, ID_EX.IR);
		}
	}
}

/************************************************************/
/* instruction fetch (IF) pipeline stage:                                                              */ 
/************************************************************/
void IF()
{
	//if(FF == 1){
	if(instruction_fetch_flag == 1){	
		printf("No instruction fetched\n");
		//NEXT_STATE.PC = 
		print_instruction(IF_ID.PC);
		printf("remains in IF stage\n");
		if(FF == 0){
			instruction_fetch_flag = 0;
		}
		printf("\n");
	}
	if(fetch_flag == 0 && FF == 0){
		IF_ID.PC = CURRENT_STATE.PC;
		IF_ID.IR = mem_read_32(CURRENT_STATE.PC);
		NEXT_STATE.PC = CURRENT_STATE.PC + 4;
		print_instruction(IF_ID.PC);
		printf(" is in IF stage\n");
		printf("IF: ");
		print_instruction(IF_ID.PC);
		//print_instruction(CURRENT_STATE.PC);
		printf("\n");
		if(FF == 1){
			instruction_fetch_flag = 1;
		}
	} 
	cycle_count++;
} 

/************************************************************/
/* parse instruction. This is from working simulator from Akturk, but modified         */ 
/************************************************************/
void find_instruct_type()
{
	/*IMPLEMENT THIS*/
	/* execute one instruction at a time. Use/update CURRENT_STATE and and NEXT_STATE, as necessary.*/
	
	//0->ALU: Reg to Reg
	//1->ALU: Reg to imm
	//2->Load
	//3->Store
	//4->SYSCALL - kill it
	
	uint32_t instruction, opcode, function, rs, rt, rd, sa, immediate, target;
	uint64_t product, p1, p2;
	
	uint32_t addr, data;
	
	int branch_jump = FALSE;
	
	//printf("[0x%x]\t ", CURRENT_STATE.PC);
	
	instruction = ID_EX.IR;
	
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
				ID_EX.type = 0;	
				break;
			case 0x02: //SRL --ALU
				ID_EX.type = 0;
				break;
			case 0x03: //SRA --ALU
				ID_EX.type = 0;		
				break;
			case 0x08: //JR
				
				break;
			case 0x09: //JALR
				
				break;
			case 0x0C: //SYSCALL
				ID_EX.type = 4;
				break;
			case 0x10: //MFHI --Load/Store........Reg to Reg?

				break;
			case 0x11: //MTHI --Load/Store........Reg to Reg?

				break;
			case 0x12: //MFLO --Load/Store........Reg to Reg?

				break;
			case 0x13: //MTLO --Load/Store........Reg to Reg?

				break;
			case 0x18: //MULT --ALU
				ID_EX.type = 0;
				break;
			case 0x19: //MULTU --ALU
				ID_EX.type = 0;
				break;
			case 0x1A: //DIV --ALU
				ID_EX.type = 0;
				break;
			case 0x1B: //DIVU --ALU
				ID_EX.type = 0;
				break;
			case 0x20: //ADD --ALU
				ID_EX.type = 0;
				break;
			case 0x21: //ADDU --ALU
				ID_EX.type = 0;
				break;
			case 0x22: //SUB --ALU
				ID_EX.type = 0;
				break;
			case 0x23: //SUBU --ALU
				ID_EX.type = 0;
				break;
			case 0x24: //AND --ALU
				ID_EX.type = 0;
				break;
			case 0x25: //OR --ALU
				ID_EX.type = 0;
				break;
			case 0x26: //XOR --ALU
				ID_EX.type = 0;
				break;
			case 0x27: //NOR --ALU
				ID_EX.type = 0;
				break;
			case 0x2A: //SLT --ALU
				ID_EX.type = 0;
				break;
			default:
				printf("find_instruct_type():Instruction at 0x%x is not implemented!\n", CURRENT_STATE.PC);
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
					
				}
				else if(rt == 0x00001){ //BGEZ
					if((CURRENT_STATE.REGS[rs] & 0x80000000) == 0x0){
						NEXT_STATE.PC = CURRENT_STATE.PC + ( (immediate & 0x8000) > 0 ? (immediate | 0xFFFF0000)<<2 : (immediate & 0x0000FFFF)<<2);
						branch_jump = TRUE;
					}
					
				}
				break;
			case 0x02: //J
				
				break;
			case 0x03: //JAL
				
				break;
			case 0x04: //BEQ
				
				break;
			case 0x05: //BNE
				
				break;
			case 0x06: //BLEZ
			
				break;
			case 0x07: //BGTZ
			
				break;
			case 0x08: //ADDI --ALU
				ID_EX.type = 1;
				break;
			case 0x09: //ADDIU --ALU
				ID_EX.type = 1;
				break;
			case 0x0A: //SLTI --ALU
				ID_EX.type = 1;
				break;
			case 0x0C: //ANDI --ALU
				ID_EX.type = 1;
				break;
			case 0x0D: //ORI --ALU
				ID_EX.type = 1;
				break;
			case 0x0E: //XORI --ALU
				ID_EX.type = 1;
				break;
			case 0x0F: //LUI --Load/Store
				ID_EX.type = 1;
				break;
			case 0x20: //LB --Load/Store
				ID_EX.type = 2;
				break;
			case 0x21: //LH --Load/Store
				ID_EX.type = 2;
				break;
			case 0x23: //LW --Load/Store
				ID_EX.type = 2;
				break;
			case 0x28: //SB --Load/Store
				ID_EX.type = 3;
				break;
			case 0x29: //SH --Load/Store
				ID_EX.type = 3;
				break;
			case 0x2B: //SW --Load/Store
				ID_EX.type = 3;
				break;
			default:
				// put more things here
				printf("find_type: Instruction at 0x%x is not implemented!\n", CURRENT_STATE.PC);
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
	ENABLE_FORWARDING = 0;
	count = 0;
	CURRENT_STATE.PC = MEM_TEXT_BEGIN;
	NEXT_STATE = CURRENT_STATE;
	RUN_FLAG = TRUE;
}

uint32_t do_instruction( uint32_t X, uint32_t Y, uint32_t instruct){
	uint32_t opcode = (instruct & 0xFC000000) >> 26;
	uint32_t function = (instruct & 0x0000003F);
	uint32_t answer;
	uint64_t p1,p2,product,quotient,remainder;
	if( FF == 1 ){
		//answer = 0x00000000;
		printf("Data Hazard Prevention\n");
	}
	else{
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
					if ((X & 0x80000000) == 0x80000000){
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
					//NEXT_STATE.LO = (product & 0X00000000FFFFFFFF);
					//NEXT_STATE.HI = (product & 0XFFFFFFFF00000000)>>32;
					answer = product;
					break;
				case 0x19: //MULTU
					//NEXT_STATE.HI = (product & 0XFFFFFFFF00000000)>>32;
					product = (uint64_t)X * (uint64_t)Y;
					//NEXT_STATE.LO = (product & 0X00000000FFFFFFFF);
					answer = product;
					break;
				case 0x1A: //DIV 
					if(Y != 0)
					{
						quotient = (int32_t)X / (int32_t)Y;
						remainder = (int32_t)X % (int32_t)Y;
					}
					answer = 0;
					answer = quotient | 0x0000FFFF;
					answer = remainder | 0xFFFF0000;
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
					printf("do_instruction: Instruction at 0x%x is not implemented!\n",EX_MEM.PC);
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
				case 0x0F: //LUI 
					X = Y << 16;
					answer = X;
					break;
				default:
					// put more things here
					printf("do_instruction: Instruction at 0x%x is not implemented!\n",EX_MEM.PC);

					break;
			}
		}
	}
	return answer;
}
/************************************************************/
/* Print the program loaded into memory (in MIPS assembly format)    */ 
/************************************************************/
void print_program(){
	int i;
	uint32_t addr;
	
	for(i=0; i<PROGRAM_SIZE; i++){
		addr = MEM_TEXT_BEGIN + (i*4);
		printf("[0x%x]\t", addr);
		print_instruction(addr);
	}
}

/************************************************************/
/* Print the instruction at given memory address (in MIPS assembly format)    */
/************************************************************/
void print_instruction(uint32_t addr){
	uint32_t instruction, opcode, function, rs, rt, rd, sa, immediate, target;
	
	instruction = mem_read_32(addr);
	
	opcode = (instruction & 0xFC000000) >> 26;
	function = instruction & 0x0000003F;
	rs = (instruction & 0x03E00000) >> 21;
	rt = (instruction & 0x001F0000) >> 16;
	rd = (instruction & 0x0000F800) >> 11;
	sa = (instruction & 0x000007C0) >> 6;
	immediate = instruction & 0x0000FFFF;
	target = instruction & 0x03FFFFFF;
	
	if(opcode == 0x00){
		/*R format instructions here*/
		
		switch(function){
			case 0x00:
				printf("SLL $r%u, $r%u, 0x%x\n", rd, rt, sa);
				break;
			case 0x02:
				printf("SRL $r%u, $r%u, 0x%x\n", rd, rt, sa);
				break;
			case 0x03:
				printf("SRA $r%u, $r%u, 0x%x\n", rd, rt, sa);
				break;
			case 0x08:
				printf("JR $r%u\n", rs);
				break;
			case 0x09:
				if(rd == 31){
					printf("JALR $r%u\n", rs);
				}
				else{
					printf("JALR $r%u, $r%u\n", rd, rs);
				}
				break;
			case 0x0C:
				printf("SYSCALL\n");
				break;
			case 0x10:
				printf("MFHI $r%u\n", rd);
				break;
			case 0x11:
				printf("MTHI $r%u\n", rs);
				break;
			case 0x12:
				printf("MFLO $r%u\n", rd);
				break;
			case 0x13:
				printf("MTLO $r%u\n", rs);
				break;
			case 0x18:
				printf("MULT $r%u, $r%u\n", rs, rt);
				break;
			case 0x19:
				printf("MULTU $r%u, $r%u\n", rs, rt);
				break;
			case 0x1A:
				printf("DIV $r%u, $r%u\n", rs, rt);
				break;
			case 0x1B:
				printf("DIVU $r%u, $r%u\n", rs, rt);
				break;
			case 0x20:
				printf("ADD $r%u, $r%u, $r%u\n", rd, rs, rt);
				break;
			case 0x21:
				printf("ADDU $r%u, $r%u, $r%u\n", rd, rs, rt);
				break;
			case 0x22:
				printf("SUB $r%u, $r%u, $r%u\n", rd, rs, rt);
				break;
			case 0x23:
				printf("SUBU $r%u, $r%u, $r%u\n", rd, rs, rt);
				break;
			case 0x24:
				printf("AND $r%u, $r%u, $r%u\n", rd, rs, rt);
				break;
			case 0x25:
				printf("OR $r%u, $r%u, $r%u\n", rd, rs, rt);
				break;
			case 0x26:
				printf("XOR $r%u, $r%u, $r%u\n", rd, rs, rt);
				break;
			case 0x27:
				printf("NOR $r%u, $r%u, $r%u\n", rd, rs, rt);
				break;
			case 0x2A:
				printf("SLT $r%u, $r%u, $r%u\n", rd, rs, rt);
				break;
			default:
				printf("Instruction is not implemented!\n");
				break;
		}
	}
	else{
		switch(opcode){
			case 0x01:
				if(rt == 0){
					printf("BLTZ $r%u, 0x%x\n", rs, immediate<<2);
				}
				else if(rt == 1){
					printf("BGEZ $r%u, 0x%x\n", rs, immediate<<2);
				}
				break;
			case 0x02:
				printf("J 0x%x\n", (addr & 0xF0000000) | (target<<2));
				break;
			case 0x03:
				printf("JAL 0x%x\n", (addr & 0xF0000000) | (target<<2));
				break;
			case 0x04:
				printf("BEQ $r%u, $r%u, 0x%x\n", rs, rt, immediate<<2);
				break;
			case 0x05:
				printf("BNE $r%u, $r%u, 0x%x\n", rs, rt, immediate<<2);
				break;
			case 0x06:
				printf("BLEZ $r%u, 0x%x\n", rs, immediate<<2);
				break;
			case 0x07:
				printf("BGTZ $r%u, 0x%x\n", rs, immediate<<2);
				break;
			case 0x08:
				printf("ADDI $r%u, $r%u, 0x%x\n", rt, rs, immediate);
				break;
			case 0x09:
				printf("ADDIU $r%u, $r%u, 0x%x\n", rt, rs, immediate);
				break;
			case 0x0A:
				printf("SLTI $r%u, $r%u, 0x%x\n", rt, rs, immediate);
				break;
			case 0x0C:
				printf("ANDI $r%u, $r%u, 0x%x\n", rt, rs, immediate);
				break;
			case 0x0D:
				printf("ORI $r%u, $r%u, 0x%x\n", rt, rs, immediate);
				break;
			case 0x0E:
				printf("XORI $r%u, $r%u, 0x%x\n", rt, rs, immediate);
				break;
			case 0x0F:
				printf("LUI $r%u, 0x%x\n", rt, immediate);
				break;
			case 0x20:
				printf("LB $r%u, 0x%x($r%u)\n", rt, immediate, rs);
				break;
			case 0x21:
				printf("LH $r%u, 0x%x($r%u)\n", rt, immediate, rs);
				break;
			case 0x23:
				printf("LW $r%u, 0x%x($r%u)\n", rt, immediate, rs);
				break;
			case 0x28:
				printf("SB $r%u, 0x%x($r%u)\n", rt, immediate, rs);
				break;
			case 0x29:
				printf("SH $r%u, 0x%x($r%u)\n", rt, immediate, rs);
				break;
			case 0x2B:
				printf("SW $r%u, 0x%x($r%u)\n", rt, immediate, rs);
				break;
			default:
				printf("Instruction is not implemented!\n");
				break;
		}
	}
}
/************************************************************/
/* Print the current pipeline                                                                                    */ 
/************************************************************/
void show_pipeline(){
	/*IMPLEMENT THIS*/
	printf("Current PC: %x\n", CURRENT_STATE.PC);
	printf("IF/ID.IR: %x\n", IF_ID.IR);
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
	
	printf("TEST0\n");
	if (argc < 2) {
		printf("Error: You should provide input file.\nUsage: %s <input program> \n\n",  argv[0]);
		exit(1);
	}
	printf("TEST1\n");
	strcpy(prog_file, argv[1]);
	printf("TEST2\n");
	initialize();
	printf("TEST2\n");
	load_program();
	printf("TEST3\n");
	help();
	printf("TEST4\n");
	while (1){
		handle_command();
		printf("TEST5\n");
	}
	return 0;
}
