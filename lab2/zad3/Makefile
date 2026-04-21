CC = gcc
CFLAGS = -Wall -fPIC
LIB_SOURCES = sig_default.c sig_mask.c sig_ignore.c sig_handle.c sig_unblock.c
LIB_OBJECTS = $(LIB_SOURCES:.c=.o)
LIBNAME = signals

.PHONY: all clean dynamic

all: dynamic

# Tworzenie biblioteki współdzielonej
lib$(LIBNAME).so: $(LIB_OBJECTS)
	$(CC) -shared -o $@ $^

# Kompilacja plików .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# REGUŁA DYNAMIC - tworzy main i child
dynamic: lib$(LIBNAME).so
	$(CC) $(CFLAGS) main.c -o main
	$(CC) $(CFLAGS) -D DYNAMIC_LOAD child.c sig_unblock.c -o child -ldl -Wl,-rpath,.

clean:
	rm -f *.o *.a *.so main child