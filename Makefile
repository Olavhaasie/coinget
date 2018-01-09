CC ?= gcc
TAG = ctags

SRC = $(wildcard src/*.c)
GIT_VERSION := $(shell git describe --abbrev=0 --tags)
TARGET = coinget

JSMNDIR = $(CURDIR)/jsmn
JSMNLIB = $(JSMNDIR)/libjsmn.a

CFLAGS = -std=c99 -Wall -Werror -pedantic -I$(JSMNDIR) -DVERSION="\"$(TARGET) $(GIT_VERSION)\""
LDFLAGS = -lcurl -L$(JSMNDIR) -ljsmn

INSTDIR = /usr/local
INSTBIN = $(INSTDIR)/bin
INSTMAN = $(INSTDIR)/man/man1
MANPAGE = $(TARGET).1

OS := $(shell uname)
ifeq ($(OS), Darwin)
    LDFLAGS = -lcurl -L$(JSMNDIR) -ljsmn -largp
    INSTMAN = $(INSTDIR)/share/man/man1
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
	$(TAG) src/*.h src/*.c

install: $(TARGET)
	test -d $(INSTBIN) || mkdir -p $(INSTBIN)
	test -d $(INSTMAN) || mkdir -p $(INSTMAN)
	install -m 0755 $^ $(INSTBIN)
	install -m 0644 doc/$(MANPAGE) $(INSTMAN)

uninstall:
	$(RM) -f $(INSTBIN)/$(TARGET)
	$(RM) -f $(INSTMAN)/$(MANPAGE)

clean:
	$(RM) $(TARGET)
	$(RM) tags
	$(MAKE) clean -C $(JSMNDIR)


.PHONY: all $(TARGET) libjsmn install uninstall clean

