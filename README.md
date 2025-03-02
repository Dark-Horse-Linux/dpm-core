# DPM-Core

The core component of the Dark Horse Linux Package Manager

## What is DPM?

https://dpm.darkhorselinux.org

## Build Dependencies

To build DPM from source, you'll need the following dependencies:


- GCC/G++ (version 10 or later, supporting C++20)
- CMake (version 3.22 or later)
- Make

## Building from Source

```
mkdir ./build && cd ./build
cmake ../
make
make install
```

This will install:
- The `dpm` binary to `/usr/bin/`
- Module shared objects to `/usr/lib/dpm/modules/`
- Configuration files to `/etc/dpm/conf.d/`

## Development

For development work, you can run DPM directly from the build directory:

```
./dpm
```

To load modules from the build directory, specify the module path:

```
./dpm -m ./modules
```