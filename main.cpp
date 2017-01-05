#include "disassemble8080.cpp"
#include "Emulator8080p.cpp"
#include <stdio.h>
#include <stdlib.h>

State8080* InitState()
{
	State8080* state = (State8080*) calloc(1, sizeof(State8080));
	state->memory = (uint8_t*) malloc(0x10000); // 16K memory
	return state;
}

void ReadFileIntoMemoryAt(State8080* state, char* filename, uint32_t offset)
{
	FILE *file = fopen (filename, "rb");
	if (file == NULL)
	{
		printf("error: Couldn't open %s\n", filename);
		exit(1);
	}
	fseek (file, 0L, SEEK_END);
	int fsize = ftell(file);
	fseek(file, 0L, SEEK_SET);

	uint8_t *buffer = &state->memory[offset];
	fread (buffer, fsize, 1, file);
	fclose(file);

}

int main (int argc, char** argv)
{
	State8080* state = InitState();
	
	bool done = 0;
	char opcion;

	ReadFileIntoMemoryAt(state, "invaders/invaders.h", 0);
	ReadFileIntoMemoryAt(state, "invaders/invaders.g", 0x800);
	ReadFileIntoMemoryAt(state, "invaders/invaders.f", 0x1000);
	ReadFileIntoMemoryAt(state, "invaders/invaders.e", 0x1800);
	bool debug = false;
	printf("Opcion: ");
	scanf("%d", &opcion);
	if(opcion == 'd')
		debug = true;
	while (done == 0)
	{
		done = Emulate8080p(state);
		if(debug)
			getchar();
	}
	return 0;
}


