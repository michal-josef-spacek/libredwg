/*****************************************************************************/
/*  LibreDWG - free implementation of the DWG file format                    */
/*                                                                           */
/*  Copyright (C) 2023-2025 Free Software Foundation, Inc.                   */
/*                                                                           */
/*  This library is free software, licensed under the terms of the GNU       */
/*  General Public License as published by the Free Software Foundation,     */
/*  either version 3 of the License, or (at your option) any later version.  */
/*  You should have received a copy of the GNU General Public License        */
/*  along with this program.  If not, see <http://www.gnu.org/licenses/>.    */
/*****************************************************************************/

/*
 * defaults.c: default header variables
 * written by Michal Josef Špaček
 */

#include <time.h>

#include "config.h"
#include "common.h"
#include "defaults.h"
#include "dwg.h"

static unsigned int loglevel;
#define DWG_LOGLEVEL loglevel
#include "logging.h"

#define CMC_DEFAULTS 0, 0, 0, 0, NULL, NULL, NULL, 0, 0, 0

int
defaults(Dwg_Data *restrict dwg)
{
  Dwg_Version_Type version = dwg->header.version;

  switch (version)
   {
     case R_1_1:
       defaults_r1_1(dwg);
       break;
     case R_1_2:
     case R_1_3:
       defaults_r1_2(dwg);
       break;
     case R_1_4:
       defaults_r1_4(dwg);
       break;
     case R_2_0b:
     case R_2_0:
       defaults_r2_0(dwg);
       break;
     case R_2_10:
     case R_2_21:
     case R_2_22:
       defaults_r2_1(dwg);
       break;
     case R_2_4:
       defaults_r2_4(dwg);
       break;
     case R_2_5:
       defaults_r2_5(dwg);
       break;
     case R_2_6:
       defaults_r2_6(dwg);
       break;
     case R_9:
       defaults_r9(dwg);
       break;
     case R_10:
       defaults_r10(dwg);
       break;
     case R_11:
       defaults_r12(dwg);
       break;
     case R_13b1:
     case R_13b2:
     case R_13:
       defaults_r13(dwg);
       break;
     case R_14:
       defaults_r14(dwg);
       break;
     case R_2000:
       defaults_r2000(dwg);
       break;
     default:
       return 1;
   }
   return 0;
}

/* Default header variables from AutoCAD r1.1
   XXX This values are deducated.
*/
int
defaults_r1_1(Dwg_Data *restrict dwg)
{
  dwg->header_vars.EXTMIN = (BITCODE_3BD){ 0.0, 0.0, 0.0 };
  dwg->header_vars.EXTMAX = (BITCODE_3BD){ 0.0, 0.0, 0.0 };
  dwg->header_vars.LIMMIN = (BITCODE_2BD){ 0.0, 0.0 };
  dwg->header_vars.LIMMAX = (BITCODE_2BD){ 12.0, 9.0 };
  // VIEWCTR
  // VIEWSIZE
  dwg->header_vars.SNAPMODE = 0;
  dwg->header_vars.SNAPUNIT = (BITCODE_2RD){ 0.25, 0 };
  dwg->header_vars.GRIDMODE = 0;
  dwg->header_vars.GRIDUNIT = (BITCODE_2RD){ 0.25, 0 };
  dwg->header_vars.ORTHOMODE = 0;
  dwg->header_vars.REGENMODE = 1;
  dwg->header_vars.FILLMODE = 1;
  dwg->header_vars.TEXTSIZE = 0.2;
  dwg->header_vars.TRACEWID = 0.05;
  // TODO
  // CLAYER = 1
  // CLAYER: (5.1.F) abs:F [H 8]
  //dwg->header_vars.CLAYER = dwg_add_handleref (dwg, 5, UINT64_C (0x10), NULL);
  dwg->header_vars.CECOLOR = (BITCODE_CMC){ 15, CMC_DEFAULTS };

  // layer_colors
  dwg->header_vars.layer_colors[0] = 0;
  dwg->header_vars.layer_colors[1] = 15;
  for (int i = 2; i < 128; i++)
    dwg->header_vars.layer_colors[i] = 255;

  return 0;
}

/* Default header variables from AutoCAD r1.2
   Based on r1.1 structure.
   XXX This values are deducated.
 */
int
defaults_r1_2(Dwg_Data *restrict dwg)
{
  defaults_r1_1(dwg);

  /* New variables after r1.1 */
  dwg->header_vars.DIMARROW = 0;
  // XXX - Most common aspect ratio
  dwg->header_vars.aspect_ratio = 1.28623;

  return 0;
}

/* Default header variables from AutoCAD r1.40
   Based on r1.2 (r1.3?) structure.
 */
int
defaults_r1_4(Dwg_Data *restrict dwg)
{
  defaults_r1_2(dwg);

  /* Changes compared to r1.2 */
  dwg->header_vars.SNAPUNIT = (BITCODE_2RD){ 1, 0 };
  dwg->header_vars.GRIDUNIT = (BITCODE_2RD){ 0, 0 };

  /* New variables after r1.2 */
  dwg->header_vars.LUNITS = 2;
  dwg->header_vars.LUPREC = 4;
  dwg->header_vars.DIMTOL = 1;
  dwg->header_vars.DIMLIM = 1;
  dwg->header_vars.AXISMODE = 0;
  dwg->header_vars.AXISUNIT = (BITCODE_2RD){ 0, 0.25 };
  dwg->header_vars.SKETCHINC = 0.1;
  dwg->header_vars.FILLETRAD = 0;

  return 0;
}

/* Default header variables from AutoCAD r2.0b
   Based on r1.4 structure.
   XXX This values are deducated.
 */
int
defaults_r2_0(Dwg_Data *restrict dwg)
{
  defaults_r1_4(dwg);

  /* Changes compared to r1.4 */

  /* New variables after r1.4 */
  dwg->header_vars.AUNITS = 0;
  dwg->header_vars.AUPREC = 0;
  // Handle = 0
  //dwg->header_vars.TEXTSTYLE = 0;

  return 0;
}

/* Default header variables from AutoCAD r2.1.
   This is not based on r2.0.
 */
int
defaults_r2_1(Dwg_Data *restrict dwg)
{
  dwg->header_vars.INSBASE = (BITCODE_3BD){ 0.0, 0.0, 0.0 };
  dwg->header_vars.EXTMIN = (BITCODE_3BD){ 0.0, 0.0, 0.0 };
  dwg->header_vars.EXTMAX = (BITCODE_3BD){ 0.0, 0.0, 0.0 };
  dwg->header_vars.LIMMIN = (BITCODE_2BD){ 0.0, 0.0 };
  dwg->header_vars.LIMMAX = (BITCODE_2BD){ 12.0, 9.0 };
  dwg->header_vars.VIEWCTR = (BITCODE_3RD){ 7.37814, 4.5, 0 };
  dwg->header_vars.VIEWSIZE = 9;
  dwg->header_vars.SNAPMODE = 0;
  dwg->header_vars.SNAPUNIT = (BITCODE_2RD){ 1, 1 };
  dwg->header_vars.SNAPBASE = (BITCODE_2RD){ 0, 0 };
  dwg->header_vars.SNAPANG = 0;
  dwg->header_vars.SNAPSTYLE = 0;
  dwg->header_vars.SNAPISOPAIR = 0;
  dwg->header_vars.GRIDMODE = 0;
  dwg->header_vars.GRIDUNIT = (BITCODE_2RD){ 0, 0 };
  dwg->header_vars.ORTHOMODE = 0;
  dwg->header_vars.REGENMODE = 1;
  dwg->header_vars.FILLMODE = 1;
  dwg->header_vars.QTEXTMODE = 0;
  dwg->header_vars.DRAGMODE = 1;
  dwg->header_vars.LTSCALE = 1;
  dwg->header_vars.TEXTSIZE = 0.2;
  dwg->header_vars.TRACEWID = 0.05;
  // TODO CLAYER = 0 - handle
  //dwg->header_vars.CLAYER = 0;
  dwg->header_vars.oldCECOLOR_lo = 15;
  dwg->header_vars.oldCECOLOR_hi = 0;
  dwg->header_vars.unknown_5 = 0;
  dwg->header_vars.unknown_6a = 0;
  dwg->header_vars.unknown_6b = 0;
  dwg->header_vars.unknown_6c = 0;
  // XXX - Most common aspect ratio
  dwg->header_vars.aspect_ratio = 1.63959;
  dwg->header_vars.LUNITS = 2;
  dwg->header_vars.LUPREC = 4;
  dwg->header_vars.AXISMODE = 0;
  dwg->header_vars.AXISUNIT = (BITCODE_2RD){ 0, 0 };
  dwg->header_vars.SKETCHINC = 0.1;
  dwg->header_vars.FILLETRAD = 0;
  dwg->header_vars.AUNITS = 0;
  dwg->header_vars.AUPREC = 0;
  // Handle = 0
  //dwg->header_vars.TEXTSTYLE = 0;
  dwg->header_vars.OSMODE = 0;
  dwg->header_vars.ATTMODE = 1;
  dwg->header_vars.MENU = dwg_add_u8_input (dwg, "ACAD");
  dwg->header_vars.DIMSCALE = 1;
  dwg->header_vars.DIMASZ = 0.18;
  dwg->header_vars.DIMEXO = 0.0625;
  dwg->header_vars.DIMDLI = 0.38;
  dwg->header_vars.DIMEXE = 0.18;
  dwg->header_vars.DIMTP = 0;
  dwg->header_vars.DIMTM = 0;
  dwg->header_vars.DIMTXT = 0.18;
  dwg->header_vars.DIMCEN = 0.09;
  dwg->header_vars.DIMTSZ = 0;
  dwg->header_vars.DIMTOL = 0;
  dwg->header_vars.DIMLIM = 0;
  dwg->header_vars.DIMTIH = 1;
  dwg->header_vars.DIMTOH = 1;
  dwg->header_vars.DIMSE1 = 0;
  dwg->header_vars.DIMSE2 = 0;
  dwg->header_vars.DIMTAD = 0;
  dwg->header_vars.LIMCHECK = 0;
  // TODO Why MENUEXT is RC[]?
  //dwg->header_vars.MENUEXT = dwg_add_u8_input (dwg, "");
  dwg->header_vars.ELEVATION = 0;
  dwg->header_vars.THICKNESS = 0;
  dwg->header_vars.VIEWDIR = (BITCODE_3BD){ 0, 0, 1 };
  dwg->header_vars.VPOINTX = (BITCODE_3RD){ 1, 0, 0 };
  dwg->header_vars.VPOINTY = (BITCODE_3RD){ 0, 1, 0 };
  dwg->header_vars.VPOINTZ = (BITCODE_3RD){ 0, 0, 1 };
  dwg->header_vars.VPOINTXALT = (BITCODE_3RD){ 1.0, 0.0, 0.0 };
  dwg->header_vars.VPOINTYALT = (BITCODE_3RD){ 0.0, 1.0, 0.0 };
  dwg->header_vars.VPOINTZALT = (BITCODE_3RD){ 0.0, 0.0, 1.0 };
  dwg->header_vars.flag_3d = 0;
  dwg->header_vars.BLIPMODE = 1;

  return 0;
}

/* Default header variables from AutoCAD r2.4.
   This is based on r2.1.
 */
int
defaults_r2_4(Dwg_Data *restrict dwg)
{
  defaults_r2_1(dwg);

  /* New variables after r2.1 */
  dwg->header_vars.DIMZIN = 0;
  dwg->header_vars.DIMRND = 0;
  dwg->header_vars.DIMDLE = 0;
  dwg->header_vars.DIMBLK_T = dwg_add_u8_input (dwg, "");
  dwg->header_vars.circle_zoom_percent = 100;
  dwg->header_vars.COORDS = 0;
  // XXX CECOLOR.index: 256 [RS 62]
  //dwg->header_vars.CECOLOR
  // XXX CELTYPE: 256 [H(RSd) 6]
  //dwg->header_vars.CELTYPE = ;
  dwg->header_vars.TDCREATE = actual_timebll();
  dwg->header_vars.TDUPDATE = dwg->header_vars.TDCREATE;
  // XXX
  // TDINDWG: [0, 34440] 00:00:29 [TIMERLL 40]
  // XXX
  // TDUSRTIMER: [0, 34440] 00:00:29 [TIMERLL 40]
  dwg->header_vars.USRTIMER = 1;
  dwg->header_vars.FASTZOOM = 1;
  dwg->header_vars.SKPOLY = 0;
  dwg->header_vars.unknown_mon = 0;
  dwg->header_vars.unknown_day = 0;
  dwg->header_vars.unknown_year = 0;
  dwg->header_vars.unknown_hour = 0;
  dwg->header_vars.unknown_min = 0;
  dwg->header_vars.unknown_sec = 0;
  dwg->header_vars.unknown_ms = 0;
  dwg->header_vars.ANGBASE = 0;
  dwg->header_vars.ANGDIR = 0;

  return 0;
}

/* Default header variables from AutoCAD r2.5.
   This is based on r2.4.
 */
int
defaults_r2_5(Dwg_Data *restrict dwg)
{
  defaults_r2_4(dwg);

  /* New variables after r2.4 */
  dwg->header_vars.PDMODE = 0;
  dwg->header_vars.PDSIZE = 0;
  dwg->header_vars.PLINEWID = 0;
  dwg->header_vars.USERI1 = 0;
  dwg->header_vars.USERI2 = 0;
  dwg->header_vars.USERI3 = 0;
  dwg->header_vars.USERI4 = 0;
  dwg->header_vars.USERI5 = 0;
  dwg->header_vars.USERR1 = 0;
  dwg->header_vars.USERR2 = 0;
  dwg->header_vars.USERR3 = 0;
  dwg->header_vars.USERR4 = 0;
  dwg->header_vars.USERR5 = 0;

  return 0;
}

/* Default header variables from AutoCAD r2.6.
   This is based on r2.5.
 */
int
defaults_r2_6(Dwg_Data *restrict dwg)
{
  defaults_r2_5(dwg);

  /* New variables after r2.5 */
  dwg->header_vars.DIMALT = 0;
  dwg->header_vars.DIMALTD = 2;
  dwg->header_vars.DIMASO = 1;
  dwg->header_vars.DIMSHO = 0;
  dwg->header_vars.DIMPOST = dwg_add_u8_input (dwg, "");
  dwg->header_vars.DIMAPOST = dwg_add_u8_input (dwg, "");
  dwg->header_vars.DIMALTF = 25.4;
  dwg->header_vars.DIMLFAC = 1;

  return 0;
}

/* Default header variables from AutoCAD r9.
   This is based on r2.6.
 */
int
defaults_r9(Dwg_Data *restrict dwg)
{
  defaults_r2_6(dwg);

  /* Changes compared to r2.6 */
  dwg->header_vars.MENU = dwg_add_u8_input (dwg, "acad");

  /* New variables after r2.6 */
  dwg->header_vars.SPLINESEGS = 8;
  dwg->header_vars.SPLFRAME = 0;
  dwg->header_vars.ATTREQ = 1;
  dwg->header_vars.ATTDIA = 0;
  dwg->header_vars.CHAMFERA = 0;
  dwg->header_vars.CHAMFERB = 0;
  dwg->header_vars.MIRRTEXT = 1;

  return 0;
}

/* Default header variables from AutoCAD r10.
   This is based on r9.
 */
int
defaults_r10(Dwg_Data *restrict dwg)
{
  defaults_r9(dwg);

  /* Changes compared to r9 */

  /* New variables after r9 */
  dwg->header_vars.PLINEGEN = 0;
  dwg->header_vars.PSLTSCALE = 0;
  dwg->header_vars.TREEDEPTH = 0;
  dwg->header_vars.codepage = 0;
  dwg->header_vars.UCSORG = (BITCODE_3BD){ 0.0, 0.0, 0.0 };
  dwg->header_vars.UCSXDIR = (BITCODE_3BD){ 1.0, 0.0, 0.0 };
  dwg->header_vars.UCSYDIR = (BITCODE_3BD){ 0.0, 1.0, 0.0 };
  dwg->header_vars.TARGET = (BITCODE_3BD){ 0.0, 0.0, 0.0 };
  dwg->header_vars.LENSLENGTH = 50;
  dwg->header_vars.VIEWTWIST = 0;
  dwg->header_vars.FRONTZ = 0;
  dwg->header_vars.BACKZ = 0;
  dwg->header_vars.VIEWMODE = 0;
  dwg->header_vars.DIMTOFL = 0;
  dwg->header_vars.DIMBLK1_T = dwg_add_u8_input (dwg, "");
  dwg->header_vars.DIMBLK2_T = dwg_add_u8_input (dwg, "");
  dwg->header_vars.DIMSAH = 0;
  dwg->header_vars.DIMTIX = 0;
  dwg->header_vars.DIMSOXD = 0;
  dwg->header_vars.DIMTVP = 0;
  dwg->header_vars.unknown_string = dwg_add_u8_input (dwg, "");
  dwg->header_vars.HANDLING = 0;
  // XXX HANDSEED: 0.8.0 [H 5]
  // Based on what is in DWG file.
  //dwg->header_vars.HANDSEED = dwg_add_handleref (dwg, 0, UINT64_C (0x25), NULL);
  dwg->header_vars.SURFU = 6;
  dwg->header_vars.SURFV = 6;
  dwg->header_vars.SURFTYPE = 6;
  dwg->header_vars.SURFTAB1 = 6;
  dwg->header_vars.SURFTAB2 = 6;
  dwg->header_vars.FLATLAND = 0;
  dwg->header_vars.SPLINETYPE = 6;
  dwg->header_vars.UCSICON = 0;
  // XXX UCSNAME: -1 [H(RSd) 2]
  // In r13 is UCSNAME: (5.0.0) abs:0 [H 2]
  //dwg->header_vars.UCSNAME = dwg_add_handleref (dwg, -1, UINT64_C (0x25), NULL);

  return 0;
}

/* Default header variables from AutoCAD r11.
   This is based on r10.
 */
int
defaults_r11(Dwg_Data *restrict dwg)
{
  defaults_r10(dwg);

  /* Changes compared to r10 */

  /* New variables after r10 */
  dwg->header_vars.WORLDVIEW = 1;
  dwg->header_vars.unknown_51e = 5;
  dwg->header_vars.unknown_520 = 1;
  dwg->header_vars.unknown_52c = -1;
  dwg->header_vars.unknown_52e = 0;
  dwg->header_vars.unknown_530 = 0;
  dwg->header_vars.DIMCLRD_C = 0;
  dwg->header_vars.DIMCLRE_C = 0;
  dwg->header_vars.DIMCLRT_C = 0;
  dwg->header_vars.SHADEDGE = 3;
  dwg->header_vars.SHADEDIF = 70;
  dwg->header_vars.unknown_59 = 0;
  dwg->header_vars.UNITMODE = 0;
  // XXX 412148564080.0
  dwg->header_vars.unit1_ratio = 4.12149e+11;
  dwg->header_vars.unit2_ratio = 1;
  dwg->header_vars.unit3_ratio = 1;
  dwg->header_vars.unit4_ratio = 1;
  dwg->header_vars.unit1_name = dwg_add_u8_input (dwg, "m");
  dwg->header_vars.unit2_name = dwg_add_u8_input (dwg, "");
  dwg->header_vars.unit3_name = dwg_add_u8_input (dwg, "");
  dwg->header_vars.unit4_name = dwg_add_u8_input (dwg, "");
  dwg->header_vars.DIMTFAC = 1;
  dwg->header_vars.PUCSORG = (BITCODE_3BD){ 0, 0, 0 };
  dwg->header_vars.PUCSXDIR = (BITCODE_3BD){ 1.0, 0.0, 0.0 };
  dwg->header_vars.PUCSYDIR = (BITCODE_3BD){ 0.0, 1.0, 0.0 };
  // XXX PUCSNAME: -1 [H(RSd) 2]
  // In r13 is PUCSNAME: (5.0.0) abs:0 [H 2]
  //dwg->header_vars.PUCSNAME = -1;
  dwg->header_vars.TILEMODE = 1;
  dwg->header_vars.PLIMCHECK = 0;
  dwg->header_vars.unknown_10 = 0;
  dwg->header_vars.PEXTMIN
    = (BITCODE_3BD){ 100000000000000000000.0, 100000000000000000000.0,
                     100000000000000000000.0 };
  dwg->header_vars.PEXTMAX
    = (BITCODE_3BD){ -100000000000000000000.0, -100000000000000000000.0,
                     -100000000000000000000.0 };
  dwg->header_vars.PLIMMIN = (BITCODE_2BD){ 0.0, 0.0 };
  dwg->header_vars.PLIMMAX = (BITCODE_2BD){ 12.0, 9.0 };
  dwg->header_vars.PINSBASE = (BITCODE_3BD){ 0.0, 0.0, 0.0 };
  dwg->header_vars.MAXACTVP = 16;
  dwg->header_vars.DIMGAP = 0.09;
  dwg->header_vars.PELEVATION = 0;
  dwg->header_vars.VISRETAIN = 0;

  return 0;
}

/* Default header variables from AutoCAD r12.
   This is based on r11.
 */
int
defaults_r12(Dwg_Data *restrict dwg)
{
  defaults_r11(dwg);

  /* Changes compared to r11 */
  dwg->header_vars.PSLTSCALE = 1;
  dwg->header_vars.TREEDEPTH = 3020;

  return 0;
}

/* Default header variables from AutoCAD r13.
   This is based on r12.
 */
int
defaults_r13(Dwg_Data *restrict dwg)
{
  defaults_r12(dwg);

  /* Changes compared to r12 */
  dwg->header_vars.DIMSHO = 1;
  dwg->header_vars.DRAGMODE = 2;
  dwg->header_vars.COORDS = 1;
  dwg->header_vars.CECOLOR = (BITCODE_CMC){ 256, CMC_DEFAULTS };
  dwg->header_vars.EXTMIN
    = (BITCODE_3BD){ 100000000000000000000.0, 100000000000000000000.0,
                     100000000000000000000.0 };
  dwg->header_vars.EXTMAX
    = (BITCODE_3BD){ -100000000000000000000.0, -100000000000000000000.0,
                     -100000000000000000000.0 };

  /* New variables after r12 */
  dwg->header_vars.CHAMFERC = 0;
  dwg->header_vars.CHAMFERD = 0;
  dwg->header_vars.FACETRES = 0.5;
  dwg->header_vars.DIMSAV = 0;
  dwg->header_vars.unknown_8 = 12;
  dwg->header_vars.unknown_9 = 0;
  dwg->header_vars.unknown_10 = 0;
  dwg->header_vars.WIREFRAME = 0;
  dwg->header_vars.DELOBJ = 1;
  dwg->header_vars.DISPSILH = 0;
  dwg->header_vars.PELLIPSE = 0;
  dwg->header_vars.PROXYGRAPHICS = 0;
  dwg->header_vars.PICKSTYLE = 1;
  dwg->header_vars.ISOLINES = 4;
  dwg->header_vars.CMLJUST = 0;
  dwg->header_vars.TEXTQLTY = 50;
  dwg->header_vars.CMLSCALE = 1;
  dwg->header_vars.CELTSCALE = 1;
  dwg->header_vars.TDUCREATE = actual_timebll();
  dwg->header_vars.TDUUPDATE = dwg->header_vars.TDUCREATE;
  dwg->header_vars.DIMSD1 = 0;
  dwg->header_vars.DIMSD2 = 0;
  dwg->header_vars.DIMTOLJ = 1;
  dwg->header_vars.DIMJUST = 0;
  dwg->header_vars.DIMFIT = 3;
  dwg->header_vars.DIMUPT = 0;
  dwg->header_vars.DIMTZIN = 0;
  dwg->header_vars.DIMALTZ = 0;
  dwg->header_vars.DIMALTTZ = 0;
  dwg->header_vars.DIMUNIT = 2;
  dwg->header_vars.DIMAUNIT = 0;
  dwg->header_vars.DIMDEC = 4;
  dwg->header_vars.DIMTDEC = 4;
  dwg->header_vars.DIMALTU = 2;
  dwg->header_vars.DIMALTTD = 2;
  dwg->header_vars.DIMTXSTY = dwg_add_handleref (dwg, 5, UINT64_C (0x11), NULL);
  dwg->header_vars.DIMCLRD = (BITCODE_CMC){ 0, CMC_DEFAULTS };
  dwg->header_vars.DIMCLRE = (BITCODE_CMC){ 0, CMC_DEFAULTS };
  dwg->header_vars.DIMCLRT = (BITCODE_CMC){ 0, CMC_DEFAULTS };
  dwg->header_vars.unknown_54 = 0;
  dwg->header_vars.unknown_55 = 0;
  dwg->header_vars.unknown_56 = 0;
  dwg->header_vars.unknown_57 = 0;

  return 0;
}

/* Default header variables from AutoCAD r14.
   This is based on r12. XXX
 */
int
defaults_r14(Dwg_Data *restrict dwg)
{
  defaults_r13(dwg);

  dwg->header.dwg_version = 0;
  dwg->header.is_maint = 0;
  dwg->header.maint_version = 0;
  dwg->header.zero_one_or_three = 1;

  /* XXX Hardcoded. */
  dwg->header_vars.HANDSEED
    = dwg_add_handleref (dwg, 0, UINT64_C (0x25), NULL);

  dwg->header_vars.BLIPMODE = 0;
  dwg->header_vars.CHAMFERA = 0.5;
  dwg->header_vars.CHAMFERB = 0.5;
  dwg->header_vars.CHAMFERC = 1.0;
  dwg->header_vars.FILLETRAD = 0.5;
  dwg->header_vars.MAXACTVP = 48;
  dwg->header_vars.MENU = dwg_add_u8_input (dwg, ".");
  dwg->header_vars.PROXYGRAPHICS = 1;
  dwg->header_vars.SKPOLY = 1;
  dwg->header_vars.VISRETAIN = 1;

  return 0;
}

/* Default header variables from AutoCAD r2000.
   This is based on r12. XXX
 */
int
defaults_r2000(Dwg_Data *restrict dwg)
{
  defaults_r14(dwg);

  dwg->header.dwg_version = 0x17;
  dwg->header.is_maint = 0x6;
  dwg->header.maint_version = 0x6;
  dwg->header.zero_one_or_three = 1;

  /* XXX Hardcoded. */
  dwg->header_vars.HANDSEED
    = dwg_add_handleref (dwg, 0, UINT64_C (0x2c), NULL);

  dwg->header_vars.DIMATFIT = 3;
  dwg->header_vars.DIMDSEP = 46;
  dwg->header_vars.DIMLWD = -2;
  dwg->header_vars.DIMLWE = -2;
  dwg->header_vars.DIMLUNIT = 2;
  /* TODO FINGERPRINTGUID */
  /* TODO VERSIONGUID */
  dwg->header_vars.INSUNITS = 1;
  dwg->header_vars.FLAGS = 10781;
  dwg->header_vars.MAXACTVP = 64;
  dwg->header_vars.SKPOLY = 0;

  return 0;
}

BITCODE_TIMEBLL
actual_timebll(void)
{
  BITCODE_TIMEBLL ret;

  time_t now = time (NULL);
  BITCODE_RLL days = now / 86400L;
  BITCODE_RLL ms = 1000 * (now % 86400L);
  long dzoff = 1000 * tm_offset ();
  ms += dzoff;
  // julian days until the 1970 epoch: 2440587.5
  // (https://planetcalc.com/503/?date=1970-01-01%2000%3A00%3A00)
  days += 2440588L;
  ret = (BITCODE_TIMEBLL){ (BITCODE_BL)days, (BITCODE_BL)ms, days + (ms * 1e-8) };

  return ret;
}

