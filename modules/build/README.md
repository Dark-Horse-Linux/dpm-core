# DPM Package Module

The package module provides functionality for creating, inspecting, and verifying DPM packages.

## Features

- Create DPM packages from source directories
- Extract and inspect DPM package contents and metadata
- Verify package integrity through checksums
- Support for package signing (future enhancement)

## Usage

```
dpm package <command> [options]
```

### Commands

- `create` - Create a new DPM package
- `info` - Display information about a DPM package
- `verify` - Verify the integrity of a DPM package
- `help` - Display help information

### Examples

Create a package:
```
dpm package create -s /path/to/source/dir -o my-package-1.0.x86_64.dpm -n "my-package" -v "1.0" -a "x86_64" -d "Example package" -u "Your Name <your.email@example.com>"
```

Display package information:
```
dpm package info -p my-package-1.0.x86_64.dpm
```

Verify package integrity:
```
dpm package verify -p my-package-1.0.x86_64.dpm
```

## Dependencies

- OpenSSL (for SHA-256 checksumming)
- Standard Unix utilities (tar, gzip)

## Building

```
mkdir build && cd build
cmake ../
make
```

This will build the package module and place it in the modules directory.

## Installation

The module will be installed automatically as part of the DPM build process.