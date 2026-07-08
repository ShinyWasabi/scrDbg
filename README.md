<p align="center">
  <img src="screenshots/scrDbg.png" alt="Logo" width="300"/>
</p>

# scrDbg

scrDbg is a script debugger for RAGE games.
It allows you to inspect, manipulate, and debug game scripts in real-time.

## Supported Games

- Grand Theft Auto IV: The Complete Edition
- Red Dead Redemption
- Max Payne 3
- Grand Theft Auto V (Legacy & Enhanced)
- Red Dead Redemption 2

## Features

- View running scripts
- Pause scripts
- Kill scripts
- Export disassembly, strings, statics, globals, and natives
- Exception handling
- View functions
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
  - Log native calls with type information
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

<p align="center">
  <img src="screenshots/scrDbgScreenshot14.png" alt="Screenshot 14" width="400"/>
</p>

<p align="center">
  <img src="screenshots/scrDbgScreenshot15.png" alt="Screenshot 15" width="400"/>
</p>

<p align="center">
  <img src="screenshots/scrDbgScreenshot16.png" alt="Screenshot 16" width="400"/>
</p>

<p align="center">
  <img src="screenshots/scrDbgScreenshot17.png" alt="Screenshot 17" width="400"/>
</p>

</details>

## Usage

- Disable BattlEye from Rockstar Games Launcher for GTA V.
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
- [maybegreat48](https://github.com/maybegreat48) for [External](https://github.com/maybegreat48/External) and [GTA-V-Script-Decompiler](https://github.com/maybegreat48/GTA-V-Script-Decompiler).
- [alloc8or](https://github.com/alloc8or) for [GTA 5](https://alloc8or.re/gta5/nativedb/), [Max Payne 3](https://alloc8or.re/mp3/nativedb/), and [RDR2](https://alloc8or.re/rdr3/nativedb/) Native DB.
- [GTAIV.EFLC.FusionFix](https://github.com/ThirteenAG/GTAIV.EFLC.FusionFix) for [GTA 4 Natives](https://github.com/ThirteenAG/GTAIV.EFLC.FusionFix/blob/master/source/natives.ixx).
- [therouletteboi](https://github.com/TheRouletteBoi) for [RDR1 Native DB](https://therouletteboi.github.io/rdr/nativedb/).
- [Foxxyyy](https://github.com/Foxxyyy) for the script decompiler in [Magic-RDR](https://github.com/Foxxyyy/Magic-RDR).
- [OldAgario](https://github.com/OldAgario) for logo design.