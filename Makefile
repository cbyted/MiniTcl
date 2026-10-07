CC = gcc

CFLAGS = -Wall -Wextra -Iinclude
LDFLAGS =
LIBS =

SECURITY_FLAGS = -O2 -fstack-protector-strong -fPIE -D_FORTIFY_SOURCE=2
RELEASE_LDFLAGS = -pie -Wl,-z,relro,-z,now

DEBUG_FLAGS = -g -O0 -fno-omit-frame-pointer -fno-stack-protector
DEBUG_LDFLAGS = -no-pie

TARGET = minitcl
BUILD_DIR = bin

SOURCES = $(shell find src -type f -name '*.c')
OBJECTS = $(patsubst src/%.c,$(BUILD_DIR)/%.o,$(SOURCES))

.PHONY: all release debug clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ $(LIBS) -o $@

release:
	$(MAKE) clean
	$(MAKE) CFLAGS="$(CFLAGS) $(SECURITY_FLAGS)" \
	        LDFLAGS="$(LDFLAGS) $(RELEASE_LDFLAGS)" \
	        $(TARGET)

debug:
	$(MAKE) clean
	$(MAKE) CFLAGS="$(CFLAGS) $(DEBUG_FLAGS)" \
	        LDFLAGS="$(LDFLAGS) $(DEBUG_LDFLAGS)" \
	        $(TARGET)

$(BUILD_DIR)/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

