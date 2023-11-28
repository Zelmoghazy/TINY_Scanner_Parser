CC=gcc

INCS=-I.
OPT=-O2
DBG=

WARNINGS=-Wall -Wextra -Wsign-conversion -Wconversion -pedantic

CFLAGS=$(DBG) $(OPT) $(INCS) $(WARNINGS)

SRC=$(foreach DIR,$(CODEDIRS),$(wildcard $(DIR)/*.c))

OBJ=$(addprefix $(BUILD_DIR)/,$(notdir $(SRC:.c=.o)))

LIB=

BUILD_DIR=build
CODEDIRS=.

PROJ=Main

all: $(BUILD_DIR)/$(PROJ).exe
	@echo "========================================="
	@echo "              BUILD SUCCESS              "
	@echo "========================================="

$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	$(CC) -c $(CFLAGS) $^ -o $@

$(BUILD_DIR)/$(PROJ).exe: $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@ 

$(BUILD_DIR):
	mkdir $@

clean:
	rm -fR $(BUILD_DIR)