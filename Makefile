CFLAGS := -g -std=c11 -Wall -Wextra -pedantic -Wwrite-strings

GENERAL_LIBRARIES := -lm -lpthread
GRAPHICS_FLAGS := -lGLEW -lglfw3 -lGL -lX11 -lXrandr -lXi -lXxf86vm \
				  -ldl -lXinerama -lXcursor -lrt

INCLUDES_CLONED := -Igl3w/include
INCLUDES := -Iincludes

CC := gcc

DIR_BIN := bin

H_FILES := $(wildcard includes/*.h)

# Look for *.c files in gl3w/src.
vpath %.c gl3w/src

all: folders $(DIR_BIN)/game

folders:
# Check if gl3w is present.
	@[ -d "./gl3w" ] || { \
		git clone https://github.com/skaslev/gl3w && \
		cd gl3w && \
		./gl3w_gen.py \
	;}
# Check if bin exists.
	@[ -d $(DIR_BIN) ] || mkdir $(DIR_BIN)

$(DIR_BIN)/game : game.c gl3w.c lib.c $(H_FILES)
	$(CC) $(filter %.c,$^) -o $@ $(CFLAGS) $(INCLUDES) $(INCLUDES_CLONED) $(GENERAL_LIBRARIES) $(GRAPHICS_FLAGS)