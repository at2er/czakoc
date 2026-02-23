include config.mk

TARGET_DIR = $(PREFIX)/lib

BUILD_DIR = build

TARGET = czakoc

SUB_DIRS = src
SRC = $(wildcard *.c $(addsuffix /*.c,$(SUB_DIRS)))
OBJ = $(addprefix $(BUILD_DIR)/,$(SRC:.c=.o))
OBJ_DIRS = $(BUILD_DIR) $(addprefix $(BUILD_DIR)/,$(SUB_DIRS))
OBJ_DEPS = $(addprefix $(BUILD_DIR)/,$(SRC:.c=.d))

CC_CMD = $(CC) $(CFLAGS) -g3 -c -o $@ $<

all: $(TARGET)

$(OBJ_DIRS):
	mkdir -p $@

$(BUILD_DIR)/%.o: %.c | $(OBJ_DIRS)
	@echo "  CC    " $@
	@$(CC_CMD) -MMD

$(TARGET): $(OBJ)
	@echo "  LD    " $@
	@$(CC) $(LDFLAGS) -o $@ $(OBJ)

clean:
	@echo "  CLEAN"
	@rm -f $(OBJ) $(TARGET)

install:
	cp -f $(TARGET) $(TARGET_DIR)/$(TARGET)

uninstall:
	rm -f $(TARGET_DIR)/$(TARGET)

%.h:
	@:
ifeq (,$(filter clean,$(MAKECMDGOALS)))
-include $(OBJ_DEPS)
endif

.PHONY: all clean install uninstall
