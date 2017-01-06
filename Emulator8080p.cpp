#ifndef EMULATOR_H
#define EMULATOR_H
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "disassemble8080.cpp"

int Disassemble8080p(unsigned char*, int);

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
		if (x & 0x1) p++;
		x = x >> 1;
	}
	return (0 == (p & 0x1));
}

void UnimplementedInstruction (State8080* state)
{
	printf("Error: Unimplemented Instruction\n");
	state->pc--;
	printf("%02x\n", state->memory[state->pc] & 0xff);
	exit(1);
}

int Emulate8080p(State8080* state)
{
	unsigned char *opcode = &state->memory[state->pc];
	Disassemble8080p(state->memory, state->pc);

	state->pc += 1;

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
			state->cc.z = (res == 0);
			state->cc.s = (0x80 == (res & 0x80));
			state->cc.p = parity(res, 8);
			state->b = res;
		}
		break;

		case 0x06:	// MVI	B,D8
			state->b = opcode[1]; state->pc++; break;

		case 0x09:	// DAD	B
		{
			uint16_t hl = (state->h << 8) | state->l;
			uint16_t bc = (state->b << 8) | state->c;
			uint16_t res = hl + bc;
			state->h = (res & 0xf00) >> 8;
			state->l = (res & 0xff);
			state->cc.cy = ((res & 0xffff0000) > 0);
		}	
		break;

		case 0x0d:		//DCR C
		{
			uint8_t res = state->c - 1;
			state->cc.z = (res == 0);
			state->cc.s = (0x80 == (res & 0x80));
			state->cc.p = parity(res, 8);
			state->c = res;
		}
		break;
		
		case 0x0e:		// MVI	C,D8
			state->c = opcode[1]; state->pc++; break;
	
		case 0x0f:		// RRC
		{
			uint8_t x = state->a;
			state->a = ((x & 1) << 7) | (x >> 1);
			state->cc.cy = (1 == (x & 1));
		}	
		break;

		case 0x11:		// LXI	D,D16
			state->d = opcode[2];
			state->e = opcode[1];
			state->pc += 2;
			break;

		case 0x13:		// INX	D
		{

			uint16_t de = ((state->d << 8) | state->e) + 1;
			state->d = (de & 0xff00) >> 8;
			state->e = (de & 0xff);
		}
		break;

		case 0x19:			// DAD	D
		{
			uint16_t hl = (state->h << 8) | state->l;
			uint16_t de = (state->d << 8) | state->e;
			uint16_t res = hl + de;
			state->h = (res & 0xf00) >> 8;
			state->l = (res & 0xff);
			state->cc.cy = ((res & 0xffff0000) > 0);
		}
		
		case 0x1a:		// LDAX	D
		{
			uint16_t de = (state->d << 8) | state->e;
			state->a = state->memory[de];
		}
		break;

		case 0x21:		// LXI	H,D16
			state->l = opcode[1];
			state->h = opcode[2];
			state->pc += 2;
			break;
		
		case 0x23:		// INX	H
		{
			uint16_t hl = ((state->h << 8) | state->l) + 1;
			state->h = ((hl & 0xff00) >> 8);
			state->l = (hl & 0xff);
		}
		break;
		
		case 0x26:		// MVI	H,D8
			state->l = opcode[1]; state->pc++; break;
	
		case 0x29:		// DAD	H	
		{
			uint16_t hl = (state->h << 8) | state->l;
			hl = hl + hl;
			state->h = ((hl & 0xff00) >> 0);
			state->l = (hl & 0xff);
			state->cc.cy = ((hl & 0xffff0000) != 0);
		}
		break;

		case 0x31:		// LXI	SP,D16
			state->sp = (opcode[2] << 8) | opcode[1];	state->pc += 2; break;
		
		case 0x32:		// STA	adr
		{
			uint16_t adr = (opcode[2] << 8) | opcode[1];
			state->memory[adr] = state->a;
			state->pc += 2;
		}
		break;

		case 0x36:		// MVI	M,D8
		{
			uint16_t hl = (state->h << 8) | state->l;
			state->memory[hl] = opcode[1];
			state->pc++;
		}
		break;		
		
		case 0x3a:		// LDA adr
			state->a = state->memory[opcode[1]]; state->pc++; break;
		
		case 0x3e:		// MVI	A,D8
			state->a = opcode[1]; state->pc++; break;
	
		case 0x56:		// MOV	D,M
		{
			uint16_t hl = (state->h << 8) | state->l;
			state->d = state->memory[hl];
		}
		break;
		
		case 0x5e:		// MOV	E,M
		{
			uint16_t hl = (state->h << 8) | state->l;
			state->e = state->memory[hl];
		}
		break;

		case 0x66:		// MOV	H,M
		{
			uint16_t hl = (state->h << 8) | state->l;
			state->e = state->memory[hl];
		}
		break;

		case 0x6f:		// MOV	L,A
			state->l = state->a; break;

		case 0x77:		// MOV	M,A
		{
			uint16_t hl = (state->h << 8) | state->l;
			state->memory[hl] = state->a;
		}
		break;
		
		case 0x7a:		// MOV	A,D
			state->a = state->d; break;
		
		case 0x7b:		// MOV	A,E
			state->a = state->e; break;
		
		case 0x7c:		// MOV	A,H
			state->a = state->h; break;
		
		case 0x7d:		// MOV	A,L
			state->a = state->l; break;
		
		case 0x7e:		// MOV	A,M
		{
			uint16_t hl = (state->h << 8) | state->l;
			state->a = state->memory[hl];
		}
		break;
		
		case 0xa7:			// ANA	A
			state->a = state->a & state->a;
			state->cc.z = (state->a == 0);
			state->cc.s = (0x80 == (state->a & 0x80));
			state->cc.p = parity(state->a, 8);
			break;
		
		case 0xaf:		// XRA	A
			state->a = state->a ^ state->a;
			state->cc.z = (state->a == 0);
			state->cc.s = (0x80 == (state->a & 0x80));
			state->cc.p = parity(state->a, 8);
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
			state->cc.p = parity(answer & 0xff, 8);
			state->a = answer & 0xff; 		
		}	
		break;
		
		case 0xc9:			// RET
			state->pc = state->memory[state->sp] | (state->memory[state->sp + 1] << 8);
			state->sp += 2;
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

		case 0xd1:		// POP	D
			state->e = state->memory[state->sp];
			state->d = state->memory[state->sp + 1];
			state->sp += 2;
			break;
		
		case 0xd3:		// OUT	D8
			state->pc++;
			break;

		case 0xd5:		// PUSH D
		{
			state->memory[state->sp - 2] = state->e;
			state->memory[state->sp - 1] = state->d;
			state->memory[state->sp] = state->sp - 2;
		}
		
		case 0xe1:		// POP	H
			state->l = state->memory[state->sp];
			state->h = state->memory[state->sp + 1];
			state->sp += 2;
			break;

		case 0xe5:		// PUSH	H
			state->memory[state->sp - 2] = state->l;
			state->memory[state->sp - 1] = state->h;
			state->sp -= 2;
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
		
		case 0xeb:		// XCHG
		{
			uint8_t temp_h = state->h;
			uint8_t temp_l = state->l;
			state->h = state->d;
			state->l = state->e;
			state->d = temp_h;
			state->e = temp_l;
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
		
		case 0xfb:		// EI
			state->int_enable = 1;

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
	printf("\tC=%d, P=%d,S=%d,Z=%d\n", state->cc.cy, state->cc.p, state->cc.s, state->cc.z);
	printf("\tA $%02x B $%02x C $%02x D $%02x E $%02x H $%02x L $%02x SP %04x\n",
		state->a, state->b, state->c, state->d, state->e, state->h, state->l, state->sp);
	return 0;
}
#endif
