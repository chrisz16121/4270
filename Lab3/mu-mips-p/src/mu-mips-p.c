#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#include "mu-mips.h"


int cycle_count = 0;

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
	uint32_t rd,rt,opcode,function,instruction;
	if(CYCLE_COUNT <=4){		//DO nothing
		//printf("WB is NULL, cycle %d\n",CYCLE_COUNT);
	}
	else if( fetch_flag == 1 && count == 3 ){
		printf("killing...\n");
		exit(NULL);
	}
	else{
		//printf("WB_STAGE: cycle %d\n",CYCLE_COUNT);
		INSTRUCTION_COUNT++;
		if(MEM_WB.type == 1){/*register-immediate*/	
			rt = (MEM_WB.IR & 0x001F0000) >> 16;
			NEXT_STATE.REGS[rt] = MEM_WB.ALUOutput;
			printf("\nIntstruction: ");
			print_instruction(MEM_WB.PC);
			printf("%x was written to register %d\n",MEM_WB.ALUOutput,rt);
		} 
		else if(MEM_WB.type == 0) {/*register-register*/
			printf("\nIntstruction: ");
			print_instruction(MEM_WB.PC);
			instruction = MEM_WB.IR;
			function = instruction & 0x0000003F;
			printf("Function: %x\n",function);
			//opcode = (instruction & 0xFC000000) >> 26;
			if( function == 0x18 || function == 0x19 || function == 0x1A || function == 0x1B){
				//printf("MULT/DIV operation!\n");
				//printf("The result is %016x\n",MEM_WB.ALUOutput);
				NEXT_STATE.HI = MEM_WB.ALUOutput & 0xFFFFFFFF00000000;
				NEXT_STATE.LO = MEM_WB.ALUOutput & 0x00000000FFFFFFFF;
				printf("%x was written to the HI register\n",NEXT_STATE.HI);
				printf("%x was written to the LO register\n",NEXT_STATE.LO);						
			}
			else{
				rd = (MEM_WB.IR & 0x0000F800) >> 11;
				NEXT_STATE.REGS[rd] = MEM_WB.ALUOutput;	
				printf("%x was written to register %d\n",MEM_WB.ALUOutput,rd);
			}
		}	
		else if(MEM_WB.type == 2){ /*Load*/
			rt = (MEM_WB.IR & 0x001F0000) >> 16;
			NEXT_STATE.REGS[rt] = MEM_WB.LMD;
			printf("\nIntstruction: ");
			print_instruction(MEM_WB.PC);
			printf("%x was loaded into register %d\n",MEM_WB.LMD,rt);
		}
	}
	INSTRUCTION_COUNT++;
}

/************************************************************/
/* memory access (MEM) pipeline stage:                                                          */ 
/************************************************************/
void MEM()
{
	if(CYCLE_COUNT <=3){		//DO nothing
		//printf("MEM is NULL, cycle %d\n",CYCLE_COUNT);
	}
	else{
		//printf("MEM_STAGE: cycle %d\n",CYCLE_COUNT);
		if(EX_MEM.type == 0 || EX_MEM.type == 1){
			MEM_WB.ALUOutput = EX_MEM.ALUOutput;
		}
		else{ /*Load/Store*/
			if(EX_MEM.type == 2){ //Load
				MEM_WB.LMD = mem_read_32(EX_MEM.ALUOutput);
			} 
			else if(EX_MEM.type == 3) { //Store
				mem_write_32(EX_MEM.ALUOutput,EX_MEM.B);
				printf("\nIntstruction: ");
				print_instruction(EX_MEM.PC);
				printf("%x was stored at location %08x\n",EX_MEM.B,EX_MEM.ALUOutput);
			}
		}
		MEM_WB.IR = EX_MEM.IR;
	}
	MEM_WB.PC = EX_MEM.PC;
	MEM_WB.A = EX_MEM.A;
	MEM_WB.B = EX_MEM.B;
	MEM_WB.imm = EX_MEM.imm;
	MEM_WB.type = EX_MEM.type;
}

/************************************************************/
/* execution (EX) pipeline stage:                                                                          */ 
/************************************************************/
void EX()
{
	if(CYCLE_COUNT <=2){		//DO nothing
		//printf("EX is NULL, cycle %d\n",CYCLE_COUNT);
	}
	else{
		//printf("EX_STAGE: cycle %d\n",CYCLE_COUNT);		
		//printf("%x %x %x %x\n", ID_EX.A, ID_EX.B, ID_EX.imm, ID_EX.IR);
		if(ID_EX.type == 0){ /*ALU, register-register*/
			//printf("X: %x\tY: %x\n",ID_EX.A,ID_EX.B);
			EX_MEM.ALUOutput = do_instruction(ID_EX.A,ID_EX.B,ID_EX.IR); 
		}
		else if(ID_EX.type == 1){ //register-immediate
			EX_MEM.ALUOutput = do_instruction(ID_EX.A,ID_EX.imm,ID_EX.IR); 	
		}
		else if(ID_EX.type == 2 || ID_EX.type == 3) { /*Load/Store*/
			EX_MEM.ALUOutput = ID_EX.A + ID_EX.imm;
			EX_MEM.B = ID_EX.B;
		}
		EX_MEM.type = ID_EX.type;
		EX_MEM.IR = ID_EX.IR;
		EX_MEM.PC = ID_EX.PC;
		EX_MEM.A = ID_EX.A;
		EX_MEM.B = ID_EX.B;
		EX_MEM.imm = ID_EX.imm;
		EX_MEM.LMD = ID_EX.LMD;
	}
}

/************************************************************/
/* instruction decode (ID) pipeline stage:                                                         */ 
/************************************************************/
void ID()
{
	if(CYCLE_COUNT <=1){		//DO nothing
		//printf("ID is NULL, cycle %d\n",CYCLE_COUNT);
	}
	else{
		//printf("ID_STAGE: cycle %d\n",CYCLE_COUNT);
		find_instruct_type();	//Parse the IF_ID.IR
		//ID_EX.type gets set in the find_instruct_type function!
		if( ID_EX.type == 4 ){
			fetch_flag = 1; //this kills the program
			count++;
		}
		else{
		uint32_t rs, rt, immediate; 
		rs = (0x03E00000 & IF_ID.IR) >> 21;
		rt = (0x001F0000 & IF_ID.IR) >> 16;
		immediate = 0x0000FFFF & IF_ID.IR;
		//immediate = 0xFFFFFFFF ^ immediate;
		ID_EX.A = CURRENT_STATE.REGS[rs];
		ID_EX.B = CURRENT_STATE.REGS[rt]; 
		//printf("X: %x\tY: %x\n",ID_EX.A,ID_EX.B);
		ID_EX.imm = immediate;
		ID_EX.IR = IF_ID.IR;	
		ID_EX.PC = IF_ID.PC;
		//printf("Intstruction: ");
		//print_instruction(IF_ID.PC);
		//printf("%x %x %x %x\n", ID_EX.A, ID_EX.B, ID_EX.imm, ID_EX.IR);
		}
	}
	ID_EX.PC = IF_ID.PC;
}

/************************************************************/
/* instruction fetch (IF) pipeline stage:                                                              */ 
/************************************************************/
void IF()
{
	//printf("IF_STAGE: cycle %d\n",CYCLE_COUNT);
	if(fetch_flag == 0){
		IF_ID.PC = CURRENT_STATE.PC;
		IF_ID.IR = mem_read_32(CURRENT_STATE.PC);
		NEXT_STATE.PC = CURRENT_STATE.PC + 4;
		//printf("%08x was fetched from instruction memory\n",IF_ID.IR);
	} 
}

/************************************************************/
/* parse instruction. This is from working simulator from Akturk, but modified         */ 
/************************************************************/
void find_instruct_type()
{
	/*IMPLEMENT THIS*/
	/* execute one instruction at a time. Use/update CURRENT_STATE and and NEXT_STATE, as necessary.*/
	
	//0->ALU: Reg to Reg
	//1->ALU: Reg to Mem
	//2->Load
	//3->Store
	//4->SYSCALL - kill it
	
	uint32_t instruction, opcode, function, rs, rt, rd, sa, immediate, target;
	uint64_t product, p1, p2;
	
	uint32_t addr, data;
	
	int branch_jump = FALSE;
	
	//printf("[0x%x]\t\n", CURRENT_STATE.PC);
	
	instruction = IF_ID.IR;
	
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
				ID_EX.type = 0;
				break;
			case 0x11: //MTHI --Load/Store........Reg to Reg?
				ID_EX.type = 0;
				break;
			case 0x12: //MFLO --Load/Store........Reg to Reg?
				ID_EX.type = 0;
				break;
			case 0x13: //MTLO --Load/Store........Reg to Reg?
				ID_EX.type = 0;
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
				printf("Instruction at 0x%x is not implemented!\n", CURRENT_STATE.PC);
				break;
		}
	}
	else{
		switch(opcode){
			case 0x01:
				if(rt == 0x00000){ //BLTZ
				// 	if((CURRENT_STATE.REGS[rs] & 0x80000000) > 0){
// 						NEXT_STATE.PC = CURRENT_STATE.PC + ( (immediate & 0x8000) > 0 ? (immediate | 0xFFFF0000)<<2 : (immediate & 0x0000FFFF)<<2);
// 						branch_jump = TRUE;
// 					}
					
				}
				else if(rt == 0x00001){ //BGEZ
			// 		if((CURRENT_STATE.REGS[rs] & 0x80000000) == 0x0){
// 						NEXT_STATE.PC = CURRENT_STATE.PC + ( (immediate & 0x8000) > 0 ? (immediate | 0xFFFF0000)<<2 : (immediate & 0x0000FFFF)<<2);
// 						branch_jump = TRUE;
// 					}
					
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
				printf("Instruction at 0x%x is not implemented!\n", CURRENT_STATE.PC);
				break;
		}
	}
}

/************************************************************/
/* Initialize Memory                                                                                                    */ 
/************************************************************/
void initialize() { 
	init_memory();
	count = 0;
	CURRENT_STATE.PC = MEM_TEXT_BEGIN;
	NEXT_STATE = CURRENT_STATE;
	RUN_FLAG = TRUE;
}

uint64_t do_instruction( uint32_t X, uint32_t Y, uint32_t instruct){
	//This is where we will have a large case statement 
	//to determine what operation to do on X and Y

	/*CHRIS ADDED THIS AND CHANGED THE PROTOYPE (SEE EX() FUNCTION)*/
	uint32_t opcode = (instruct & 0xFC000000) >> 26;
	uint32_t function = (instruct & 0x0000003F);
	uint64_t answer;
	uint64_t p1,p2,product,quotient,remainder;
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
				case 0x10: //MFHI --Load/Store........Reg to Reg?
					ID_EX.type = 0;
					break;
				case 0x11: //MTHI --Load/Store........Reg to Reg?
					ID_EX.type = 0;
					break;
				case 0x12: //MFLO --Load/Store........Reg to Reg?
					ID_EX.type = 0;
					break;
				case 0x13: //MTLO --Load/Store........Reg to Reg?
					ID_EX.type = 0;
					break;
				case 0x18: //MULT
					if ((X & 0x80000000) == 0x80000000){
						p1 = 0xFFFFFFFF00000000 | (uint64_t)X;
					}else{
						p1 = 0x00000000FFFFFFFF & (uint64_t)X;
					}
					if ((Y & 0x80000000) == 0x80000000){
						p2 = 0xFFFFFFFF00000000 | (uint64_t)Y;
					}else{
						p2 = 0x00000000FFFFFFFF & (uint64_t)Y;
					}
					
					product = p1 * p2;
					//NEXT_STATE.LO = (product & 0X00000000FFFFFFFF);
					//NEXT_STATE.HI = (product & 0XFFFFFFFF00000000)>>32;
					answer = product;
					//printf("Mulriply result is %x * %x = %016x\n",p1,p2,answer);
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
					answer = quotient | 0x00000000FFFFFFFF;
					remainder = remainder << 32;
					answer = remainder | 0xFFFFFFFF00000000;
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
				case 0x0F: //LUI
					answer = Y << 16;
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
