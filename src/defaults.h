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

#ifndef DEFAULTS_H
#define DEFAULTS_H

// from dwg_api
#ifndef _DWG_API_H_
BITCODE_T dwg_add_u8_input (Dwg_Data *restrict dwg,
                            const char *restrict u8str) __nonnull_all;
#endif

int defaults(Dwg_Data *restrict dwg);

int defaults_r1_1(Dwg_Data *restrict dwg);
int defaults_r1_2(Dwg_Data *restrict dwg);
int defaults_r1_4(Dwg_Data *restrict dwg);
int defaults_r2_0(Dwg_Data *restrict dwg);
int defaults_r2_1(Dwg_Data *restrict dwg);
int defaults_r2_4(Dwg_Data *restrict dwg);
int defaults_r2_5(Dwg_Data *restrict dwg);
int defaults_r2_6(Dwg_Data *restrict dwg);
int defaults_r9(Dwg_Data *restrict dwg);
int defaults_r10(Dwg_Data *restrict dwg);
int defaults_r11(Dwg_Data *restrict dwg);
int defaults_r12(Dwg_Data *restrict dwg);
int defaults_r13(Dwg_Data *restrict dwg);
int defaults_r14(Dwg_Data *restrict dwg);
int defaults_r2000(Dwg_Data *restrict dwg);

BITCODE_TIMEBLL actual_timebll(void);

#endif
