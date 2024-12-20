# wl-find-cursor
wl-find-cursor is a tool to highlight and print out mouse position in wayland.

Due to security concerns, it's difficult to obtain the global mouse position in Wayland. 
This is generally not a problem. However, when we have multiple monitors and many editor windows open, 
it is sometimes necessary to quickly locate the mouse position.

![screenshot-2024-12-20-21-07-18](https://github.com/user-attachments/assets/daac6cb8-b9e5-4a35-ab90-8367342c23fd)


# Build and Installation
```
git clone https://github.com/cjacker/wl-find-cursor
make
```

You may need to install wayland-devel or wayland-dev pkgs, it's depend on which distribution you use.

And after built, install `wl-find-cursor` to PATH (your local path or global path such as /usr/bin).


# Usage

Run `wl-find-cursor` directly, it will draw an animation (a rectangle) surround mouse cursor, the duration is 1 second,it should be enough to locate the mouse cursor, and move mouse will quit the animation immediatly.

If you only want to obtain the mouse coordinates, use `wl-find-cursor -p` to skip the animation.


# And more

**config file?**

This is a really simple and single-source tool, I prefer no config files for it. 

If you want to change the color or change the animation duration, please change the codes directly, I defined RGBA and DURATION at the top of source.

**so ugly codes**

I drafted this in just a few dozen minutes and it works.

If you really hate global variables, go ahead and put them in structures😊️

**why square?**

I use i3/sway for more than ten years, everything is flat rectangle.

And drawing sqaure is toooo much simpler than circle.

