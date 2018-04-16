
Rapid prototyping with [Bosch XDK](https://xdk.bosch-connectivity.com/) hardware and
[MicroFlo](https://microflo.org) for the firmware. MicroFlo lets one visually and live reprogram the device,
to quickly and easily implement on-edge logic.

## Status
**Experimental**


## TODO

* Write some useful/fun components for XDK
* Build demo graph

## Modify XDK for C++ support

    ln -s /CHANGE/TO/PATH/TO/XDK ./my-xdk
    patch -p1 --binary my-xdk < fix-cpp-guard.diff
    patch -p1 my-xdk < enable-cpp-app-build.diff

## Building

Configure according to your installation

    export XDK=/home/jon/XDK/XDK

Build and flash

    export BCDS_BASE_DIR=${XDK_DIR}/SDK
    export BCDS_TOOL_CHAIN_PATH=${XDK_DIR}/armGCC/bin/
    make clean microflo_generate debug flash_debug_bin


## Connect from Flowhub

    make microflo_runtime

Open the LIVE url that is shown in browser
