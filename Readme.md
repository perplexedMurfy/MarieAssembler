# Marie Assembler
An assembler for the fictional Marie computer

## Features
* 4 output formats:
   * Raw program image
   * Symbol table for assembled program
   * Assembly listing with High Level approximation
   * Logisim-compatible ROM/RAM image

 * Recognizes the following operations:
 
| Memonic | Parameter | Hexadecimal Representation |
|-|-|-|
| jns | Identifier or Hex literal | `0x0xxx` Low 3 bytes are parameter |
| load | Identifier or Hex literal | `0x1xxx` Low 3 bytes are parameter |
| store | Identifier or Hex literal | `0x2xxx` Low 3 bytes are parameter |
| add | Identifier or Hex literal | `0x3xxx` Low 3 bytes are parameter |
| subt | Identifier or Hex literal | `0x4xxx` Low 3 bytes are parameter |
| input | No param | `0x5000` |
| output | No param | `0x6000` |
| halt | No param | `0x7000` |
| skipcond | "greater", "lesser", "equal", or Hex literal | `0x8xxx`[[1]](#1) |
| jump | Identifier or Hex literal | `0x9xxx` Low 3 bytes are parameter |
| clear | Identifier or Hex literal | `0xAxxx` Low 3 bytes are parameter |
| addi | Identifier or Hex literal | `0xBxxx` Low 3 bytes are parameter |
| jumpi | Identifier or Hex literal | `0xCxxx` Low 3 bytes are parameter |
| loadi | Identifier or Hex literal | `0xDxxx` Low 3 bytes are parameter |
| storei | Identifier or Hex literal | `0xExxx` Low 3 bytes are parameter |
| data | Hex literal or Dec literal | Inserts 4 byte Hex/Dec literal into your program |
| .SetAddr | Hex literal | Assembler Directive: Output following program bytes starting from [Param] |
| .Ident | Identifier Name | Assembler Directive: Declare the provided name as a alias for the preceding operation's memory address. The Identifier name may be used anywhere where a Identifier can be a parameter for. |

###### [1]
 * When given "greater" as a parameter, opcode is `0x8C00`
 * When given "equal" as a parameter, opcode is `0x8400`
 * When given "lesser" as a parameter, opcode is `0x8000`
 * When given a Hex literal as a parameter, the low 3 bytes of the opcode are the parameter

For a demo of the syntax, consult `bin/testprograms/Tutorial.MarieAsm`

## Building
### Windows
#### Commandline
 * Run `build_win32.bat` from a Visual Studio Command Prompt.
   * The output will be in `bin/`

----

#### GUI
 * Run `build_win32gui_dll.bat` from a Visual Studio Command Prompt.
   * This will output `bin/DynamicMarieAssembler.dll`
 * Open `MarieAssembler_win32GUI/MarieAssembler_win32GUI.sln` With Visual studio
   * Ensure that you have ".NET desktop development" installed from the Visual Studio Installer. This project Targets .NET 4.5
 * Build the solution in Visual Studio
   * This will output `MarieAssembler_win32GUI/MarieAssembler_win32GUI/bin/x64/`
 * Copy `bin/DynamicMarieAssembler.dll` into the same folder as the executable produced in the previous step.
 
 -----

### Linux
#### Commandline
 * Run `build_linux.sh` from your commandline
   * This requires that gcc is installed
   * The output will be in `bin/`

## Command Line Usage
```
MarieAssembler.exe <InFileName> [Output Options]
Where [Output Options] can be any combination of:
  --logisim [FileName] ==> Outputs Logisim rom image at [FileName], or if blank <InFileName>.LogisimImage
  --rawhex [FileName] ==> Outputs a file containing the raw hex for the program at [FileName], or if blank <InFileName>.hex
  --symboltable [FileName] ==> Outputs a file containing a symbol table for the program at [FileName], or if blank <InFileName>.sym
  --listing [FileName] ==> Outputs a file containing a listing for the program at [FileName], or if blank <InFileName>.lst
```
