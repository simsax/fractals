build = debug

TARGET_EXE = fractal
INCDIRS = ./src
CODEDIRS = ./src
BUILD_DIR = ./build
CFLAGS = -std=c11 -Wall -Wextra -pedantic -MP -MMD

ifeq ($(build), debug)
	CFLAGS += -O0 -g
else # release
	CFLAGS += -O3 -DNDEBUG
endif

CFLAGS += $(foreach D,$(INCDIRS),-I$(D))

SRCS = $(foreach D,$(CODEDIRS),$(wildcard $(D)/*.c))
OBJS = $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS = $(OBJS:.o=.d)

LIBS = -lm

all: $(BUILD_DIR)/$(TARGET_EXE)

$(BUILD_DIR)/$(TARGET_EXE): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LIBS)

$(BUILD_DIR)/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@


.PHONY: clean
clean:
	rm -r $(BUILD_DIR)

-include $(DEPS)