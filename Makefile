DESTDIR=
PREFIX=/usr/local

all: headers
	gcc -g -o wl-find-cursor main.c tmp/xdg-shell.c tmp/wlr-layer-shell-unstable-v1.c tmp/wlr-virtual-pointer-unstable-v1.c tmp/single-pixel-buffer-v1.c tmp/viewporter.c -I./tmp -lwayland-client

headers:
	mkdir -p tmp
	wayland-scanner client-header /usr/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml tmp/xdg-shell.h
	wayland-scanner public-code /usr/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml tmp/xdg-shell.c
	wayland-scanner client-header ./protocols/wlr-layer-shell-unstable-v1.xml tmp/wlr-layer-shell-unstable-v1.h
	wayland-scanner public-code ./protocols/wlr-layer-shell-unstable-v1.xml tmp/wlr-layer-shell-unstable-v1.c 
	wayland-scanner client-header ./protocols/wlr-virtual-pointer-unstable-v1.xml tmp/wlr-virtual-pointer-unstable-v1.h
	wayland-scanner public-code ./protocols/wlr-virtual-pointer-unstable-v1.xml tmp/wlr-virtual-pointer-unstable-v1.c
	wayland-scanner client-header /usr/share/wayland-protocols/staging/single-pixel-buffer/single-pixel-buffer-v1.xml tmp/single-pixel-buffer-v1.h
	wayland-scanner public-code /usr/share/wayland-protocols/staging/single-pixel-buffer/single-pixel-buffer-v1.xml tmp/single-pixel-buffer-v1.c
	wayland-scanner client-header /usr/share/wayland-protocols/stable/viewporter/viewporter.xml tmp/viewporter.h
	wayland-scanner public-code /usr/share/wayland-protocols/stable/viewporter/viewporter.xml tmp/viewporter.c


install: default
	mkdir -p $(DESTDIR)/$(PREFIX)/bin
	install -m0755 wl-find-cursor $(DESTDIR)/$(PREFIX)/bin/

clean:
	rm -f wl-find-cursor
	rm -rf ./tmp 

