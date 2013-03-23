CC := gcc
LIBTOOL := libtool

ifeq ($(PREFIX),)
  LIB_INSTALL_DIR = $(HOME)/.purple/plugins
else
  LIB_INSTALL_DIR = $(PREFIX)/lib/purple-2/
endif

PLUGIN_NAME = autorespond

PURPLE_CFLAGS  = $(shell pkg-config purple --cflags)
PURPLE_LIBS    = $(shell pkg-config purple --libs)

LUA_CFLAGS  = $(shell pkg-config lua5.2 --cflags)
LUA_LIBS    = $(shell pkg-config lua5.2 --libs)

all: $(PLUGIN_NAME).so

install: all
	mkdir -p $(LIB_INSTALL_DIR)
	cp $(PLUGIN_NAME).so $(LIB_INSTALL_DIR)

$(PLUGIN_NAME).so: $(PLUGIN_NAME).o
	$(CC) -shared $(CFLAGS) $< -o $@ $(LUA_LIBS) $(PURPLE_LIBS) $(PIDGIN_LIBS) $(GTK_LIBS) -Wl,--export-dynamic -Wl,-soname

$(PLUGIN_NAME).o:$(PLUGIN_NAME).c 
	$(CC) $(CFLAGS) -fPIC -c $< -o $@ $(LUA_CFLAGS) $(PURPLE_CFLAGS) $(PIDGIN_CFLAGS) $(GTK_CFLAGS) -DHAVE_CONFIG_H

clean:
	rm -rf *.o *.c~ *.h~ *.so *.la .libs

user:
	cp -r purple/* $(HOME)/.purple/
	mv $(HOME)/.purple/$(PLUGIN_NAME).sh $(HOME)/.purple/$(PLUGIN_NAME)
