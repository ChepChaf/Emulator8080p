#include "disassemble8080.cpp"
#include "Emulator8080p.cpp"
#include <stdio.h>
#include <stdlib.h>

int main (int argc, char** argv)
{
	FILE *file = fopen (argv[1], "rb");

	if (file == NULL)
	{
		printf("error: Couldn't open %s\n", argv[1]);
		exit(1);
	}

	fseek (file, 0L, SEEK_END);
	int fsize = ftell(file);
	fseek(file, 0L, SEEK_SET);

	unsigned char *buffer = (unsigned char*) malloc(fsize);

	fread (buffer, fsize, 1, file);
	fclose(file);

	int pc = 0;

	while (pc < fsize)
	{
		pc += Disassemble8080p (buffer, pc);
	}
	return 0;
}


