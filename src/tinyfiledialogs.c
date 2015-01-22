/*
tinyfiledialogs.c
Unique code file of tiny file dialogs library - tinyfd
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


#ifdef WIN32
#include <windows.h>
#include <shlobj.h>
#else
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#if defined(__sun) && defined(__SVR4) /* SOLARIS */
#include <dirent.h>
#else
#include <sys/dir.h>
#endif /* SOLARIS */
#endif /* WIN32 */


/* #include "tinyfiledialogs.h" // not needed */


static char * tinyfd_getPathWithoutFinalSlash(
	char * const aoDestination, /* make sure it is allocated, use _MAX_PATH */
	char const * const aSource) /* aoDestination and aSource can be the same */
{
	char const * lTmp = strrchr(aSource, (int)* "/");
	if (!lTmp)
	{
		lTmp = strrchr(aSource, (int)* "\\");
	}
	if (lTmp)
	{
		strncpy(aoDestination, aSource, lTmp - aSource);
		aoDestination[lTmp - aSource] = 0;
	}
	else
	{
		aoDestination[0] = 0;
	}
	return aoDestination;
}


static char * tinyfd_GetLastName(
	char * const aoDestination, /* make sure it is allocated */
	char const * const aSource)
{
	/* copy the last name after SLASH '/' or '\' */
	char const * lTmp = strrchr(aSource, (int)* "/");
	if (!lTmp)
	{
		lTmp = strrchr(aSource, (int)* "\\");
	}
	if (lTmp)
	{
		strcpy(aoDestination, lTmp + 1);
	}
	else
	{
		strcpy(aoDestination, aSource);
	}
	return aoDestination;
}


#ifdef WIN32
char const * tinyfd_saveFileDialog (
    char const * const aTitle , /* "" */
    char const * const aDefaultPathAndFile , /* "" */
    int const aNumOfFileFilters , /* 0 */
    char const * const * const aFileFilters ) /* NULL or {"*.jpg","*.png"} */
{
    static char strbuf [ 1024 ] ;
    int i ;
    char * p;
	OPENFILENAME ofn ;
    char lDirname [ _MAX_PATH ] ;
    char lFileFilters[1024] = "";
    char lTempString[1024] ;

    if ( ! aDefaultPathAndFile )
    {
		strcpy ( strbuf , "" ) ;
    }
	else
	{
		strcpy ( strbuf , aDefaultPathAndFile ) ;
	}


    if (aNumOfFileFilters > 0)
    {
        strcat(lFileFilters, aFileFilters[0]);
        for (i = 1; i < aNumOfFileFilters; i++)
        {
            strcat(lFileFilters, ";");
            strcat(lFileFilters, aFileFilters[i]);
        }
        strcat(lFileFilters, "\n");
        strcpy(lTempString, lFileFilters);
        strcat(lFileFilters, lTempString);
        strcat(lFileFilters, "All Files\n*.*\n");
        p = lFileFilters;
        while ((p = strpbrk(p, "\n")) != NULL)
        {
            *p = '\0';
            p ++ ;
        }
    }

	tinyfd_getPathWithoutFinalSlash ( lDirname , strbuf ) ;

	ofn.lStructSize     = sizeof(OPENFILENAME) ;
	ofn.hwndOwner       = 0 ;
	ofn.hInstance       = 0 ;
	ofn.lpstrFilter     = lFileFilters ;
	ofn.lpstrCustomFilter = NULL ;
	ofn.nMaxCustFilter  = 0 ;
	ofn.nFilterIndex    = 0 ;
	ofn.lpstrFile       = strbuf ;
	ofn.nMaxFile        = _MAX_PATH ;
	ofn.lpstrFileTitle  = NULL ;
	ofn.nMaxFileTitle   = _MAX_FNAME + _MAX_EXT ;
	ofn.lpstrInitialDir = lDirname ;
	ofn.lpstrTitle      = aTitle ;
	ofn.Flags           = OFN_OVERWRITEPROMPT ;
	ofn.nFileOffset     = 0 ;
	ofn.nFileExtension  = 0 ;
	ofn.lpstrDefExt     = "" ; //"*";
	ofn.lCustData       = 0L ;
	ofn.lpfnHook        = NULL ;
	ofn.lpTemplateName  = NULL ;

	if ( GetSaveFileName ( & ofn ) == 0 )
    {
		return NULL ;
    }
	else
    {
		return strbuf ;
	}
}


char const * tinyfd_openFileDialog (
    char const * const aTitle , /* "" */
    char const * const aDefaultPathAndFile , /* "" */
    int const aNumOfFileFilters , /* 0 */
    char const * const * const aFileFilters , /* NULL or {"*.jpg","*.png"} */
    int aAllowMultipleSelects ) /* 0 or 1 */
/* in case of multiple files, the separator is | */
{
	static char lBuff[8192];
	int i , j ;
	char * p;
	OPENFILENAME ofn;
	char lDirname [ _MAX_PATH ] ;
	char lFileFilters[1024] = "";
	char lTempString[1024] ;
    char * lPointers[64];
	size_t lLengths[64];
    size_t lBuffLen ;

	if ( ! aDefaultPathAndFile )
    {
		strcpy ( lBuff , "" ) ;
    }
	else
	{
		strcpy ( lBuff , aDefaultPathAndFile);
	}

	if (aNumOfFileFilters > 0)
	{
		strcat(lFileFilters, aFileFilters[0]);
		for (i = 1; i < aNumOfFileFilters; i++)
		{
			strcat(lFileFilters, ";");
			strcat(lFileFilters, aFileFilters[i]);
		}
		strcat(lFileFilters, "\n");
		strcpy(lTempString, lFileFilters);
		strcat(lFileFilters, lTempString);
		strcat(lFileFilters, "All Files\n*.*\n");
		p = lFileFilters;
		while ((p = strpbrk(p, "\n")) != NULL)
		{
			*p = '\0';
			p ++ ;
		}
	}

	tinyfd_getPathWithoutFinalSlash ( lDirname , lBuff ) ;

	ofn.lStructSize     = sizeof ( OPENFILENAME ) ;
	ofn.hwndOwner       = 0 ;
	ofn.hInstance       = 0 ;
	ofn.lpstrFilter		= lFileFilters;
	ofn.lpstrCustomFilter = NULL ;
	ofn.nMaxCustFilter  = 0 ;
	ofn.nFilterIndex    = 0 ;
	ofn.lpstrFile		= lBuff ;
	ofn.nMaxFile        = _MAX_PATH ;
	ofn.lpstrFileTitle  = NULL ;
	ofn.nMaxFileTitle   = _MAX_FNAME + _MAX_EXT ;
	ofn.lpstrInitialDir = lDirname ;
	ofn.lpstrTitle      = aTitle ;
	ofn.Flags			= OFN_EXPLORER ;
	ofn.nFileOffset     = 0 ;
	ofn.nFileExtension  = 0 ;
	ofn.lpstrDefExt     = "*" ;
	ofn.lCustData       = 0L ;
	ofn.lpfnHook        = NULL ;
	ofn.lpTemplateName  = NULL ;

	if ( aAllowMultipleSelects )
	{
		ofn.Flags |= OFN_ALLOWMULTISELECT;
	}

	if ( GetOpenFileName ( & ofn ) == 0 )
    {
		return NULL ;
    }
	else
    {
        lBuffLen = strlen(lBuff) ;
        lPointers[0] = lBuff + lBuffLen + 1 ;
		if ( !aAllowMultipleSelects || (lPointers[0][0] == '\0')  )
            return lBuff ;

        i = 0 ;
        do
        {
			lLengths[i] = strlen(lPointers[i]);
            lPointers[i+1] = lPointers[i] + lLengths[i] + 1 ;
            i ++ ;
        }
        while ( lPointers[i][0] != '\0' );
		i--;
        p = lBuff + sizeof(lBuff) - 1 ;
        * p = '\0';
        for ( j = i ; j >=0 ; j-- )
        {
            p -= lLengths[j];
			memcpy(p, lPointers[j], lLengths[j]);
			p--;
            *p = '\\';
            p -= lBuffLen ;
			memcpy(p, lBuff, lBuffLen);
            p--;
            *p = '|';
        }
		p++;
		return p ;
	}
}


char const * tinyfd_selectFolderDialog (
	char const * const aTitle , /* "" */
	char const * const aDefaultPath ) /* "" */
{
	static char szDir [ MAX_PATH ] ;
	BROWSEINFO bInfo ;
	LPITEMIDLIST lpItem ;

	bInfo.hwndOwner = 0 ;
	bInfo.pidlRoot = NULL ;
	bInfo.pszDisplayName = szDir ;
	bInfo.lpszTitle = aTitle ;
	bInfo.ulFlags = 0 ;
	bInfo.lpfn = NULL ;
	bInfo.lParam = 0 ;
	bInfo.iImage = -1 ;

	lpItem = SHBrowseForFolder ( & bInfo ) ;
	if ( lpItem )
	{
		SHGetPathFromIDList ( lpItem , szDir ) ;
	}

	return szDir ;
}
#else /* unix */
char const * tinyfd_saveFileDialog (
    char const * const aTitle , /* "" */
    char const * const aDefaultPathAndFile , /* "" */
    int const aNumOfFileFilters , /* 0 */
    char const * const * const aFileFilters ) /* NULL or {"*.jpg","*.png"} */
{
    static char lBuff [ 1024 ] ;
    int i ;
    char lDialogString [ 1024 ] ;
    char lFileFilters [ 1024 ] ;
    char lFilenameonly [ 256 ] ;
    char lDirname [ PATH_MAX ] ;
    char lInteractiv [ 8 ] = "" ;
    DIR * lDir ;
    FILE * lIn ;
    FILE * lZenity = popen ( "which zenity" , "r" ) ;
    if ( ( fgets ( lBuff , sizeof ( lBuff ) , lZenity ) != NULL )
        || ( ! strncmp ( "zenity:" , lBuff , 7 ) ) )
    { /* zenity is present: linux and some others */

        if ( aNumOfFileFilters > 0 )
        {
            strcpy ( lFileFilters , "--file-filter='" ) ;
            for ( i = 0 ; i < aNumOfFileFilters ; i ++ )
            {
                strcat ( lFileFilters , aFileFilters [ i ] ) ;
                strcat ( lFileFilters , " " ) ;
            }
            strcat ( lFileFilters , "' --file-filter='All files | *'" ) ;
        }

        sprintf ( lDialogString ,
"zenity --file-selection --save --confirm-overwrite \
--title=\"%s\" --filename=\"%s\" %s",
                  aTitle , aDefaultPathAndFile , lFileFilters) ;
    }
    else
    { /* python: osx and others */
        if ( ( aNumOfFileFilters > 1 )
            || ( ( aNumOfFileFilters == 1 )  // test because poor osx behaviour
                && ( aFileFilters[0][strlen(aFileFilters[0])-1] != '*' ) ) )
        {
            strcpy ( lFileFilters , ",filetypes=(" ) ;
            for ( i = 0 ; i < aNumOfFileFilters ; i ++ )
            {
                strcat ( lFileFilters , "('','" ) ;
                strcat ( lFileFilters , aFileFilters [ i ] ) ;
                strcat ( lFileFilters , "')," ) ;
            }
            strcat ( lFileFilters , "('All files','*'))" ) ;
        }

        if ( ! isatty ( 1 ) )
        {
            strcpy ( lInteractiv , "-i" ) ; /* for osx without console */
        }

        tinyfd_getPathWithoutFinalSlash ( lDirname , aDefaultPathAndFile ) ;
        tinyfd_GetLastName ( lFilenameonly , aDefaultPathAndFile ) ;
        sprintf ( lDialogString ,
"python %s -c \"import Tkinter,tkFileDialog;root=Tkinter.Tk();root.withdraw();\
print tkFileDialog.asksaveasfilename(\
title='%s',initialdir='%s',initialfile='%s' %s)\"" ,
            lInteractiv , aTitle , lDirname , lFilenameonly , lFileFilters ) ;
    }
    /* printf ( "lDialogString: %s\n" , lDialogString ) ; //*/
    pclose ( lZenity ) ;
    if ( ! ( lIn = popen ( lDialogString , "r" ) ) )
    {
        return NULL ;
    }
    while ( fgets ( lBuff , sizeof ( lBuff ) , lIn ) != NULL )
    {
    }
    pclose ( lIn ) ;
    lBuff[ strlen ( lBuff ) -1 ] = 0 ;

	/* printf ( "lBuff: %s\n" , lBuff ) ; //*/

    tinyfd_getPathWithoutFinalSlash ( lDirname , lBuff ) ;
    if ( strlen ( lDirname ) > 0 )
    {
        lDir = opendir ( lDirname ) ;
        if ( ! lDir )
        {
            return NULL ;
        }
        closedir ( lDir ) ;
    }
    return lBuff ;
}


char const * tinyfd_openFileDialog (
    char const * const aTitle , /* "" */
    char const * const aDefaultPathAndFile , /* "" */
    int const aNumOfFileFilters , /* 0 */
    char const * const * const aFileFilters , /* NULL or {"*.jpg","*.png"} */
    int aAllowMultipleSelects ) /* 0 or 1 */
/* in case of multiple files, the separator is | */
{
    static char lBuff [ 8192 ] ;
    int i ;
    char lDialogString [ 1024 ] ;
    char lFileFilters [ 1024 ] = "" ;
    char lMultipleSelects [ 16 ] = "" ;
    char lInteractiv [ 8 ] = "" ;
   FILE * lIn ;
    FILE * lZenity = popen ( "which zenity" , "r" ) ;
    if ( ( fgets ( lBuff , sizeof ( lBuff ) , lZenity ) != NULL )
        || ( ! strncmp ( "zenity:" , lBuff , 7 ) ) )
    { /* zenity is present: linux and some others */

        if ( aNumOfFileFilters > 0 )
        {
	        strcpy ( lFileFilters , "--file-filter='" ) ;
            for ( i = 0 ; i < aNumOfFileFilters ; i ++ )
            {
                strcat ( lFileFilters , aFileFilters [ i ] ) ;
                strcat ( lFileFilters , " " ) ;
            }
 	        strcat ( lFileFilters , "' --file-filter='All files | *'" ) ;
        }

        if ( aAllowMultipleSelects )
        {
	        strcpy ( lMultipleSelects , "--multiple" ) ;
        }

        sprintf ( lDialogString ,
"zenity --file-selection --title=\"%s\" --filename=\"%s\" %s %s",
			aTitle , aDefaultPathAndFile , lFileFilters , lMultipleSelects ) ;
    }
    else
    { /* python: osx and others */

        if ( ( aNumOfFileFilters > 1 )
          || ( ( aNumOfFileFilters == 1 )  // test because poor osx behaviour
        	&& ( aFileFilters[0][strlen(aFileFilters[0])-1] != '*' ) ) )
        {
	        strcpy ( lFileFilters , ",filetypes=(" ) ;
            for ( i = 0 ; i < aNumOfFileFilters ; i ++ )
            {
                strcat ( lFileFilters , "('','" ) ;
                strcat ( lFileFilters , aFileFilters [ i ] ) ;
                strcat ( lFileFilters , "')," ) ;
            }
 	        strcat ( lFileFilters , "('All files','*'))" ) ;
        }

        if ( aAllowMultipleSelects )
        {
	        strcpy ( lMultipleSelects , ",multiple=1" ) ;
        }

        if ( ! isatty ( 1 ) )
        {
            strcpy ( lInteractiv , "-i" ) ; /* for osx without console */
        }

        sprintf ( lDialogString ,
"python %s -c \"\
import Tkinter,tkFileDialog;root=Tkinter.Tk();root.withdraw();\
lFiles=tkFileDialog.askopenfilename(title='%s',\
initialdir='%s' %s %s);\
\nif not isinstance(lFiles, tuple):\n\tprint lFiles\nelse:\
\n\tlFilesString=''\n\tfor lFile in lFiles:\n\t\tlFilesString+=str(lFile)+'|'\
\n\tprint lFilesString[:-1]\n\"" ,
			lInteractiv , aTitle , aDefaultPathAndFile ,
            lFileFilters , lMultipleSelects ) ;
    }
    /* printf ( "lDialogString: %s\n" , lDialogString ) ; //*/
    pclose ( lZenity ) ;
    lBuff[0]='\0';
    if ( ! ( lIn = popen ( lDialogString , "r" ) ) )
    {
        return NULL ;
    }
    while ( fgets ( lBuff , sizeof ( lBuff ) , lIn ) != NULL )
    {
    }
    pclose ( lIn ) ;
    lBuff[ strlen ( lBuff ) -1 ] = 0 ;

    /*printf ( "lBuff: %s\n" , lBuff ) ; //*/

    lIn = fopen( lBuff , "r" ) ;
    if ( ! lIn )
    {
        return NULL ;
    }
    fclose ( lIn ) ;
    return lBuff ;
}


char const * tinyfd_selectFolderDialog (
	char const * const aTitle , /* "" */
	char const * const aDefaultPath ) /* "" */
{
	static char lBuff [ PATH_MAX ] ;
    char lDialogString [ 1024 ] ;
    char lInteractiv [ 8 ] = "" ;
    DIR * lDir ;
	FILE * lIn ;
	FILE * lZenity = popen ( "which zenity" , "r" ) ;
	if ( ( fgets ( lBuff , sizeof ( lBuff ) , lZenity ) != NULL )
      || ( ! strncmp ( "zenity:" , lBuff , 7 ) ) )
	{ /* zenity is present: linux and some others */
        sprintf ( lDialogString ,
"zenity --file-selection --directory --title=\"%s\" --filename=\"%s\"" ,
                 aTitle , aDefaultPath ) ;
	}
	else
	{ /* python: osx and others */

        if ( ! isatty ( 1 ) )
        {
            strcpy ( lInteractiv , "-i" ) ;  /* for osx without console */
        }

        sprintf ( lDialogString ,
"python %s -c \"import Tkinter,tkFileDialog;root=Tkinter.Tk();root.withdraw();\
print tkFileDialog.askdirectory(title='%s',initialdir='%s')\"" ,
            lInteractiv , aTitle , aDefaultPath ) ;
	}
    /*printf ( "lDialogString: %s\n" , lDialogString ) ; //*/
	pclose ( lZenity ) ;
    if ( ! ( lIn = popen ( lDialogString , "r" ) ) )
    {
        return NULL ;
    }
	while ( fgets ( lBuff , sizeof ( lBuff ) , lIn ) != NULL )
	{
	}
	pclose ( lIn ) ;
	lBuff [ strlen ( lBuff ) - 1 ] = 0 ;

	/*printf ( "lBuff: %s\n" , lBuff ) ; //*/

	if ( strlen ( lBuff ) > 0 )
	{
		lDir = opendir ( lBuff ) ;
		if ( ! lDir )
		{
			return NULL ;
		}
		closedir ( lDir ) ;
	}
	return lBuff ;
}
#endif /* WIN32 */
