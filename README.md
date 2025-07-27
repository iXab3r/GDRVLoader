# GDRVLoader

GDRVLoader is a command line tool that demonstrates a BYOVD (Bring Your Own Vulnerable Driver) technique on Windows 10 and 11. The loader drops the known Gigabyte GIO driver to disk and uses its memory copy interface to patch the kernel, disabling Driver Signature Enforcement (DSE) so that unsigned drivers can be loaded or removed.

This project builds on earlier work by the authors at:
- https://github.com/zer0condition/GDRVLoader
- https://github.com/AlexisDx13/GDRVLoader

## Features
- Works on Windows 10 and Windows 11
- Drops the vulnerable Gigabyte driver from memory if it is not found on disk
- Interactive menu or fully automated command line mode
- Optional custom service name when loading a driver

## Getting Started
### Building
1. Open the Visual Studio solution using version 2022 or newer.
2. Build the **Release x64** configuration. The compiled binary will appear in the Release folder.

### Running
Run the executable as administrator.

```
GDRVLoader.exe <driver.sys> [command] [ServiceName]
```

- `command` may be `load` (default), `unload`, `dse` or `test`.
- `ServiceName` optionally overrides the service name used when installing the driver.
- If no arguments are provided the program enters an interactive menu.

## Technical Details
- The vulnerable Gigabyte driver is embedded as a byte array and written to disk when needed.
- A helper component issues the driver's memory copy IOCTL to read and write kernel memory.
- Kernel variables controlling DSE are patched so unsigned drivers can be loaded.
- Driver services are installed and removed via the Windows Service Control Manager.
- The loader supports both interactive and command-line modes.
