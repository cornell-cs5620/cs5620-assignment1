/*
tinyfiledialogs.h
optional unique header file of the tiny file dialogs library - tinyfd
created [November 9, 2014]
Last changed [December 1, 2014]
Copyright (c) 2014 Guillaume Vareille http://ysengrin.com
http://tinyfiledialogs.sourceforge.net

Version 1.1.4

tiny file dialogs - tinyfd
Cross platform file dialogs in C / C++ for WINDOWS OSX UNIX

A single C file with 3 function calls.
They pop up the open file(s) dialog,
the save file dialog or the select folder dialog.
It also compiles on c++ compilers.
Conceived as an independent complement to glut,
THERE IS NO MAIN LOOP.
On windows, native code is used to create the dialogs.
On unix, it uses zenity, present on linux, freebsd and illumos / solaris.
Otherwise it uses python 2 with tkinter, present on osx.


This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
claim that you wrote the original software. If you use this software
in a product, any form of acknowledgment would be appreciated but is not
required.
2. Altered source versions must be plainly marked as such, and must not be
misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/

#ifndef TINYFILEDIALOGS_H
#define TINYFILEDIALOGS_H

#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

char const * tinyfd_saveFileDialog (
    char const * const aTitle , /* "" */
    char const * const aDefaultPathAndFile , /* "" */
    int const aNumOfFileFilters , /* 0 */
    char const * const * const aFileFilters ) ; /* NULL or {"*.jpg","*.png"} */

char const * tinyfd_openFileDialog (
    char const * const aTitle , /* "" */
    char const * const aDefaultPathAndFile , /* "" */
    int const aNumOfFileFilters , /* 0 */
    char const * const * const aFileFilters , /* NULL or {"*.jpg","*.png"} */
    int aAllowMultipleSelects ) ; /* 0 or 1 */
/* in case of multiple files, the separator is | */

char const * tinyfd_selectFolderDialog (
	char const * const aTitle , /* "" */
    char const * const aDefaultPath ) ; /* "" */

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif /* TINYFILEDIALOGS_H */

