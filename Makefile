CC := gcc

CFLAGS := -g -Wall -Werror
LDFLAGS :=
INCLUDE := 

BUILD_DIR := build
INSTALL_DIR := .
EXECUTABLE := registerLanServer

OBJ := $(BUILD_DIR)/register_lan_server.o $(BUILD_DIR)/tun_open.o

.PHONY: all clean install

all: $(BUILD_DIR)/$(EXECUTABLE)

install: $(INSTALL_DIR)/$(EXECUTABLE)

$(INSTALL_DIR)/%: $(BUILD_DIR)/%
	cp $^ $@

$(BUILD_DIR)/$(EXECUTABLE): $(OBJ) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	$(CC) -c $(CFLAGS) $(INCLUDE) -o $@  $^

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm -f $(BUILD_DIR)/*.o $(BUILD_DIR)/$(EXECUTABLE)