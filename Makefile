include config.mk

TARGET_DIR = $(PREFIX)/bin

BUILD_DIR = build

TARGET = czakoc

SUB_DIRS = lib
SRC = $(wildcard *.c $(addsuffix /*.c,$(SUB_DIRS)))
OBJ = $(addprefix $(BUILD_DIR)/,$(SRC:.c=.o))
OBJ_DIRS = $(BUILD_DIR) $(addprefix $(BUILD_DIR)/,$(SUB_DIRS))
OBJ_DEPS = $(addprefix $(BUILD_DIR)/,$(SRC:.c=.d))

CC_CMD = $(CC) $(CFLAGS) -g3 -c -o $@ $<

all: gen/ast.h $(TARGET)

$(OBJ_DIRS):
	mkdir -p $@

$(BUILD_DIR)/%.o: %.c | $(OBJ_DIRS)
	@echo "  CC    " $@
	@$(CC_CMD) -MMD

$(TARGET): $(OBJ)
	@echo "  LD    " $@
	@$(CC) -o $@ $(OBJ) $(LDFLAGS)

clean:
	@echo "  CLEAN"
	@rm -f $(OBJ) $(TARGET)

clean-all: clean clean-gen
clean-gen:
	rm -f gen/ast.h

install:
	cp -f $(TARGET) $(TARGET_DIR)/$(TARGET)

uninstall:
	rm -f $(TARGET_DIR)/$(TARGET)

gen/%: gen/%.c
	@echo "  TOOL  " $@
	@$(CC) $(CFLAGS) -g3 -MMD -o $@ $<

gen/ast.h: gen/ast gen/ast.def
	@echo "  GEN   " $@
	@gen/ast < gen/ast.def > $@

%.h:
	@:
ifeq (,$(filter clean,$(MAKECMDGOALS)))
-include $(OBJ_DEPS)
-include gen/ast.d
endif

.PHONY: all clean clean-all clean-gen install uninstall
