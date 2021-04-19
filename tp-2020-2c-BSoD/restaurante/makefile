TARGET = restaurante
LIBS = -lcommons -lpthread -lm -lutils
CC = gcc
CFLAGS = -g -Wall

.PHONY: default all clean

default: $(TARGET) # la regla por defecto (tp0)
all: default # Si ejecutan all, que ejecute la default

# Definamos un par de constantes mas, OBJECTS (que vamos a usar para generar los .o)
# y HEADERS, para incluir los headers que definamos.
# Nota: wildcard implica que seleccione todos los que cumplen con la regex (osea todos los que terminen con .h)

OBJECTS = $(patsubst %.c, %.o, $(wildcard *.c))
HEADERS = $(wildcard *.h)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) $(CFLAGS) $(LIBS) -o $@
id:
	-rm -f team_id1.txt team_id2.txt team_id4.txt
clean:
	-rm -f *.o
	-rm -f $(TARGET)
