TITLE = xdashy
OBJ := 

include src/Makefile-part

CC  = gcc
 
CC_FLAGS  = -O3 -std=gnu99 

INCLUDE  = -I/usr/include/SDL

LD_DIRS = -L/usr/lib/SDL 
LD_LIBS  = $(LD_DIRS) -lSDL_ttf -lSDL_image -ljpeg -lpng -lz -lSDL -lfreetype 

.PHONY: all
all: linux

.c.o:
	$(CC) -c -o $@ $(CC_FLAGS) $(INCLUDE) $<

.PHONY: linux
linux: $(OBJ) 
	$(CC) -o $(TITLE) $(OBJ) $(LD_LIBS)

.PHONY: clean
clean: 
	rm -f $(OBJ) $(TITLE) 

