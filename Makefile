main:
	gcc -o Emulator8080 main.cpp
default:
	make main
clean:
	rm -f *.o
