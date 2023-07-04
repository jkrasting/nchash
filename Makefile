CFLAGS = -I$(CONDA_PREFIX)/include
LDFLAGS = -L$(CONDA_PREFIX)/lib
LDLIBS = -lnetcdf -lcrypto
TARGET = nchash
OBJECTS = nchash.o

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJECTS) $(TARGET)
