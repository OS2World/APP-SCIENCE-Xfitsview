/*    Colormap functions for XFITSview */
/*-----------------------------------------------------------------------
*  Copyright (C) 1996,1998
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
#include <Xm/DrawingA.h> 
#include <Xm/MainW.h> 
#include <Xm/Scale.h> 
#include <stdlib.h>
#include <stdio.h>
#include "imagedisp.h"
#include "color.h"
#include "messagebox.h"

/* internal function prototypes */
void SetColor1(ImageDisplay* IDdata);
void SetColor2(ImageDisplay *IDdata);
void RevColor(ImageDisplay *IDdata);
void UpdateColor (ImageDisplay *IDdata);


void ColContourCB (Widget w, XtPointer clientData, XtPointer callData)
/* Color coutour colors callback */
{
  ImageDisplay *IDdata = (ImageDisplay *)clientData;
  SetColor1(IDdata);     /* set new colors */
  SetColorTable(IDdata); /* install new colors */
} /* end ColContourCB */

void PhlameCB (Widget w, XtPointer clientData, XtPointer callData)
/* Pseudo Flame colors callback */
{
  ImageDisplay *IDdata = (ImageDisplay *)clientData;
  SetColor2(IDdata);     /* set new colors */
  SetColorTable(IDdata); /* install new colors */
} /* end PhlameCB */

void GrayscaleCB (Widget w, XtPointer clientData, XtPointer callData)
/* Gray scale callback */
{
  ImageDisplay *IDdata = (ImageDisplay *)clientData;
  InitColorTable(IDdata);     /* set new colors */
  SetColorTable(IDdata); /* install new colors */
} /* end GrayscaleCB */

void ReverseColCB (Widget w, XtPointer clientData, XtPointer callData)
/* Reverse color table callback */
{
  ImageDisplay *IDdata = (ImageDisplay *)clientData;
  RevColor(IDdata);     /* set new colors */
  SetColorTable(IDdata); /* install new colors */
} /* end ReverseColCB */

void ResetColCB (Widget w, XtPointer clientData, XtPointer callData)
/* reset color table callback */
{
  ImageDisplay *IDdata = (ImageDisplay *)clientData;
  IDdata->value[0] = 128; IDdata->value[1]=128;
/* reset sliders */
  XmScaleSetValue(IDdata->BriScroll, 128);
  XmScaleSetValue(IDdata->ConScroll, 128);
  InitColorTable(IDdata);     /* set new colors */
  SetColorTable(IDdata); /* install new colors */
} /* end ResetColCB */


void SetupColorMap (Widget shell, ImageDisplay *IDdata)
/* create and install a color map - fill with grayscale ramp */
{
  int         icol, i, ncolor;
  Display     *dpy = XtDisplay (shell);
  XColor      *Colors;
  Window      windows [2];
  float       frac;

/* allocate color array */
  ncolor = XDisplayCells (dpy, XDefaultScreen (dpy));
  Colors = (XColor*) XtMalloc (sizeof (XColor) * ncolor);

  /* The first time, create a colormap and install it in the
     canvas widget's window. Also set up the window manager
     colormap windows list so both colormaps get installed,
     if the system is capable of handling multiple colormaps. */
    
  if (!IDdata->cmap) {
    IDdata->cmap = DefaultColormap (dpy, DefaultScreen(dpy));
/* Attempt to allocate colors in default color table */
    if (!XAllocColorCells(dpy, IDdata->cmap, FALSE, NULL, 0, IDdata->colut, 
			  IDdata->ncolors)) {
/* could not allocate colors - try a new color table */
      MessageShow ("Colormap full, create new one");
      IDdata->cmap = 
	XCreateColormap (dpy, XtWindow(IDdata->canvas), 
			 DefaultVisual(dpy, DefaultScreen(dpy)), AllocAll);
      if (!IDdata->cmap) {
	MessageShow ("Could not create colormap");
	return;}
/* Copy as much of the default color table as possible */
      for (i = 0; i < ncolor; i++) {
         Colors[i].pixel = i;
         Colors[i].flags = DoRed | DoGreen | DoBlue;
         }
      XQueryColors (dpy, DefaultColormap(dpy, DefaultScreen(dpy)),
         Colors, ncolor);
      XStoreColors (dpy, IDdata->cmap, Colors, ncolor);
/* install color map */
      XSetWindowColormap (dpy, XtWindow (shell), IDdata->cmap);
/* let window see changes in colormap */
/*ugly results      XSelectInput (dpy, XtWindow (shell), ColormapChangeMask);*/
/* use middle of color map */
     for (i=0; i<IDdata->ncolors;i++) 
/* 64 colors       IDdata->colut[i] = i+ncolor/2-IDdata->ncolors;*/
       IDdata->colut[i] = i+((3*ncolor)/4)-IDdata->ncolors; /* 128 colors */

    } /* end of install virtual colormap */
  } /* end of allocation/installation of colormap */

/* fill with ramp in gray */

  InitColorTable (IDdata);

/* first color always black */
  Colors[0].pixel = IDdata->colut[0];
  Colors[0].flags = DoRed|DoGreen|DoBlue;
  Colors[0].red   = Colors[0].blue = Colors[0].green =  BlackPixel(dpy, 0);
/*  XStoreColor (dpy, IDdata->cmap, &Colors[0]);*/
  IDdata->red[0] = Colors[0].red; 
  IDdata->blue[0] = Colors[0].blue; 
  IDdata->green[0] = Colors[0].green;
 
/* rest of color table */
  for (i = 0; i < IDdata->ncolors; i++) {
    Colors[i].pixel = IDdata->colut[i];
    Colors[i].flags = DoRed|DoGreen|DoBlue;
    Colors[i].red   = IDdata->red[i];
    Colors[i].blue  = IDdata->blue[i];
    Colors[i].green = IDdata->green[i];
/*    XStoreColor (dpy, IDdata->cmap, &Colors[i]);*/
    IDdata->red[i] = Colors[i].red; 
    IDdata->blue[i] = Colors[i].blue; 
    IDdata->green[i] = Colors[i].green;
  } /* end of fill color table loop */

  XStoreColors (dpy, IDdata->cmap, Colors, IDdata->ncolors);

  XtFree ((char *) Colors);
/* init Display */
  ResetColCB (IDdata->display, (XtPointer)IDdata, NULL);
} /* end of SetupColorMap */
                            
void SetColorTable (ImageDisplay* IDdata)
/* routine to change the color table */
/* IDdata->value[0] = Brightness control (0 - 255) */
/* IDdata->value[1] = Contrast control (0- 255) */
/* IDdata->ncolors = number of colors (e.g. 128) */
/* IDdata->red = default red color table (0 - 65535) */
/* IDdata->green = default green color table (0 - 65535) */
/* IDdata->blue = default blue color table (0 - 65535) */

/* The new color table is the entries from the default table with  */
/* 2(IDdata->value[0]-128) added to each and  then multiplied by  */
/* 1 + 5*((IDdata->value[1]-128)/128); */

{
   int i;
   long lColor;
   unsigned short cRed, cBlue, cGreen;
   float scale, offset;
   XColor *colors;
   Display     *dpy = XtDisplay (IDdata->canvas);

   scale = 1.0 + 5.0*((IDdata->value[1]-128.0)/128.0);
   offset=2.0*(IDdata->value[0]-128.0);

/* Don't bother if no colormap */
   if (!IDdata->cmap) return;

/* allocate color array */
  colors = (XColor*) XtMalloc (sizeof (XColor) * (IDdata->ncolors));

/* replace table / update colormap; reserve color 0=(0,0,0) for blanked */
   for (i=0; i<IDdata->ncolors; i++) {
      lColor = scale * ((float)i + offset) + 0.5;
      if (lColor>=IDdata->ncolors) lColor = IDdata->ncolors-1; 
      if (lColor<0) lColor = 0;
      if ((i>0) && (lColor==0)) lColor = 1;
      if (i==0) lColor = 0;
      colors[i].pixel = IDdata->colut[i];
      colors[i].flags = DoRed|DoGreen|DoBlue;
      colors[i].red   = IDdata->red[lColor]; 
      colors[i].green = IDdata->green[lColor]; 
      colors[i].blue  = IDdata->blue[lColor]; 
      if (i==0)
	colors[i].red = colors[i].blue = colors[i].green = BlackPixel(dpy, 0);
/*      XStoreColor (dpy, IDdata->cmap, &colors[i]);*/
   }  /* end of loop over color table */
  XStoreColors (dpy, IDdata->cmap, colors, IDdata->ncolors);
   XtFree ((char *) colors);

   return;
}  /* end of SetColorTable */


void InitColorTable(ImageDisplay* IDdata)
/* Set initial values in color table */
/* IDdata->ncolors = number of colors (e.g. 128) */
/* IDdata->red = default red color table (0 - 65535) */
/* IDdata->green = default green color table (0 - 65535) */
/* IDdata->blue = default blue color table (0 - 65535) */
{
   int i;
   long lColor;
   float frac;
   Display     *dpy = XtDisplay (IDdata->canvas);

   for (i=0; i<IDdata->ncolors; i++) {
     frac = (float)(i+1) / (float)IDdata->ncolors;
     lColor = frac * 65536.0 + frac * 256.0;
     if (lColor > 65535) lColor = 65535;
     if (i==0) lColor = BlackPixel(dpy, 0);  /* reserve black for blanked */
     IDdata->red[i] = lColor;
     IDdata->green[i] = lColor;
     IDdata->blue[i] = lColor;
   }
 } /* end InitColorTable */

void SetColor1(ImageDisplay* IDdata)
/* color scheme lifted from AIPS */
/* IDdata->ncolors = number of colors (e.g. 128) */
/* IDdata->red =  red color table*/
/* IDdata->green =  green color table*/
/* IDdata->blue =  blue color table*/
{
  short i;
  long    r, g, b, rt, gt, bt;
#if MAXCOLOR==128   /* 128 color table */
unsigned char bc_tab[128]={0,            /*blue table  */
   15,15,15,15,15,15,15, 15,15,15,15,15,15,15,15,   
   72,72,72,72,72,72,72, 72,72,72,72,72,72,72,
   127,127,127,127,127,127,127, 127,127,127,127,127,127,127,  
   203,203,203,203,203,203,203, 203,203,203,203,203,203,203,
   0,0,0,0,0,0,0, 0,0,0,0,0,0,0,
   0,0,0,0,0,0,0, 0,0,0,0,0,0,0,
   0,0,0,0,0,0,0, 0,0,0,0,0,0,0, 
   0,0,0,0,0,0,0, 0,0,0,0,0,0,0,
   0,0,0,0,0,0,0, 0,0,0,0,0,0,0};
   unsigned char gc_tab[128]={0,         /* green table  */
   15,15,15,15,15,15,15, 15,15,15,15,15,15,15,15, 
   0,0,0,0,0,0,0,  0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,  0,0,0,0,0,0,0,
   76,76,76,76,76,76,76, 76,76,76,76,76,76,76,
   59,59,59,59,59,59,59,  59,59,59,59,59,59,59,  
   229,229,229,229,229,229,229, 229,229,229,229,229,229,229,
   255,255,255,255,255,255,255, 255,255,255,255,255,255,255, 
   89,89,89,89,89,89,89, 89,89,89,89,89,89,89, 
   0,0,0,0,0,0,0,  0,0,0,0,0,0,0};
   unsigned char rc_tab[128]={0,          /*red table  */
   15,15,15,15,15,15,15,  15, 15,15,15,15,15,15,15,  
   36,36,36,36,36,36,36, 36,36,36,36,36,36,36,
   0,0,0,0,0,0,0, 0,0,0,0,0,0,0, 
   15,15,15,15,15,15,15, 15,15,15,15,15,15,15,
   0,0,0,0,0,0,0, 0,0,0,0,0,0,0, 
   0,0,0,0,0,0,0, 0,0,0,0,0,0,0,
   255,255,255,255,255,255,255, 255,255,255,255,255,255,255,  
   255,255,255,255,255,255,255, 255,255,255,255,255,255,255,
   255,255,255,255,255,255,255, 255,255,255,255,255,255,255};
#else /* 64 color table */
unsigned char bc_tab[64]={0,            /*blue table  */
   15,15,15,15,15,15,15,   72,72,72,72,72,72,72,
   127,127,127,127,127,127,127,  203,203,203,203,203,203,203,
   0,0,0,0,0,0,0,  0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,  0,0,0,0,0,0,0,
   0,0,0,0,0,0,0};
   unsigned char gc_tab[64]={0,         /* green table  */
   15,15,15,15,15,15,15,  0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,  76,76,76,76,76,76,76,
   59,59,59,59,59,59,59,  229,229,229,229,229,229,229,
   255,255,255,255,255,255,255,  89,89,89,89,89,89,89,
   0,0,0,0,0,0,0};
   unsigned char rc_tab[64]={0,          /*red table  */
   15,15,15,15,15,15,15,  36,36,36,36,36,36,36,
   0,0,0,0,0,0,0,  15,15,15,15,15,15,15,
   0,0,0,0,0,0,0,  0,0,0,0,0,0,0,
   255,255,255,255,255,255,255,  255,255,255,255,255,255,255,
   255,255,255,255,255,255,255};  /*255  */
#endif
   for (i=1; i<(short)IDdata->ncolors; i++)
     { rt = rc_tab[i]; gt = gc_tab[i]; bt = bc_tab[i];
       r = rt + rt * 256; g = gt + gt * 256; b = bt + bt * 256;
       if (r>65536) r = 65536;
       if (g>65536) g = 65536;
       if (b>65536) b = 65536;
       IDdata->red[i] =   r;
       IDdata->green[i] = g;
       IDdata->blue[i] =  b;}
}  /* End of SetColor1 */

void SetColor2(ImageDisplay *IDdata)
/* color scheme lifted from AIPS (PHLAME) */
/* IDdata->ncolors = number of colors (e.g. 128) */
/* IDdata->red =  red color table*/
/* IDdata->green =  green color table*/
/* IDdata->blue =  blue color table */
{
  short i;
  long    r, g, b, rt, gt, bt;
#if MAXCOLOR==128   /* 128 color table */
   unsigned char bc_tab[]=         /* green table  */
  {0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,  26,  53,  66,  79,  89,  99, 108, 117, 115, 133, 140, 147, 154, 161, 167,
 174, 180, 186, 191, 197, 202, 208, 212, 219, 224, 229, 233, 238, 242, 248, 252};
   unsigned char gc_tab[]=         /* green table  */
  {0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,  31,   0,   0,   0,   0,   0,   0,   0,  15,
  31,  38,  46,  51,  57,  62,  68,  72,  77,  81,  86,  89,  93,  97, 101, 104,
 107, 110, 114, 117, 120, 123, 127, 130, 133, 135, 138, 141, 144, 146, 149, 151,
 154, 156, 159, 162, 165, 167, 170, 172, 175, 179, 182, 185, 189, 192, 195, 198,
 202, 205, 209, 212, 215, 218, 222, 225, 228, 231, 234, 237, 240, 243, 246, 251};
   unsigned char rc_tab[]=          /*red table  */
  {0,   0,   0,   0,  0,   0,    0,   0,   0,   0,   0,   0,   0,   0,   0,  15,
  29,  36,  43,  49, 55,  59,   64,  68,  73,  77,  81,  84,  88,  91,  95,  98, 
 102, 105, 109, 112, 114, 117, 120, 123, 126, 128, 131, 134, 137, 139, 142, 144,
 147, 149, 152, 154, 156, 158, 161, 163, 165, 167, 170, 172, 174, 176, 178, 180,
 183, 185, 187, 189, 191, 193, 195, 196, 198, 200, 202, 204, 207, 209, 211, 212,
 214, 216, 218, 219, 221, 223, 225, 226, 228, 230, 232, 231, 235, 236, 238, 240,
 242, 244, 246, 247, 248, 249, 251, 253, 255, 255, 255, 255, 255, 255, 255, 255,
 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255};
#else /* 64 color table */
unsigned char bc_tab[64]=            /*blue table  */
  {0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,
   0,  53,  79,  99, 117, 133, 147, 161,
 174, 186, 197, 208, 219, 229, 238, 248};
   unsigned char gc_tab[64]=         /* green table  */
  {0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,  31,
  46,  57,  68,  77,  86,  93, 101, 107,
 114, 120, 127, 133, 138, 144, 149, 154,
 159, 165, 170, 175, 182, 189, 195, 202,
 209, 215, 222, 228, 234, 240, 246, 251};
   unsigned char rc_tab[64]=          /*red table  */
  {0,   0,   0,   0,   0,   0,   0,  29,
  43,  55,  64,  73,  81,  88,  95, 102,
 109, 114, 120, 126, 131, 137, 142, 147,
 152, 156, 161, 165, 170, 174, 178, 183,
 187, 191, 195, 198, 202, 207, 211, 214,
 218, 221, 225, 228, 232, 235, 238, 243,
 246, 248, 251, 255, 255, 255, 255, 255,
 255, 255, 255, 255, 255, 255, 255, 255};
#endif

   for (i=1; i<(short)IDdata->ncolors; i++) 
   { rt = rc_tab[i]; gt = gc_tab[i]; bt = bc_tab[i];
       r = rt + rt * 256; g = gt + gt * 256; b = bt + bt * 256;
       if (r>65536) r = 65536;
       if (g>65536) g = 65536;
       if (b>65536) b = 65536;
       IDdata->red[i] =   r;
       IDdata->green[i] = g;
       IDdata->blue[i] =  b;}
}  /* End of SetColor2 */

void RevColor(ImageDisplay *IDdata)
/* IDdata->ncolors = number of colors (e.g. 128) */
/* IDdata->red =  red color table  */
/* IDdata->green =  green color table */
/* IDdata->blue =  blue color table  */
{
  short i, j;
  unsigned short cTemp;
   for (i=1; i<IDdata->ncolors/2; i++)
     {j = IDdata->ncolors - i;
      cTemp = IDdata->red[i];
      IDdata->red[i] =   IDdata->red[j];
      IDdata->red[j] =   cTemp;
      cTemp = IDdata->green[i];
      IDdata->green[i] =   IDdata->green[j];
      IDdata->green[j] =   cTemp;
      cTemp = IDdata->blue[i];
      IDdata->blue[i] =   IDdata->blue[j];
      IDdata->blue[j] =   cTemp;
     }
}  /* End of RevColor */

void UpdateColor (ImageDisplay *IDdata)
/* routine to update the color map */
/* Calls SetColorTable, gives */
/* IDdata->value[0] = Brightness control (0 - 255) */
/* IDdata->value[1] = Contrast control (0- 255) */
/* IDdata->ncolors = number of colors (e.g. 128) */
/* IDdata->red = default red color table */
/* IDdata->green = default green color table */
/* IDdata->blue = default blue color table */
/* The new color table is the entries from the default table with  */
/* 2(IDdata->value[0]-128) added to each and  then multiplied by  */
/* 1 + 5*((IDdata->value[1]-128)/128); */
{
   SetColorTable (IDdata);
}  /* end of UpdateColor */

