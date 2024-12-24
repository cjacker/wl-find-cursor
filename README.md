# wl-find-cursor

wl-find-cursor is a tool to highlight and print out global mouse position in wayland, especially for compositors based on wlroots, such as sway.

Due to security concerns, it's difficult to obtain the global mouse position in Wayland, that's to say, there is no `xeyes` for wayland.
This is generally not a problem. However, when we have multiple monitors and many editor windows open, 
it is sometimes necessary to quickly locate the mouse position.

This tool use layer-shell and virtual-pointer protocols to highlight and print out global mouse position in wayland. If the compositor lacks virtual pointer support, it can be worked around, but layer shell is essential.

**Since GNOME rejected to implement layer shell and virtual pointer protocol several years ago, it can not be supported by wl-find-cursor.**

![screenshot-2024-12-20-21-07-18](https://github.com/user-attachments/assets/daac6cb8-b9e5-4a35-ab90-8367342c23fd)

# Build and Installation

Before building, you may need to install `wayland-devel` and `wayland-protocols-devel` pkgs first, it depends on the distribution you use.

```
git clone https://github.com/cjacker/wl-find-cursor
make
```

After built, install `wl-find-cursor` to `PATH` (local path or global path such as /usr/bin, /usr/local/bin).

# Usage

```
wl-find-cursor: highlight and report cursor position in wayland.
Options:
  -s <int>    : animation square size.
  -a <hex int>: alpha value of color.
  -r <hex int>: red value of color.
  -g <hex int>: green value of color.
  -b <hex int>: blue value of color.
  -c <string> : cmd to emulate mouse event for compositor lack of virtual pointer support.
  -p          : skip animation, print out mouse coordinate in 'x y' format and exit
```

Run `wl-find-cursor` directly, it will draw an animation (a growing square) at the position of mouse cursor and exit. The duration is 1 second by default and the square color is default to `0xcfd79921`. It should be enough to locate the mouse cursor. Moving mouse will quit the animation immediatly.

If you only want to obtain the mouse coordinates, use `wl-find-cursor -p` to skip the animation.

If you need customize the animation duration, square size, square color, you can corresponding parameters, such as:

```
wl-find-cursor -a 0x88 -r 0xcc -g 0x24 -b 0x1d -s 400 -d 2
```

It will show a semi-transparent red square with size 400, the duration is 2 second.

Usually you should bind it with a hot key such as `Super+m`, for example, for sway:

```
bindsym $mod+m exec wl-find-cursor
```

You can also use it with other tools such as grim and slurp:

```
wl-find-cursor && grim -g "$(slurp -d)"
```

### for KDE user

It seems kwin_wayland didn't implement virtual pointer protocol, but had layer shell support. I make a workaround for it, wl-find-cursor can accept a cmd to emulate mouse move event, for example, with 'ydotool':

```
wl-find-cursor -c "ydotool mousemove 0 1" -p
```

# How to enlarge the cursor size

A large cursor will make things easier, usually I set cursor size to 64:

```
gsettings set org.gnome.desktop.interface cursor-size 64
swaymsg seat seat0 xcursor_theme "$(gsettings get org.gnome.desktop.interface cursor-theme)" 64
```

And append `Xcursor.size: 64` to `~/.Xresources`.

Above settings should work well with all common applications include wayland/X/gtk and qt.

# And more

**a config file?**

It is really a simple and single-source tool, I prefer no config file. 

If you want to change the color or change the animation duration, please change the codes directly, the RGBA and DURATION were defined at the top of source.

**ugly codes**

I drafted this in a few dozen minutes and it works.

If you really hate global variables, go ahead and put them in some structures and pass these structures everywhere.😄️

**why square?**

I use i3/sway for more than ten years, everything in i3/sway is flat rectangle.

And I don't want to introduce cairo dependency， also don't want to handle anti-alias issue manually, drawing square is toooo much simpler than circle.
