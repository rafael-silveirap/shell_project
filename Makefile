# variaveis
CC = gcc
CFLAGS = -Wall -Wextra -g
TARGET = my_shell

# compila
all: $(TARGET)

$(TARGET): main.c
	$(CC) $(CFLAGS) main.c -o $(TARGET) -lreadline

# limpa
clean:
	rm -f $(TARGET)


run: all
	./$(TARGET)