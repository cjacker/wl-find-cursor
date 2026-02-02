{ config, lib, pkgs, stdenv, ... }:

stdenv.mkDerivation rec {
  pname = "wl-find-cursor";
  version = "unstable-2024-12-24";

  src = ./.;

  buildInputs = [
    pkgs.gnumake
    pkgs.gcc
    pkgs.wayland-scanner
    pkgs.wayland-protocols
    pkgs.wayland
    pkgs.wayland.dev
  ];
  buildPhase = ''
    mkdir -p tmp

    wayland-scanner client-header ${pkgs.wayland-protocols}/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml tmp/xdg-shell.h
    wayland-scanner public-code ${pkgs.wayland-protocols}/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml tmp/xdg-shell.c
    wayland-scanner client-header ${pkgs.wayland-protocols}/share/wayland-protocols/staging/single-pixel-buffer/single-pixel-buffer-v1.xml tmp/single-pixel-buffer-v1.h
    wayland-scanner public-code ${pkgs.wayland-protocols}/share/wayland-protocols/staging/single-pixel-buffer/single-pixel-buffer-v1.xml tmp/single-pixel-buffer-v1.c
    wayland-scanner client-header ${pkgs.wayland-protocols}/share/wayland-protocols/stable/viewporter/viewporter.xml tmp/viewporter.h
    wayland-scanner public-code ${pkgs.wayland-protocols}/share/wayland-protocols/stable/viewporter/viewporter.xml tmp/viewporter.c

    wayland-scanner client-header ./protocols/wlr-layer-shell-unstable-v1.xml tmp/wlr-layer-shell-unstable-v1.h
    wayland-scanner public-code ./protocols/wlr-layer-shell-unstable-v1.xml tmp/wlr-layer-shell-unstable-v1.c 
    wayland-scanner client-header ./protocols/wlr-virtual-pointer-unstable-v1.xml tmp/wlr-virtual-pointer-unstable-v1.h
    wayland-scanner public-code ./protocols/wlr-virtual-pointer-unstable-v1.xml tmp/wlr-virtual-pointer-unstable-v1.c

    gcc \
      -g \
      -o \
      wl-find-cursor \
      main.c \
      tmp/xdg-shell.c \
      tmp/wlr-layer-shell-unstable-v1.c \
      tmp/wlr-virtual-pointer-unstable-v1.c \
      tmp/single-pixel-buffer-v1.c \
      tmp/viewporter.c \
      -I./tmp \
      -lwayland-client
  '';
  installPhase = ''
    mkdir -p $out/bin
    install -m0755 wl-find-cursor $out/bin/
  '';
}