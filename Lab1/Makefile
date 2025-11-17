CC=gcc
CFLAGS=-Wall -O2
TARGET=index.exe
SRC=index.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	del $(TARGET) result.txt
