CC = gcc
CFLAGS = -Wall -std=c11

SDL2_INCLUDE = include
SDL2_LIB = lib

IFLAGS = -I$(SDL2_INCLUDE)
LFLAGS = -L$(SDL2_LIB) -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf -lm

SRC_DIR = src
BUILD_DIR = build

TARGET = SandScape.exe
TEST_TARGET = test.exe

SOURCES = main.c game.c simulation.c color_palettes.c
TEST_SOURCES = test.c game.c simulation.c color_palettes.c
OBJECTS = $(patsubst %.c,$(BUILD_DIR)/%.o,$(SOURCES))
TEST_OBJECTS = $(patsubst %.c,$(BUILD_DIR)/%.o,$(TEST_SOURCES))

APP = $(BUILD_DIR)/$(TARGET)
TEST_APP = $(BUILD_DIR)/$(TEST_TARGET)

all: $(APP)

$(APP): $(OBJECTS)
	$(CC) $(CFLAGS) $(IFLAGS) -o $@ $^ $(LFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(IFLAGS) -c -o $@ $<

run: $(APP)
	./$(APP)

test: $(TEST_APP)
	./$(TEST_APP)

$(TEST_APP): $(TEST_OBJECTS)
	$(CC) $(CFLAGS) $(IFLAGS) -o $@ $^ $(LFLAGS)

clean:
	rm -rf $(TEST_OBJECTS) $(TEST_TARGET)