CC = gcc
TAG = ctags
RM = rm -f

SRC = $(wildcard *.c)
GIT_VERSION := $(shell git describe --abbrev=0 --tags)
TARGET = coinget

CFLAGS = -std=c99 -Wall -DVERSION="\"$(TARGET) $(GIT_VERSION)\""
JSMNDIR = $(CURDIR)/jsmn
JSMNLIB = $(JSMNDIR)/libjsmn.a
LDFLAGS = -lcurl -L$(JSMNDIR) -ljsmn

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
	$(TAG) *.h *.c

install: $(TARGET)
	install -m 0755 $^ $(INSTDIR)

uninstall:
	rm -f $(INSTDIR)/$(TARGET)

clean:
	$(RM) $(TARGET)

.PHONY: all $(TARGET) tags install uninstall clean

