TITLE = xdashy
OBJ := 

include src/Makefile-part

CC := gcc
CFLAGS := -O3 -ffast-math -std=gnu99 -pedantic -Wall `sdl-config --cflags` 
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
