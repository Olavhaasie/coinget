CC = gcc
TAG = ctags
RM = rm -f

SRC = $(wildcard src/*.c)
GIT_VERSION := $(shell git describe --abbrev=0 --tags)
TARGET = coinget

JSMNDIR = $(CURDIR)/jsmn
JSMNLIB = $(JSMNDIR)/libjsmn.a

CFLAGS = -std=c99 -Wall -I$(JSMNDIR) -DVERSION="\"$(TARGET) $(GIT_VERSION)\""
LDFLAGS = -lcurl -L$(JSMNDIR) -ljsmn

INSTDIR = /usr/local
INSTBIN = $(INSTDIR)/bin
INSTMAN = $(INSTDIR)/man/man1
MANPAGE = $(TARGET).1

OS := $(shell uname)
ifeq ($(OS), Darwin)
    LDFLAGS = -lcurl -L$(JSMNDIR) -ljsmn -largp
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
	test -d $(INSTBIN) || mkdir -p $(INSTBIN)
	test -d $(INSTMAN) || mkdir -p $(INSTMAN)
	install -m 0755 $^ $(INSTBIN)
	install -m 0644 doc/$(MANPAGE) $(INSTMAN)

uninstall:
	rm -f $(INSTBIN)/$(TARGET)
	rm -f $(INSTMAN)/$(MANPAGE)

clean:
	$(RM) $(TARGET)

.PHONY: all $(TARGET) libjsmn tags install uninstall clean

