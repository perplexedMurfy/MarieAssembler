/* Author: Michael Roskuski <mroskusk@student.fitchburgstate.edu>
 * Date: 2021-11-10
 * Program: Assembler for the fictional Marie computer. Supports the extended instruction set.
 * File: Contains the definitions for the interface between the Platform we're running on, and the Application.
 */

#ifndef PLATFORM_MARIEASSEMBLER_H
#define PLATFORM_MARIEASSEMBLER_H

#include "MarieAssembler.h"
#include "Memory_MarieAssembler.h"

//-----
//~ Functions defined in the platform layer

/* Gets filesize
 * @Param FileName Name of file
 * @Param Success if this function was successful.
 * @Return the size of the file.
 */
size_t Platform_GetFileSize(wchar_t* FileName, int* Success);

/* Auto generates the output file name based on the input file name.
 * This is platform spefic because different platforms have different file path conventions.
 * @Param Path input file path
 * @Param Arena Memory Arena
 * @Param Indicates if this function was successful.
 * Example:
 *  Path = L"C:\Foo\Bar\Test.MarieAsm"
 *  Result = L"C:\Foo\Bar\OUT_Test"
 */
wchar_t *Platform_CreateOutputFileName(wchar_t *Path, int *Success);

//-----
//~ Functions defined in the application layer

/* "Main" function for the application
 * While the application's actual entry point is in the platform spefic file, ApplicationMain() actually runs the application.
 * @Params ArgCount The Amount of stings in Args
 * @Params Args A list of strings that contain the arguments to this application.
 */
int ApplicationMain(int ArgCount, wchar_t **Args);

#endif
