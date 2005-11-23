TITLE = xdashy
OBJ := 

include src/Makefile-part

CC := gcc
CFLAGS := -g -O3 -std=gnu99 -pedantic -Wall `sdl-config --cflags` 
LDFLAGS := `sdl-config --libs` -lSDL_ttf -lSDL_image -ljpeg -lpng -lz -lfreetype 

.PHONY: all
all: linux

.PHONY: linux
linux: $(TITLE)

$(TITLE): $(OBJ)
	$(CC) -o $(TITLE) $(OBJ) $(LDFLAGS)

.PHONY: clean
clean: 
	$(RM) $(OBJ) $(TITLE)
