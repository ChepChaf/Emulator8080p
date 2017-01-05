#ifndef DISASSEMBLE_H
#define DISASSEMBLE_H
#include <stdlib.h>
#include <stdio.h>
#include "Emulator8080p.cpp"

int Disassemble8080Op (unsigned char *codebuffer, int pc);
int Disassemble8080Op (unsigned char *codebuffer, int pc)
{
	unsigned char *code = &codebuffer[pc];
	int opbytes = 1;
	printf ("%04x ", pc);
	switch (*code)
	{
		case 0x00: printf("NOP"); break;
		case 0x01: printf("LXI	B, #$%02x%02x", code[2], code[1]); opbytes = 3; break;
		case 0x02: printf("STAX	B"); break;
		case 0x03: printf("INX	B"); break;
		case 0x04: printf("INR	B"); break;
		case 0x05: printf("DCR	B"); break;
		case 0x06: printf("MVI	B,#$%02x", code[1]); opbytes = 2; break;
		case 0x07: printf("RLC"); break;
		case 0x08: printf("NOP"); break;
		case 0x09: printf("DAD	B"); break;
		case 0x0a: printf("LDAX	B"); break;
		case 0x0b: printf("DCX	B"); break;
		case 0x0c: printf("INR	C"); break;
		case 0x0d: printf("DCR	C"); break;
		case 0x0e: printf("MVI	C,#$%02x", code[1]); opbytes = 2; break;
		case 0x0f: printf("RRC"); break;
		case 0x10: printf("NOP"); break;
		case 0x11: printf("LXI	D,#$%02x%02x", code[2], code[1]); opbytes = 3; break;
		case 0x12: printf("STAX	D"); break;
		case 0x13: printf("INX	D"); break;
		case 0x14: printf("INR	D"); break;
		case 0x15: printf("DCR	D"); break;
		case 0x16: printf("MVI	D,#$%02x", code[1]); opbytes = 2; break;
		case 0x17: printf("RAL"); break;
		case 0x18: printf("NOP"); break;
		case 0x19: printf("DAD	D"); break;
		case 0x1a: printf("LDAX	D"); break;
		case 0x1b: printf("DCX	D"); break;
		case 0x1c: printf("INR	E"); break;
		case 0x1d: printf("DCR	E"); break;
		case 0x1e: printf("MVI	E,#$%02x", code[1]); opbytes = 2; break;
		case 0x1f: printf("RAR"); break;
		case 0x20: printf("RIM"); break;
		case 0x21: printf("LXI	H,#$%02x%02x", code[2], code[1]); opbytes = 3; break;
		case 0x22: printf("SHLD	$%02x%02x", code[2], code[1]); opbytes = 3; break;
 	
		case 0x3e: printf("MVI	A,#0x%02x", code[1]); opbytes = 2; break;
		
		case 0xc1: printf("POP	B"); break;
		case 0xc2: printf("JNZ	#%02x%02x", code[2], code[1]); opbytes = 3; break;
		case 0xc3: printf("JMP	$%02x%02x", code[2], code[1]); opbytes = 3; break;

		case 0xc5: printf("PUSH	B"); break;
		
		case 0xcd: printf("CALL	 #%02x%02x", code[2], code[1]); opbytes = 3; break;

		case 0xd1: printf("POP	D"); break;
		case 0xd2: printf("JNC	#%02x%02x", code[2], code[1]); opbytes = 3; break;
		case 0xd3: printf("OUT	D,#$%02x", code[1]); opbytes = 2; break;
		case 0xd4: printf("CNC	#%02x%02x", code[2], code[1]); opbytes = 3; break;
		case 0xd5: printf("PUSH	D"); break;

		case 0xe5: printf("PUSH H"); break;

		case 0xf5: printf("PUSH PSW"); break;
		
		default: printf("UNKN"); break;
	}
	printf("\n");
	return opbytes;
}
#endif
