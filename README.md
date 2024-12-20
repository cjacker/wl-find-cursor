# wl-find-cursor
wl-find-cursor is a tool to highlight and print out global mouse position in wayland.

Due to security concerns, it's difficult to obtain the global mouse position in Wayland. 
This is generally not a problem. However, when we have multiple monitors and many editor windows open, 
it is sometimes necessary to quickly locate the mouse position.

This tool use layer-shell and virtual-pointer protocols to get global mouse position in wayland.

![screenshot-2024-12-20-21-07-18](https://github.com/user-attachments/assets/daac6cb8-b9e5-4a35-ab90-8367342c23fd)


# Build and Installation

Before building, you may need to install `wayland-devel` or `libwayland-dev` pkgs first, it's depend on which distribution you use.

```
git clone https://github.com/cjacker/wl-find-cursor
make
```

After built, install `wl-find-cursor` to `PATH` (your local path or global path such as /usr/bin, /usr/local/bin).

# Usage

Run `wl-find-cursor` directly, it will draw an animation (a rectangle) at mouse cursor and exit. The duration is 1 second by default. It should be enough to locate the mouse cursor. Moving mouse will quit the animation immediatly.

If you only want to obtain the mouse coordinates, use `wl-find-cursor -p` to skip the animation.

You can also use it with other tools such as grim and slurp:

```
wl-find-cursor && grim -g "$(slurp -d)"
```

# How to enlarge the cursor size

Usually I set cursor size to 64:
```
gsettings set org.gnome.desktop.interface cursor-size 64
swaymsg seat seat0 xcursor_theme "$(gsettings get org.gnome.desktop.interface cursor-theme)" 64
```
And append `Xcursor.size: 64` to `~/.Xresources`.

Above settings should work well with all applications include wayland/X/gtk and qt.

# And more

**config file?**

This is a really simple and single-source tool, I prefer no config files for it. 

If you want to change the color or change the animation duration, please change the codes directly, I defined RGBA and DURATION at the top of source.

**ugly codes**

I drafted this in a few dozen minutes and it works.

If you really hate global variables, go ahead and put them in some structures😊️

**why square?**

I use i3/sway for more than ten years, everything in i3/sway is flat rectangle.

And I don't want to introduce cairo dependency, drawing sqaure is toooo much simpler than circle.

With cairo, you can draw something more complex very easy.

