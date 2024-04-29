# CS4350-Final

When running the executable the ports will have to be swapped to allow for messaging. The following line will have to be uncommented in aftr.conf.
#sharedmultimediapath=../../../../shared/mm/

Ensure that you run the executable after running the solution.

The mm folder MUST be copied into the cwin64 folder as the aftr cannot set the local multimedia folder in aftr.conf (there is a setting for it but it is broken in the engine)

Controls:

WASD, Space, and LCtrl movement
clicking esc will exit any gui being used or end the module
left click to select objects or confirm object location for instancing
right click to cancel placing objects
9 enables networking

CMake will also have to be modified for GDAL and FREETYPE, do this in the GUI and follow the video on youtube as a tutorial if needed.