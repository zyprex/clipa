# LAST MODIFIED : May 23, 2024
# MAKE PROGRAM  : mingw32-make
# PLATFORM      : windows
# DESCRIPTION   : this makefile is used to compile clipa 

CC:=gcc

CFLAGS:=-Wall -std=c99 \
	-finput-charset=UTF-8 \
	-fexec-charset=UTF-8

LINK:=-lgdi32

SRC:=clipa.c cfg.c

EXE:=clipa.exe

debug:
	@$(CC) $(CFLAGS) -g -DDEBUG $(SRC) -o $(EXE) $(LINK)
	@echo "clipa debug version...ok"
	@$(EXE)

ok:
	@$(CC) $(CFLAGS) $(SRC) -o $(EXE) $(LINK) -mwindows
	@echo "clipa ... ok"

kill:
	@taskkill /im $(EXE)

.PHONY:
clean:
	@del $(EXE)

