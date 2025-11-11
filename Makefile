CC		:= gcc
CFLAGS	:= -Wall -std=c11
TARGET	:= authInotifyIp
SRC		:= authInotifyIp.c

.PHONY : all build run clean

all: build

build: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $<
	
run: build
	@echo "[INFO] Running $(TARGET) (needs sudo for /var/log/auth.log)"
	sudo ./$(TARGET)

clean:
	rm -f $(TARGET)
