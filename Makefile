CC = gcc
TAG = ctags
RM = rm -f
SRC = $(wildcard *.c)
CFLAGS = -std=c99 -Wall
JSMNDIR = $(CURDIR)/jsmn
JSMNLIB = $(JSMNDIR)/libjsmn.a
LDFLAGS = -lcurl -L$(JSMNDIR) -ljsmn
TARGET = coinget

INSTDIR = /usr/local/bin

OS := $(shell uname)
ifeq ($(OS), Darwin)
    LDFLAGS +=  -largp
endif

all: $(TARGET)

$(TARGET): $(JSMNLIB)
	$(CC) $(CFLAGS) $(SRC) -o $@ $(LDFLAGS)

debug: $(JSMNLIB)
	$(CC) $(CFLAGS) -g $(SRC) -o $@ $(LDFLAGS)

$(JSMNLIB): libjsmn
	@# do nothing

libjsmn:
	$(MAKE) -C $(JSMNDIR)

tags:
	$(TAG) *.c

install: $(TARGET)
	install -m 0755 $^ $(INSTDIR)

uninstall:
	rm -f $(INSTDIR)/$(TARGET)

clean:
	$(RM) $(TARGET)

.PHONY: all $(TARGET) tags install uninstall clean

