CC = gcc
RM = rm -f
SRC = $(wildcard *.c)
CFLAGS = -std=c99 -Wall
JSMNDIR = $(CURDIR)/jsmn
JSMNLIB = $(JSMNDIR)/libjsmn.a
LDFLAGS = -lcurl -L$(JSMNDIR) -ljsmn
TARGET = coinget

all: $(TARGET)

$(TARGET): $(JSMNLIB)
	$(CC) $(CFLAGS) $(SRC) -o $@ $(LDFLAGS)

debug: $(JSMNLIB)
	$(CC) $(CFLAGS) -g $(SRC) -o $@ $(LDFLAGS)

$(JSMNLIB): libjsmn
	@# do nothing

libjsmn:
	$(MAKE) -C $(JSMNDIR)

clean:
	$(RM) $(TARGET)

.PHONY: all debug $(TARGET) clean

