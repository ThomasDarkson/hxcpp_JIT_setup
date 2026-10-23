[![](https://thomasdarkson.com/assets/stickers/sticker.png)](https://thomasdarkson.com)

# hxcpp_JIT_setup

A small program that installs everything [hxcpp_JIT](https://github.com/ThomasDarkson/hxcpp_JIT) needs. It is included with [hxcpp_JIT](https://github.com/ThomasDarkson/hxcpp_JIT) and will run automatically if any required component is missing.

![](https://thomasdarkson.com/assets/hxcpp_JIT/window.png)

## Usage

Any project that uses [hxcpp_JIT](https://github.com/ThomasDarkson/hxcpp_JIT) will automatically include `hxcpp_JIT_setup.exe` when it is compiled. The setup program is embedded in the executable and will extract itself to `%TEMP%\hxcpp_JIT\setup_<hash>.exe`. The components are also downloaded there.


When the program starts, the setup initially runs silently in the background to check if any component is missing. If something is missing, the setup asks for administrator permission and then opens a window that shows the installation progress, temporarily hanging the main process.

The installation starts automatically. The window shows the progress, plus links to Microsoft's license terms and the source code. When everything is installed, the window closes by itself after a moment and the main program continues. The setup will not be shown again. If the installation fails, or Windows needs a restart first, the window stays open and a **Close** button appears.

The installed components are:

- Visual Studio 2026 Build Tools
  - MSVC C++ x64/x86 build tools
  - Windows 11 SDK (10.0.28000)

Any Visual Studio 2026 installation of any edition (Build Tools, Community, Professional or Enterprise) that has both components counts as installed, so nothing is downloaded if you already have one.

All components are downloaded from Microsoft's official links.

### Arguments

[hxcpp_JIT](https://github.com/ThomasDarkson/hxcpp_JIT) will run this program with the `--quiet-unless-missing` argument first. This runs the setup without showing a window. It returns `1` if any components are missing and `0` otherwise.

If the return code is `1`, it will run the setup again with the `--app [APP_NAME]` argument, which opens the setup window. `--app` simply tells the setup what the program's name is.

There are other arguments, mostly for debugging purposes.

| Argument        | Effect                                                                                                                                     |
| --------------- | ------------------------------------------------------------------------------------------------------------------------------------------ |
| `--version`     | Prints the version (for example `hxcpp_jit_setup 0.0.1`) to the console it was started from and exits with `0`.                            |
| `--check`       | No window. Exits `0` if everything is installed, or `10` if something is missing.                                                          |

#### Exit codes

| Code | Meaning                                                                                    |
| ---- | ------------------------------------------------------------------------------------------ |
| `0`  | Everything is installed                                                                    |
| `1`  | Something is missing and no installation was attempted                                     |
| `2`  | An installation was attempted, but something is still missing (or the installation failed) |
| `3`  | Everything is installed, but Windows needs a restart                                       |
| `10` | `--check` only, something is missing                                                       |

## Building

You need to have **Visual Studio 2026** (or the Visual Studio 2026 Build Tools) with the **MSVC C++ x64/x86 build tools** and the **Windows 11 SDK (10.0.28000)** to build this.

Open **x64 Native Tools Command Prompt for VS** and run `build.bat`. The output will be `hxcpp_JIT_setup.exe`.

## License
hxcpp_JIT_setup is licensed under the [Apache License 2.0](LICENSE). See [NOTICE](NOTICE) for attribution.
 
This program does not contain any Microsoft software. It downloads the components listed above from Microsoft, and only after the user approves the administrator permission prompt. They are licensed to the user by Microsoft under Microsoft's own license terms.