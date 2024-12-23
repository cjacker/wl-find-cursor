# wl-find-cursor

wl-find-cursor is a tool to highlight and print out global mouse position in wayland, especially for compositors based on wlroots.

Due to security concerns, it's difficult to obtain the global mouse position in Wayland, that's to say, there is no `xeyes` for wayland.
This is generally not a problem. However, when we have multiple monitors and many editor windows open, 
it is sometimes necessary to quickly locate the mouse position.

This tool use layer-shell and virtual-pointer protocols to highlight and print out global mouse position in wayland.

**Since KDE/GNOME didn't implement the virtual pointer protocol, they cannot be supported by wl-find-cursor now. These compositors have their own methods to locate the mouse cursor or retrieve its position.**

![screenshot-2024-12-20-21-07-18](https://github.com/user-attachments/assets/daac6cb8-b9e5-4a35-ab90-8367342c23fd)


# Build and Installation

Before building, you may need to install `wayland-devel` and `wayland-protocols-devel` pkgs first, it depends on the distribution you use.

```
git clone https://github.com/cjacker/wl-find-cursor
make
```

After built, install `wl-find-cursor` to `PATH` (local path or global path such as /usr/bin, /usr/local/bin).

# Usage

Run `wl-find-cursor` directly, it will draw an animation (a growing square) at the position of mouse cursor and exit. The duration is 1 second by default. It should be enough to locate the mouse cursor. Moving mouse will quit the animation immediatly.

If you only want to obtain the mouse coordinates, use `wl-find-cursor -p` to skip the animation.

Usually you should bind it with a hot key such as `Super+m`, for example, for sway:

```
bindsym $mod+m exec wl-find-cursor
```

You can also use it with other tools such as grim and slurp:

```
wl-find-cursor && grim -g "$(slurp -d)"
```

# How to enlarge the cursor size

A large cursor will make things easier, usually I set cursor size to 64:

```
gsettings set org.gnome.desktop.interface cursor-size 64
swaymsg seat seat0 xcursor_theme "$(gsettings get org.gnome.desktop.interface cursor-theme)" 64
```

And append `Xcursor.size: 64` to `~/.Xresources`.

Above settings should work well with all common applications include wayland/X/gtk and qt.

# GNOME/KDE issue

As mentioned above, these compositors didn't implement wayland virtual-pointer procotol, but provide different way to find the cursor or get cursor position.

for GNOME:

```
Settings > Accessibility > Pointing & Clicking > Locate Pointer
```

for KDE:

Run kwin script as below:
```
print("Mouse position x=" + workspace.cursorPos.x + " y=" + workspace.cursorPos.y)
```

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
