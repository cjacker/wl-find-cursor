default:
	mkdir -p proto
	wayland-scanner client-header /usr/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml proto/xdg-shell.h
	wayland-scanner public-code /usr/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml proto/xdg-shell.c
	wayland-scanner client-header ./wlr-layer-shell-unstable-v1.xml proto/wlr-layer-shell-unstable-v1.h
	wayland-scanner public-code ./wlr-layer-shell-unstable-v1.xml proto/wlr-layer-shell-unstable-v1.c 
	wayland-scanner client-header ./wlr-virtual-pointer-unstable-v1.xml proto/wlr-virtual-pointer-unstable-v1.h
	wayland-scanner public-code ./wlr-virtual-pointer-unstable-v1.xml proto/wlr-virtual-pointer-unstable-v1.c
	wayland-scanner client-header /usr/share/wayland-protocols/staging/single-pixel-buffer/single-pixel-buffer-v1.xml proto/single-pixel-buffer-v1.h
	wayland-scanner public-code /usr/share/wayland-protocols/staging/single-pixel-buffer/single-pixel-buffer-v1.xml proto/single-pixel-buffer-v1.c
	wayland-scanner client-header /usr/share/wayland-protocols/stable/viewporter/viewporter.xml proto/viewporter.h
	wayland-scanner public-code /usr/share/wayland-protocols/stable/viewporter/viewporter.xml proto/viewporter.c
	gcc -I./proto -o wl-find-cursor main.c proto/xdg-shell.c proto/wlr-layer-shell-unstable-v1.c proto/wlr-virtual-pointer-unstable-v1.c proto/single-pixel-buffer-v1.c proto/viewporter.c -lwayland-client

clean:
	rm -f wl-find-cursor
	rm -rf proto

