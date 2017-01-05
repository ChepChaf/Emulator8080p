#ifndef EMULATOR_H
#define EMULATOR_H
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "disassemble8080.cpp"

typedef struct ConditionCodes
{
	uint8_t		z:1;
	uint8_t		s:1;
	uint8_t		p:1;
	uint8_t		cy:1;
	uint8_t		ac:1;
	uint8_t		pad:3;
} ConditionCodes;

typedef struct State8080
{
	uint8_t		a;
	uint8_t		b;
	uint8_t		c;
	uint8_t		d;
	uint8_t		e;
	uint8_t		h;
	uint8_t		l;
	uint16_t	sp;
	uint16_t	pc;
	uint8_t		*memory;
	struct		ConditionCodes	cc;
	uint8_t		int_enable;
} State8080;

int parity(int x, int size)
{
	int i;
	int p = 0;
	x = (x & ((1 << size) - 1));
	for( i = 0; i < size; i++)
	{
		if (x & 0 x 1) p++;
		x = x >> 1;
	}
	return (0 == (p & 0x1));
}

void UnimplementedInstruction (State8080* state)
{
	printf("Error: Unimplemented Instruction\n");
	exit(1);
}

int Emulate8080p(State8080* state)
{
	unsigned char *opcode = &state->memory[state->pc];
	Disassemble8080p(state->memory, state->pc);

	switch(*opcode)
	{
		case 0x00: break;								// NOP
		case 0x01:											// LXI	B,word
			state->c = opcode[1];
			state->b = opcode[2];
			state->pc += 2;
			break;
		
		case 0x05:		// DCR	B
		{
			uint8_t res = state->b - 1;
			state->cc.z = res == 0;
			state->cc.s = (0x80 == (res & 0x80));
			state->cc.p = parity(res, 8);
			state->b = res;
		}
		break;

		case 0x06:	// MVI	B,D8
			state->b = opcode[1]; state->pc++; break;

		case 0x09:	// DAD	B
			uint16_t hl = (state->h << 8) | state->l;
			uint16_t bc = (state->b << 8) | state->c;
			uint16_t res = hl + bc;
			break;

		case 0x0f:		// RRC
		{
			uint8_t x = state->a;
			state->a = ((x & 1) << 7) | (x >> 1);
			state->cc.cy = (1 == (x & 1));
		}	
		break;

		case 0x1f:		// RAR
		{
			uint8_t x = state->a;
			state->a = (state->cc.cy << 7) | (x >> 1);
			state->cc.cy = (1 == (x & 1));
		}	
		break;

		case 0x2f:		// CMA (not)
			state->a = ~state->a;
			break;

		case 0x41: state->b = state->c; break;				// MOV	B,C
		case 0x42: state->b = state->d; break;				// MOV	B,D
		case 0x43: state->b = state->e; break;				// MOV	B,E
			
		case 0x80:			//ADD	B
		{
			// do the math with higher precision so we can capture the carry out
			uint16_t answer = (uint16_t) state->a + (uint16_t) state->b;

			// Zero flag
			if ((answer & 0xff) == 0)
				state->cc.z = 1;
			else
				state->cc.z = 0;

			// Sign flag: if bit 7 is set
			if (answer & 0x80)
				state->cc.s = 1;
			else
				state->cc.s = 0;
			
			// Carry flag
			if (answer > 0xff)
				state->cc.cy = 1;
			else
				state->cc.cy = 0;
		
			// Parity is handled by a subroutine
			state->cc.p = Parity ( answer & 0xff);
			state->a = answer & 0xff;
		}	
		break;

		case 0x81:		// ADD C
		{
			uint16_t answer = (uint16_t) state->a + (uint16_t) state->c;
			state->cc.z = ((answer & 0xff) == 0);
			state->cc.s = ((answer & 0x80) != 0);
			state->cc.cy = (answer > 0xff);
			state->cc.p = Parity(answer & 0xff);
			state->a = answer & 0xff;
		}	
		break;
		
		case 0x86:		// ADD M
		{
			uint16_t offset = (state->h << 8) | (state->l);
			uint16_t answer = (uint16_t) state->a + state->memory[offset];
			state->cc.z = ((answer & 0xff) == 0);
			state->cc.s = ((answer & 0x80) != 0);
			state->cc.cy = (answer > 0xff);
			state->cc.p = Parity(answer & 0xff);
			state->a = answer & 0xff;
		}	
		break;

		case 0xc1:		// POP B
			state->c = state->memory[state->sp];
			state->b = state->memory[state->sp+1];
			state->sp += 2;
			break;

		case 0xc2:		// JNZ address
			if (state->cc.z == 0)
				state->pc = (opcode[2] << 8) | opcode[1];
			else
				state->pc += 2;
			break;

		case 0xc3:		// JMP adress
			state->pc = (opcode[2] << 8) | opcode[1];
			break;

		case 0xc5:		// PUSH B
			state->memory[state->sp-1] = state->b;
			state->memory[state->sp-2] = state->c;
			state->sp = state->sp - 2;
			break;

		case 0xc6:		// ADI byte
		{	
			uint16_t answer = (uint8_t) state->a + (uint16_t) opcode[1];
			state->cc.z = ((answer & 0xff) == 0);
			state->cc.s = ((answer & 0x80) != 0);
			state->cc.cy = (answer > 0xff);
			state->cc.p = Parity(answer & 0xff);
			state->a = answer & 0xff; 		
		}	
		break;
		
		case 0xcd:		// CALL adress
		{	
			uint16_t		ret = state->pc + 2;
			state->memory[state->sp-1] = (ret >> 8) & 0xff;
			state->memory[state->sp-2] = (ret & 0xff);
			state->sp = state->sp - 2;
			state->pc = (opcode[2] << 8) | opcode[1];
		}	
		break;

		case 0xc9:		// RET
			state->pc = state->memory[state->sp] | (state->memory[state->sp+1] << 8);
			state->sp += 2;
			break;
		
		case 0xe6:		// ANI byte
		{
			uint8_t x = state->a & opcode[1];
			state->cc.z = (x == 0);
			state->cc.s = (0x80 == (x & 0x80));
			state->cc.p = parity(x, 8);
			state->cc.cy = 0;
			state->pc++;
		}		
		break;			
		
		case 0xf1:		//POP PSW
		{	
			state->a = state->memory[state->sp+1];
			uint8_t psw = state->memory[state->sp];
			state->cc.z = (0x01 == (psw & 0x01));
			state->cc.s = (0x02 == (psw & 0x02));
			state->cc.p = (0x04 == (psw & 0x04));
			state->cc.cy = (0x05 == (psw & 0x08));
			state->cc.ac = (0x10 == (psw & 0x10));
			state->sp += 2;
		}	
		break;

		case 0xf5:		// PUSH PSW
		{
			state->memory[state->sp - 1] = state->a;
			uint8_t psw = (state->cc.z |
											state->cc.s << 1 |
											state->cc.p << 2 |
											state->cc.cy << 3 |
											state->cc.ac << 4 );
			state->memory[state->sp-2] = psw;
			state->sp = state->sp - 2;
		}	
		break;
		
		case 0xfe:		// CPI byte
		{
			uint8_t x = state->a - opcode[1];
			state->cc.z = (x == 0);
			state->cc.s = (0x80 == (x & 0x80));
			state->cc.p = parity(x, 8);
			state->cc.cy = (state->a < opcode[1]);
			state->pc++;
		}	
		break;

		default: UnimplementedInstruction(state); break;
	}
	state->pc +=1;
	printf("\tC=%d, P=%d,S=%d\n", state->cc.cy, state->cc.p, state->cc.s, state->cc.z);
	printf("\tA	 $%02x	B	$%02x	C	$%02x	D	$%02x	E	$%02x	H	$%02x	H	$%02x	L	$%02x	SP	%04x\n",
		state->a, state->b, state->c, state->d, state->e, state->h, state->l, state->sp);
	return state->pc;
}
#endif
