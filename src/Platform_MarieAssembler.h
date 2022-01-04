/* Author: Michael Roskuski <mroskusk@student.fitchburgstate.edu>
 * Date: 2021-11-10
 * File: Contains the definitions for the interface between the Platform we're running on, and the Application.
 */

#ifndef PLATFORM_MARIEASSEMBLER_H
#define PLATFORM_MARIEASSEMBLER_H

#ifdef TRUE
#undef TRUE
#endif

#define TRUE (1)

#ifdef FALSE
#undef FALSE
#endif

#define FALSE (0)

#ifdef Assert
#undef Assert
#endif

#ifdef Min
#undef Min
#endif

#define Min(A, B) ((A) < (B) ? (A) : (B))

#ifdef Max
#undef Max
#endif

#undef Assert

#if DEBUG
# define Assert(Cnd) if ((Cnd) != TRUE) { Platform_Breakpoint(); }
#else
# define Assert(Cnd)
#endif

#define Max(A, B) ((A) > (B) ? (A) : (B))

#define Kilobyte(A) ((A) * 1024)
#define Megabyte(A) ((Kilobyte(A) * 1024)

#define global_var static
#define local_persist static

#include <stdio.h>
#include "MarieAssembler.h"

//-----
//~ Functions defined in the platform layer

void Platform_Breakpoint();

//-----
//~ Functions defined in the application layer

/* "Main" function for the application
 * While the application's actual entry point is in the platform spefic file, ApplicationMain() actually runs the application.
 * @Params InFile  Handle to the input file
 * @Params InFileSize  Size in bytes of the file which InFile repersents.
 * @Params LogisimOut  Handle where a Logisim rom file should be writen to
 * @Params RawHexOut  Handle where file containing a raw hex output should be writen to
 * @Params SymbolTableOut  Handle where a symbol table should be writen to
 * @Params ListingOut  Handle where a assembly listing should be writen to
 */
int ApplicationMain(FILE *InFile, int InFileSize, FILE *OutLogisim, FILE *OutRawHex, FILE *OutSymbolTable, FILE *OutListing);

#endif
