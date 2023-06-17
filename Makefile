LINK_FLAGS=-L"PATH/TO/MSYS2/mingw64/lib" -lWs2_32
CC=gcc

all: bin/mainXOR.exe

bin/mainXOR.exe: bin/mainXOR.o bin/networkLayer.o
	$(CC) bin/mainXOR.o bin/networkLayer.o -o bin/mainXOR.exe $(LINK_FLAGS)
	py tools/append_zip.py
	py tools/append_scripts.py
bin/mainXOR.o: src/mainXOR.c src/networkLayer.h
	$(CC) -c src/mainXOR.c -o bin/mainXOR.o
bin/networkLayer.o: src/networkLayer.c src/networkLayer.h
	$(CC) -c src/networkLayer.c -o bin/networkLayer.o

exe:
	@rm bin/mainXOR.o bin/networkLayer.o
clean:
	@rm bin/mainXOR.o bin/networkLayer.o bin/mainXOR.exe