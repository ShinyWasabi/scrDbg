<p align="center">
  <img src="screenshots/scrDbg.png" alt="Logo" width="300"/>
</p>

# scrDbg

ScrDbg is a script debugger for Grand Theft Auto V (Legacy & Enhanced).
It allows you to inspect, manipulate, and debug game scripts in real-time.

## Features

- View running scripts
- Pause scripts
- Kill scripts
- Export disassembly, strings, statics, globals, and natives
- View stack
  - View callstack
  - Edit stack frame (function args, locals, temps)
- View disassembly
  - Jump to a raw address
  - Binary search (pattern, hex, decimal, float, string)
  - NOP instructions
  - Apply custom patches
  - Generate patterns
  - View xrefs
  - Set breakpoints
- Logging
  - Log script frame times
  - Log function calls
  - Log native calls
  - Log static and global writes
- Statics editor
- Globals editor

## Screenshots

<details>
  <summary>View</summary>

<p align="center">
  <img src="screenshots/scrDbgScreenshot1.png" alt="Screenshot 1" width="400"/>
</p>

<p align="center">
  <img src="screenshots/scrDbgScreenshot2.png" alt="Screenshot 2" width="400"/>
</p>

<p align="center">
  <img src="screenshots/scrDbgScreenshot3.png" alt="Screenshot 3" width="400"/>
</p>

<p align="center">
  <img src="screenshots/scrDbgScreenshot4.png" alt="Screenshot 4" width="400"/>
</p>

<p align="center">
  <img src="screenshots/scrDbgScreenshot5.png" alt="Screenshot 5" width="400"/>
</p>

<p align="center">
  <img src="screenshots/scrDbgScreenshot6.png" alt="Screenshot 6" width="400"/>
</p>

<p align="center">
  <img src="screenshots/scrDbgScreenshot7.png" alt="Screenshot 7" width="400"/>
</p>

<p align="center">
  <img src="screenshots/scrDbgScreenshot8.png" alt="Screenshot 8" width="400"/>
</p>

<p align="center">
  <img src="screenshots/scrDbgScreenshot9.png" alt="Screenshot 9" width="400"/>
</p>

<p align="center">
  <img src="screenshots/scrDbgScreenshot10.png" alt="Screenshot 10" width="400"/>
</p>

<p align="center">
  <img src="screenshots/scrDbgScreenshot11.png" alt="Screenshot 11" width="400"/>
</p>

<p align="center">
  <img src="screenshots/scrDbgScreenshot12.png" alt="Screenshot 12" width="400"/>
</p>

<p align="center">
  <img src="screenshots/scrDbgScreenshot13.png" alt="Screenshot 13" width="400"/>
</p>

</details>

## Usage

- Disable BattlEye from Rockstar Games Launcher.
- Launch the game.
- Run `scrDbg.exe`.

## Building

To build scrDbg, you need:

- [Visual Studio 2022](https://visualstudio.microsoft.com/)
  - Desktop development with C++
  - C++ CMake tools for Windows
- [Qt 6.10.0](https://www.qt.io/download-qt-installer-oss)
  - Components for MSVC 2022 64-bit
  - `Qt6_ROOT` environment variable set to your Qt installation (e.g., `<QtInstallFolder>\6.10.0\msvc2022_64\`)

Once you have these installed, you can simply clone the repository, open it in Visual Studio, and build it.

## Credits
- [maybegreat48](https://github.com/maybegreat48) for [External](https://github.com/maybegreat48/External).
- [OldAgario](https://github.com/OldAgario) for logo design.