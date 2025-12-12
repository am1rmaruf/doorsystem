PROG = Main.exe
SRC = Main.c safeinput.c
CFLAGS = -g
LIBS =

all: $(PROG)

$(PROG): $(SRC)
	$(CC) -o $@ $(CFLAGS) $(SRC) $(LIBS)

clean:
	del /F /Q $(PROG) 2>nul || echo Nothing to delete.

.PHONY: all clean
