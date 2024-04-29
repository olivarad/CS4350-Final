# CS4350-Final

When running the executable the ports will have to be swapped to allow for messaging. The following lines will also have to be uncommented. (This should already be taken care of as there are multiple config files but sometimes the config file in debug is not resected so change both if you have issues)
#sharedmultimediapath=../../../../shared/mm/
#localmultimediapath=../mm/

Ensure that you run the executable after running the solution.

The mm folder MUST be copied into the cwin64 folder as the aftr cannot set the local multimedia folder in aftr.conf (there is a setting for it but it is broken in the engine)

Controls:

WASD, Space, and LCtrl movement
clicking esc will exit any gui being used or end the module
left click to select objects or confirm object location for instancing
right click to cancel placing objects
9 enables networking

