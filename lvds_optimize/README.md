wl-lvds

LVDS capture program for Wayland based on weston-simple-shm

Dependencies

- wayland-client
- egl
- opengl es 2.0

Build

    $ cd wl-lvds
    $ make

Run

    $ ./wl-lvds

Or, in order to specify video capture device execute below

    $ ./wl-lvds --device /dev/videoX

(/dev/videoX is path to video capture device)

