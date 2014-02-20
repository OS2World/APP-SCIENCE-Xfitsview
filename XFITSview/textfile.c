/* TextFile routines for XFITSview */
/*-----------------------------------------------------------------------
*  Copyright (C) 1996
*  Associated Universities, Inc. Washington DC, USA.
*  This program is free software; you can redistribute it and/or
*  modify it under the terms of the GNU General Public License as
*  published by the Free Software Foundation; either version 2 of
*  the License, or (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*-----------------------------------------------------------------------*/
#include <Xm/Xm.h> 
#include <Xm/FileSB.h> 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "fitsio.h"
#include "zsubs.h"
#include "gzipread.h"
#include "textfile.h" 
#include "messagebox.h" 
#include "helpbox.h" 
#include "xfitsview.h" /* a bit of uglyness for saving directories */

/* define file types */
#define TEXTTYPE    1   /* Unix text file (LF only) */
#define FITSTYPE    2   /* FITS file header */
#define DOSTEXTTYPE 3   /* DOS text file (CR, LF) */
  
/* internal functions */
/* Flush output buffer - TFilePtr already locked*/
int TextFileFlush(TextFilePtr TFilePtr); /* returns 1 if OK */
/* read text file buffer */
int TextFileBufferIn(TextFilePtr TFilePtr);/* returns 1 if OK */
/* If file specified then fills in file info in TextFileInfo structure */
void TextFileOKCB (Widget filebox, XtPointer clientData, XtPointer callData);
/* TextFile selection canceled */
void TextFileCancelCB (Widget filebox, XtPointer clientData, XtPointer callData);

  
/* create/initialize TextFileInfo structure                               */
/* w is a Widget for dialog boxes to be attached to                       */
/* if non null filename and directory filled in                           */
TextFilePtr TextFileMake (Widget w, char* filename, char* directory)
{
  TextFilePtr TFilePtr;

/* create structure */
  TFilePtr = (TextFilePtr)malloc(sizeof(TextFileInfo));
  if (!TFilePtr) return TFilePtr; /* alloc failed? */

/* fill in value passed */
  TFilePtr->w = w;
  if (filename) { /* make copy */
    TFilePtr->FileName = (char*)malloc(strlen(filename)+1);
    strcpy (TFilePtr->FileName, filename);}
  else
    TFilePtr->FileName = NULL;
  if (directory) { /* make copy */
    TFilePtr->directory = (char*)malloc(strlen(directory)+1);
    strcpy (TFilePtr->directory, directory);}
  else
    TFilePtr->directory = NULL;

/* others */
    TFilePtr->file = NULL;
    TFilePtr->OKfunc = NULL;
    TFilePtr->CancelFunc = NULL;
    TFilePtr->hFile = 0;
    TFilePtr->Good = 0;
    TFilePtr->HitEOF = 0;
    TFilePtr->InBuffer = 0;
    TFilePtr->BufferPos = 0;
    TFilePtr->State = 0;
    TFilePtr->FileType = 0;
    TFilePtr->isGzip = 0;
    TFilePtr->chkGzip = 1;

  return TFilePtr;
} /* end TextFileMake */

/* delete TextFileInfo structure             */
void TextFileKill (TextFilePtr TFilePtr)
{
  if (!TFilePtr) return; /* anybody home */

/* delete any characters string and buffer */
  if (TFilePtr->FileName) free(TFilePtr->FileName);
  if (TFilePtr->directory) free(TFilePtr->directory);

/* need to close output file? */
  if (TFilePtr->file) fclose(TFilePtr->file);
  if (TFilePtr->hFile) FileClose(TFilePtr->hFile);

/* delete any gzip structures */
  if (TFilePtr->isGzip) gz_close();

/* delete structure */
  free(TFilePtr);
} /* end TextFileKill */

/* get file specification                                                 */
/* inout = 1 => input; 2=> output                                         */
/* TFilePtr->Good indicated if file selected                              */
void TextFileFind (int inout, TextFilePtr TFilePtr, TextFileProc OKfunc,
		   TextFileProc CancelFunc)
{
  Widget       filebox;
  XmString     wierdstring = NULL;
  Arg          wargs[5]; 

/* save function pointesr */
  TFilePtr->OKfunc = OKfunc;
  TFilePtr->CancelFunc = CancelFunc;
  TFilePtr->State = inout; /* input or output? */

/* bring up selection box */
  filebox = (Widget) XmCreateFileSelectionDialog (TFilePtr->w, 
						  "text_file", NULL, 0);
  XtAddCallback (filebox, XmNokCallback, TextFileOKCB, (XtPointer)TFilePtr);
  XtAddCallback (filebox, XmNcancelCallback, TextFileCancelCB, 
		 (XtPointer)TFilePtr);
  XtAddCallback (filebox, XmNhelpCallback, HelpBoxTopicCB, 
		   (XtPointer)"Browser");

/* set directory if it is defined */
  if (TFilePtr->directory) {
    wierdstring = XmStringCreateSimple (TFilePtr->directory);
    XtSetArg (wargs[0], XmNdirectory, wierdstring);
    XtSetValues (filebox, wargs, 1);
    if (wierdstring) XmStringFree(wierdstring); wierdstring = NULL;
  }

/* Shazam appear: */
  XtManageChild (filebox);
  XtPopup (XtParent (filebox), XtGrabNone);
/* all the action is in the callback routine */
} /* end TextFileFind */
  
/* open text file                                                         */ 
/* inout = 1 => input; 2=> output                                         */
/* returns 1 if successful                                                */
int TextFileOpen (TextFilePtr TFilePtr, int inout)
{ 
/* use standard c I/O on write and fitsio on read which allows using the
   gzip decompression routines */
  char fullname[120];
  int  length;
  FITSfile *Fname;

  TFilePtr->State = inout;
  
/* put together full file name */
  strcpy(fullname, TFilePtr->directory);
  length = strlen(TFilePtr->directory);
  if (length>0) fullname[length] = '/';
  else length = -1;
  strcpy(&fullname[length+1], TFilePtr->FileName);

  /* open for read */
  if (inout==1) { 
   Fname = (FITSfile*)MakeString(fullname);
   TFilePtr->hFile = FileOpen(Fname);
   KillString(Fname); /* as ye create shall thou destroy */
   if (!TFilePtr->hFile) { /* open failed */
     MessageShow ("Error openinging text file");
     return 0; } /* end of open failed */
   /* read first block, decide type */
   if (!TextFileBufferIn(TFilePtr)) { /* I/O failed */
     MessageShow ("Error reading text file");
     TextFileClose(TFilePtr);
     return 0; } /* end of I/O failed */
  } /* end open for read */
  /* open for write */
  else if (inout==2) { 
    TFilePtr->file = fopen ((const char*)fullname,"at");
    if (!TFilePtr->file) { /* failed? */
     MessageShow ("Error openinging text file");
     return 0; } /* end of open failed */
  } /* end open for write */
  return 1; /* OK */
} /* end TextFileOpen */
  
/* close file specified by handle TFilePtr                                */ 
/* returns 1 if successful                                                */ 
int TextFileClose(TextFilePtr TFilePtr)
{
  int good = 1;
  if (!TFilePtr) return 0;   /* validity check */

/* flush buffer on write */
  if ((TFilePtr->State==2) && (TFilePtr->BufferPos>0))
    if (!TextFileFlush(TFilePtr))
      {MessageShow("Error flushing text file buffer"); good = 0;}
/* Close */
  if (TFilePtr->State==1) {/* close read */
    if (FileClose(TFilePtr->hFile)!=1) { /* close failed */
      MessageShow("Error closing text file"); good = 0;}
    TFilePtr->hFile = 0;
    /* delete any gzip structures */
    if (!TFilePtr->isGzip) gz_close();
    TFilePtr->isGzip = 0;
  } /* end close read */
  else if (TFilePtr->State==2) {/* close write */
    if (fclose(TFilePtr->file))
      {MessageShow("Error closing text file"); good = 0;}
    TFilePtr->file = NULL;
  } /* end close write */

 
 TFilePtr->State = 0; /* mark as closed */

  return good;
} /* end TextFileClose */
  
/* Read a line of text from a file                                        */ 
/* returns 1 if OK, 0 if failed, -1 -> HitEOF                             */ 
/* maxchar is the maximum number of characters allowed in line */
int TextFileRead(TextFilePtr TFilePtr, char* line, int maxchar)
{
   char *next, CR=13, LF=10, *linein;
   long loop, count=0;

   if (!TFilePtr) return 0;   /* validity check */
   if (!TFilePtr->State) return 0; /* active? */
   linein = line;

/* branch by file type */
   switch (TFilePtr->FileType) {
      case TEXTTYPE: /* Unix text file */
/* loop through buffer until hitting a LF */
           next = (char *)(TFilePtr->Buffer + TFilePtr->BufferPos);
           for (loop=0; loop<1000; loop++) {
             if (TFilePtr->BufferPos>=TFilePtr->InBuffer) 
	       { /* read next buffer full */
                if (!TextFileBufferIn(TFilePtr)) { /* I/O failed */
                    MessageShow ("Error reading text file");
                     TextFileClose(TFilePtr);
                     return 0;
                     } /* end of I/O failed */
                  next = (char *)(TFilePtr->Buffer);
                  } /*end of read buffer */
               if (TFilePtr->InBuffer<1) break; /* end of file? */
               if (*next!=LF) 
                  {count++; if (count<=maxchar) *line++ = *next; 
		   next++; 
		   TFilePtr->BufferPos++;}
               else {TFilePtr->BufferPos++; break;}
               } /* end of loop reading card */
            *line = 0; /* terminating null */
      /* done with file ?*/
           if ((TFilePtr->InBuffer==0) && TFilePtr->HitEOF) {return -1;}
           break;
      case FITSTYPE: /* FITS file */
           /* done? */
           if ((TFilePtr->InBuffer==0) && TFilePtr->HitEOF) {return -1;}
           /* Next 80 characters */
           next = (char *)(TFilePtr->Buffer + TFilePtr->BufferPos);
           for (loop=0; loop<80; loop++) {
             if (TFilePtr->BufferPos>=TFilePtr->InBuffer) { /* read next buffer full */
                if (!TextFileBufferIn(TFilePtr)) { /* I/O failed */
                    MessageShow ("Error reading text file");
                     TextFileClose(TFilePtr);
                     return 0;
                     } /* end of I/O failed */
                  next = (char *)(TFilePtr->Buffer);
                  } /*end of read buffer */
               if (TFilePtr->InBuffer<1) break; /* end of file? */
               count++; 
               if (count<=maxchar) *line++ = *next; 
               next++; 
               TFilePtr->BufferPos++;
               } /* end of loop reading card */
            *line = 0; /* terminating null */
      /* done with file ? - check for "END     " card*/
           if (!strncmp (linein, "END     ", 8)) 
	     {TFilePtr->InBuffer=0; TFilePtr->HitEOF = 1; return -1;}
           break;
      case DOSTEXTTYPE: /* line feeds in text file */
/* loop through buffer until hitting a CR, drop LF */
           next = (char *)(TFilePtr->Buffer + TFilePtr->BufferPos);
           for (loop=0; loop<1000; loop++) {
             if (TFilePtr->BufferPos>=TFilePtr->InBuffer) { /* read next buffer full */
                if (!TextFileBufferIn(TFilePtr)) { /* I/O failed */
                    MessageShow ("Error reading text file");
                     TextFileClose(TFilePtr);
                     return 0;
                     } /* end of I/O failed */
                  next = (char *)(TFilePtr->Buffer);
                  } /*end of read buffer */
               if (TFilePtr->InBuffer<1) break; /* end of file? */
               /*debug if (*next==LF) next++;  ignore line feeds */
               if (*next!=LF) 
                  {count++; if (count<=maxchar) *line++ = *next; 
		   next++; 
		   TFilePtr->BufferPos++;}
               else {TFilePtr->BufferPos++; break;}
               } /* end of loop reading card */
            *line = 0; /* terminating null */
      /* done with file ?*/
           if ((TFilePtr->InBuffer==0) && TFilePtr->HitEOF) {return -1;}
           break;
      default: 
           if ((TFilePtr->InBuffer==0) && TFilePtr->HitEOF) {return -1;}
           sprintf (line, "Unknown file type ");
           TFilePtr->InBuffer=0; 
           TFilePtr->HitEOF = 1;
           break;
      }  /* end of branch by file type */
   return 1;
} /* end TextFileRead */
  
/* Write a line of text to a file                                          */ 
/* returns 1 if OK, 0 if failed                                           */ 
int TextFileWrite(TextFilePtr TFilePtr, char* line)
{
   long loop, count;
   char *next, LF=10;

   if (!TFilePtr) return 0;   /* validity check */
   if (TFilePtr->State!=2) {return 0;} /* Open write? */
   count = strlen (line);
   /* copy string to buffer flushing buffer when full */
    next = (char *)(TFilePtr->Buffer + TFilePtr->BufferPos);
   for (loop = 0; loop<count; loop++) {
      if ((TFilePtr->BufferPos)+1>=TEXTBUFFER_SIZE) { /* flush buffer? */
         if (!TextFileFlush(TFilePtr)) { /* I/O failed */
            MessageShow ("Error writing text file");
            TextFileClose(TFilePtr);
            return 0;
            } /* end of I/O failed */
         next = (char *)(TFilePtr->Buffer);
         } /* end flush buffer */

      *next++ = *line++; TFilePtr->BufferPos++;
      } /* end loop copying string */
   if ((TFilePtr->BufferPos)+1>=TEXTBUFFER_SIZE) { /* flush buffer? */
      if (!TextFileFlush(TFilePtr)) { /* I/O failed */
         MessageShow ("Error writing text file");
         TextFileClose(TFilePtr);
         return 0;
         } /* end of I/O failed */
      } /* end of flush buffer */
   *next = LF; TFilePtr->BufferPos++;/* add LF */
   return 1;
} /* end TextFileWrite */

/* internal functions */
/* Flush output buffer - TFilePtr already locked*/
/* returns 1 if OK, 0 if failed */
int TextFileFlush(TextFilePtr TFilePtr)
{
   long count;
   size_t size, nout, number=1;
   if (!TFilePtr) return 0;   /* validity check */
   size = TFilePtr->BufferPos;
   if (size<=0) return 1; /* nothing to do */
   nout = fwrite ((const void*)TFilePtr->Buffer, size, number, 
		  TFilePtr->file);
   if (nout!=1) return 0; /* write failed? */
   TFilePtr->InBuffer = 0;
   TFilePtr->BufferPos = 0;
   return 1;
} /* end TextFileFlush */

/* Read input buffer */
/* returns 1 if OK, 0 if failed */
int TextFileBufferIn(TextFilePtr TFilePtr)
{
   char CR=13, LF=10;
   int hasSimple=0, hasCR=0, hasLF=0, hasBinary=0;
   int count, i, nread, readret, error, nred;
   unsigned char gzip_magic[2] = {"\037\213"}; /* gzip magic number */
   cMemPtr test;

   if (!TFilePtr) return 0;   /* validity check */
   if (TFilePtr->HitEOF) { /* previous EOF? */
      TFilePtr->InBuffer = 0;
      TFilePtr->BufferPos = 0;
      return 1;}

   count = TEXTBUFFER_SIZE;
/* regular or gzipped file? */
  if (!TFilePtr->isGzip)
    {nread = FileRead (TFilePtr->hFile, (MemPtr)TFilePtr->Buffer, count); 
     error = nread==0; }
  else
    {nred = count;
     readret = gz_read (&nred, (cMemPtr)TFilePtr->Buffer); /* read block */
     nread = nred;
     error = (readret != PR_SUCCESS) || (nread==0); }
   
/* look for magic number for gzip files on first read */
  if ((!error) && TFilePtr->chkGzip) {
    test = (cMemPtr)TFilePtr->Buffer;
    TFilePtr->chkGzip = 0; /* only check first block */
    if ((test[0]==gzip_magic[0]) && (test[1]==gzip_magic[1]))
      { /* TFilePtr file is gzipped - set up */
	TFilePtr->isGzip = 1; /* it's gzip compressed */
	readret = FileSeek (TFilePtr->hFile, 0L);  /* reposition */
	error = (readret != 1);
	if (!error) readret = gz_init (TFilePtr->hFile);  /* init */
	error = error || (readret != PR_SUCCESS);
	if (!error) readret = gz_seek (0L); /* position */
	error = error || (readret != PR_SUCCESS);
	nred = count;
	if (!error) readret = gz_read (&nred, (cMemPtr)TFilePtr->Buffer);
	nread = nred;
	error = error || (readret != PR_SUCCESS) || (nred==0);
      }
  } /* end of gzip check */

/*Error check */ 
   if (nread<count) { /* hit EOF in read - some good data */
      TFilePtr->HitEOF = 1; error = 0;}
   if (error) return 0; /* read failed? */

   TFilePtr->InBuffer = nread;
   TFilePtr->BufferPos = 0;

   /* determine file type if not known */
   if (!TFilePtr->FileType) { /* determine file type from first buffer */
     /* look for FITS intro */
      hasSimple = (!strncmp ((char*)TFilePtr->Buffer, "SIMPLE  =      ", 15));
      /* look for carrage return */
      for (i=0;i<nread;i++) 
	{if (TFilePtr->Buffer[i]==CR)
	   {hasCR = 1;  break;}}
      /* look for line feed */
      for (i=0;i<nread;i++) 
	{if (TFilePtr->Buffer[i]==LF)
	   {hasLF = 1;  break;}}
      /* look for binary values */
      for (i=0;i<nread;i++) 
	{if (TFilePtr->Buffer[i]>128)
	   {hasBinary = 1;  break;}}
      /* decide type */
      /* DOS has CR and LF */
      if (hasCR && hasLF) TFilePtr->FileType = DOSTEXTTYPE;
      /* Unix text has LF but no CR */
      if ((!TFilePtr->FileType) && hasLF) TFilePtr->FileType = TEXTTYPE;
      /* FITS has SIMPLE = ... but no CR or LF */
      if ((!TFilePtr->FileType) && hasSimple) TFilePtr->FileType = FITSTYPE;
      
      /* check for bad file types */
      if (hasBinary && (!hasSimple)) /* can't cope with binary files */
	{MessageShow("Selected file appears to be neither text nor FITS");
	 return 0;}
      if (!TFilePtr->FileType) /* Don't recognize text file type */
	{MessageShow("Unrecognized text file type");
	 return 0;}
      } /* end of determining file type */
   return 1;
} /* end TextFileBufferIn */
  
void TextFileOKCB (Widget filebox, XtPointer clientData, XtPointer callData)
/* Gets file information and calls OKfunc if non NULL*/
{
  XmFileSelectionBoxCallbackStruct *cbs;
  TextFilePtr TFilePtr = (TextFilePtr)clientData;
  XPointer XPtr;
  char *filename, *directory;
  int i, length, first;

  cbs = (XmFileSelectionBoxCallbackStruct *) callData;

/* delete any old file names */
  if (TFilePtr->FileName) free(TFilePtr->FileName);
  if (TFilePtr->directory) free(TFilePtr->directory);

/* get file name */
 if (!XmStringGetLtoR (cbs->value, XmSTRING_DEFAULT_CHARSET, &filename))
   return; /* error */
/* remove directory part of name */
  length = strlen(filename);
  first = 0;
  for (i=0; i<length; i++) if (filename[i]=='/') first = i+1;
  TFilePtr->FileName = (char*)malloc(length-first+2);
  strcpy (TFilePtr->FileName,&filename[first]);

/* get directory name */
  if (!XmStringGetLtoR (cbs->dir, XmSTRING_DEFAULT_CHARSET, &directory))
    return; /* error */
  TFilePtr->directory = directory;

/* ugly hack to save directory information */
  if (TFilePtr->State==1)
    {
      if (!FITS_dir) FITS_dir = MakeString(" ");
      StringFill(FITS_dir, directory); 
    }
  if (TFilePtr->State==2)
    {
      if (!log_dir) log_dir = MakeString(" ");
      StringFill(log_dir, directory); 
    }

/* Shazam disappear and die */
  XtUnmanageChild (filebox); 
  XtPopdown (XtParent (filebox)); 
  XtDestroyWidget(filebox); 
  XtFree(filename);

/* call func if specified */
  XPtr = (XPointer)TFilePtr;
  if (TFilePtr->OKfunc) TFilePtr->OKfunc(XPtr);
} /* end FileOKCB */

void TextFileCancelCB (Widget filebox, XtPointer clientData, XtPointer callData)
/* cancel file selection dialog box */
/* deletes box and calls CancelFunc if non NULL*/
{
  TextFilePtr TFilePtr = (TextFilePtr)clientData;
  XPointer XPtr;

/* Shazam disappear and die */
  XtUnmanageChild (filebox); 
  XtPopdown (XtParent (filebox)); 
  XtDestroyWidget(filebox); 

/* call func if specified */
  XPtr = (XPointer)TFilePtr;
  if (TFilePtr->CancelFunc) TFilePtr->CancelFunc(XPtr);
} /* end FileCancelCB */

