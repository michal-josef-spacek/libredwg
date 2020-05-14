#include "../../src/config.h"
#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define __BSD_VISIBLE 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <sys/stat.h>
#include <dirent.h>
#include "../../src/common.h"
#include "../../src/classes.h"
static unsigned int loglevel;
#define DWG_LOGLEVEL loglevel
#include "../../src/logging.h"

#include "dwg.h"
#include "dwg_api.h"
#include "tests_common.h"

dwg_data g_dwg;
int g_counter;
#define MAX_COUNTER 6
int g_countmax = MAX_COUNTER;

// in radians. angles are not normalized. max observed: 10.307697 in ELLIPSE.end_angle
#define MAX_ANGLE 12.57

// needs Dwg_Version_Type dwg_version = obj->parent->header.version;
#undef PRE
#undef VERSION
#undef VERSIONS
#undef UNTIL
#undef SINCE
#define VERSION(version) if (dwg_version == version)
#define VERSIONS(v1, v2)   if (dwg_version >= v1 && dwg_version <= v2)
#define PRE(version)   if (dwg_version < version)
#define UNTIL(version) if (dwg_version <= version)
#define SINCE(version) if (dwg_version >= version)
#define LATER_VERSIONS else

/// test a DWG file
int test_code (const char *filename, int cov);

/// test all DWG's in a subdir
int test_subdirs (const char *dirname, int cov);

/// Return the name of a handle
char *handle_name (const Dwg_Data *restrict dwg, Dwg_Object_Ref *restrict hdl);

/// iterate over objects of a block
void output_BLOCK_HEADER (dwg_object_ref *ref);

/// Check the dwg type and calls output_process
void output_object (dwg_object *obj);

/// blocks to be iterated over
void output_test (dwg_data *dwg);

void output_process (dwg_object *obj);

/// Low Level processing function declaration
void low_level_process (dwg_object *obj);

/// API based processing function declaration
void api_process (dwg_object *obj);

void api_common_entity (dwg_object *obj);

void api_common_object (dwg_object *obj);

/// API based printing function declaration
void print_api (dwg_object *obj);

/// Low level printing function declaration
void print_low_level (dwg_object *obj);

/// Main function
int
main (int argc, char *argv[])
{
  char *input = getenv ("INPUT");
  char *dir = NULL;
  int error = 0;
  int i = 1, cov = 1;
  //#ifdef USE_TRACING
  char *probe = getenv ("LIBREDWG_TRACE");
  if (probe)
    loglevel = atoi (probe);
  //#endif

  if (argc > i)
    {
      // don't warn/error on no coverage. for unit_testing_all.sh
      if (strEQc (argv[i], "-n"))
        {
          cov = 0;
          i++;
        }
      // --all, not stopping at 6
      else if (strEQc (argv[i], "-an"))
        {
          g_countmax = 1000;
          cov = 0;
          i++;
        }
      else if (strEQc (argv[i], "-a"))
        {
          g_countmax = 1000;
          i++;
        }
      // process subdirs
      if (argc > i && *argv[i] != '-')
        dir = argv[i];
    }

  if (!input)
    {
      char **ptr;
      struct stat attrib;
      const char *const files[]
          = { "example_2000.dwg", "example_2004.dwg", "example_2007.dwg",
              "example_2010.dwg", "example_2013.dwg", "example_2018.dwg",
              "example_r14.dwg",
              //"2000/PolyLine2D.dwg",
              //"2007/PolyLine3D.dwg",
              NULL };
      const char *prefix = "../test-data/";

      if (dir)
        {
          if (!stat (dir, &attrib) &&
              S_ISDIR (attrib.st_mode))
#ifndef HAVE_SCANDIR
            fprintf (stderr, "dir argument not supported yet on this platform (missing scandir)\n");
#else
            return test_subdirs (dir, cov);
#endif
        }

      /* ../configure out-of-tree. find srcdir */
      if (stat (prefix, &attrib))
        prefix = "../../../test/test-data/";
      if (stat (prefix, &attrib)) /* dir not found. one more level up */
        prefix = "../../../../test/test-data/";

      for (ptr = (char **)&files[0]; *ptr; ptr++)
        {
          if (stat (*ptr, &attrib))
            {
              char tmp[80];
              strcpy (tmp, prefix);
              strcat (tmp, *ptr);
              if (stat (tmp, &attrib))
                fprintf (stderr, "Env var INPUT not defined, %s not found\n",
                         tmp);
              else
                error += test_code (tmp, cov);
            }
          else
            error += test_code (*ptr, cov);
        }
      // if no coverage
      if (!numpassed () && !numfailed ())
        {
          char tmp[80];
          if (DWG_TYPE == DWG_TYPE_POLYLINE_2D || DWG_TYPE == DWG_TYPE_SEQEND
              || DWG_TYPE == DWG_TYPE_VERTEX_2D)
            {
              strcpy (tmp, prefix);
              strcat (tmp, "2000/PolyLine2D.dwg");
              error += test_code (tmp, cov);
            }
          if (DWG_TYPE == DWG_TYPE_POLYLINE_3D
              || DWG_TYPE == DWG_TYPE_VERTEX_3D)
            {
              strcpy (tmp, prefix);
              strcat (tmp, "2000/PolyLine3D.dwg");
              error += test_code (tmp, cov);
              strcpy (tmp, prefix);
              strcat (tmp, "2004/PolyLine3D.dwg");
              error += test_code (tmp, cov);
              strcpy (tmp, prefix);
              strcat (tmp, "2018/PolyLine3D.dwg");
              error += test_code (tmp, cov);
            }
          if (DWG_TYPE == DWG_TYPE_POLYLINE_MESH
              || DWG_TYPE == DWG_TYPE_VERTEX_MESH || DWG_TYPE == DWG_TYPE_TRACE
              || DWG_TYPE == DWG_TYPE_DIMENSION_ANG3PT
              || DWG_TYPE == DWG_TYPE_DIMENSION_DIAMETER
              || DWG_TYPE == DWG_TYPE_DIMENSION_RADIUS
              || DWG_TYPE == DWG_TYPE_OLE2FRAME)
            {
              strcpy (tmp, prefix);
              strcat (tmp, "2000/TS1.dwg");
              error += test_code (tmp, cov);
            }
          if (DWG_TYPE == DWG_TYPE_IMAGE || DWG_TYPE == DWG_TYPE_LEADER)
            {
              strcpy (tmp, prefix);
              strcat (tmp, "2000/Leader.dwg");
              error += test_code (tmp, cov);
              strcpy (tmp, prefix);
              strcat (tmp, "2004/Leader.dwg");
              error += test_code (tmp, cov);
              strcpy (tmp, prefix);
              strcat (tmp, "2018/Leader.dwg");
              error += test_code (tmp, cov);
            }
          if (DWG_TYPE == DWG_TYPE_HATCH)
            {
              strcpy (tmp, prefix);
              strcat (tmp, "2004/HatchG.dwg");
              error += test_code (tmp, cov);
            }
          if (DWG_TYPE == DWG_TYPE_UNDERLAY || DWG_TYPE == DWG_TYPE_UNDERLAYDEFINITION)
            {
              strcpy (tmp, prefix);
              strcat (tmp, "2004/Underlay.dwg");
              error += test_code (tmp, cov);
            }
          if (DWG_TYPE == DWG_TYPE_LIGHT || DWG_TYPE == DWG_TYPE_VISUALSTYLE)
            {
              strcpy (tmp, prefix);
              strcat (tmp, "2004/Visualstyle.dwg");
              error += test_code (tmp, cov);
              strcpy (tmp, prefix);
              strcat (tmp, "2018/Visualstyle.dwg");
              error += test_code (tmp, cov);
              strcpy (tmp, prefix);
            }
          if (DWG_TYPE == DWG_TYPE_GEODATA)
            {
              strcpy (tmp, prefix);
              strcat (tmp, "2010/gh209_1.dwg");
              error += test_code (tmp, cov);
            }
          if (DWG_TYPE == DWG_TYPE_PLOTSETTINGS)
            {
              strcpy (tmp, prefix);
              strcat (tmp, "2013/gh109_1.dwg");
              error += test_code (tmp, cov);
            }
          if (DWG_TYPE == DWG_TYPE_DBCOLOR)
            {
              strcpy (tmp, prefix);
              strcat (tmp, "2004/dbcolor.dwg");
              error += test_code (tmp, cov);
              strcpy (tmp, prefix);
              strcat (tmp, "2004/Truecolor.dwg");
              error += test_code (tmp, cov);
            }
          if (DWG_TYPE == DWG_TYPE_HELIX)
            {
              strcpy (tmp, prefix);
              strcat (tmp, "2000/work.dwg");
              error += test_code (tmp, cov);
              strcpy (tmp, prefix);
              strcat (tmp, "2004/Helix.dwg");
              error += test_code (tmp, cov);
              strcpy (tmp, prefix);
              strcat (tmp, "2018/Helix.dwg");
              error += test_code (tmp, cov);
            }
          if (DWG_TYPE == DWG_TYPE_ASSOCPLANESURFACEACTIONBODY ||
              DWG_TYPE == DWG_TYPE_ASSOCPERSSUBENTMANAGER ||
              DWG_TYPE == DWG_TYPE_ASSOCACTION)
            {
              strcpy (tmp, prefix);
              strcat (tmp, "2004/Surface.dwg");
              error += test_code (tmp, cov);
            }
          if (DWG_TYPE == DWG_TYPE_ASSOCNETWORK)
            {
              strcpy (tmp, prefix);
              strcat (tmp, "2000/Constraints.dwg");
              error += test_code (tmp, cov);
              strcpy (tmp, prefix);
              strcat (tmp, "2004/Constraints.dwg");
              error += test_code (tmp, cov);
              strcpy (tmp, prefix);
              strcat (tmp, "2007/Constraints.dwg");
              error += test_code (tmp, cov);
              strcpy (tmp, prefix);
              strcat (tmp, "2010/Constraints.dwg");
              error += test_code (tmp, cov);
              strcpy (tmp, prefix);
              strcat (tmp, "2013/Constraints.dwg");
              error += test_code (tmp, cov);
            }
          if (DWG_TYPE == DWG_TYPE_ACSH_BOX_CLASS ||
              DWG_TYPE == DWG_TYPE_ACSH_EXTRUSION_CLASS)
            {
              strcpy (tmp, prefix);
              strcat (tmp, "2013/JW.dwg");
              error += test_code (tmp, cov);
            }
          if (DWG_TYPE == DWG_TYPE_BLKREFOBJECTCONTEXTDATA)
            {
              strcpy (tmp, prefix);
              strcat (tmp, "2004/Visualstyle.dwg");
              error += test_code (tmp, cov);
              strcpy (tmp, prefix);
              strcat (tmp, "2018/Visualstyle.dwg");
              error += test_code (tmp, cov);
              strcpy (tmp, prefix);
              strcat (tmp, "2010/ACI_20160321_A_30_east.dwg");
              error += test_code (tmp, cov);
            }
          if (DWG_TYPE == DWG_TYPE_LEADEROBJECTCONTEXTDATA)
            {
              strcpy (tmp, prefix);
              strcat (tmp, "2000/Leader.dwg");
              error += test_code (tmp, cov);
              strcpy (tmp, prefix);
              strcat (tmp, "2004/Leader.dwg");
              error += test_code (tmp, cov);
              strcpy (tmp, prefix);
              strcat (tmp, "2013/gh55-ltype.dwg");
              error += test_code (tmp, cov);
            }
          if (DWG_TYPE == DWG_TYPE_MLEADEROBJECTCONTEXTDATA ||
              DWG_TYPE == DWG_TYPE_MTEXTOBJECTCONTEXTDATA)
            {
              strcpy (tmp, prefix);
              strcat (tmp, "2010/ACI_20160321_A_30_east.dwg");
              error += test_code (tmp, cov);
            }
          if (DWG_TYPE == DWG_TYPE_MTEXTATTRIBUTEOBJECTCONTEXTDATA)
            {
              strcpy (tmp, prefix);
              strcat (tmp, "2013/gh55-ltype.dwg");
              error += test_code (tmp, cov);
              strcpy (tmp, prefix);
              strcat (tmp, "2010/ACI_20160321_A_30_east.dwg");
              error += test_code (tmp, cov);
            }
          if (DWG_TYPE == DWG_TYPE_FIELD || DWG_TYPE == DWG_TYPE_FIELDLIST)
            {
              strcpy (tmp, prefix);
              strcat (tmp, "2000/TS1.dwg");
              error += test_code (tmp, cov);
              strcpy (tmp, prefix);
              strcat (tmp, "2010/5151-024.dwg");
              error += test_code (tmp, cov);
            }
          if (DWG_TYPE == DWG_TYPE_DATALINK)
            {
              strcpy (tmp, prefix);
              strcat (tmp, "2010/5151-019.dwg");
              error += test_code (tmp, cov);
              strcpy (tmp, prefix);
              strcat (tmp, "2010/5151-022.dwg");
              error += test_code (tmp, cov);
              strcpy (tmp, prefix);
              strcat (tmp, "2010/5151-023.dwg");
              error += test_code (tmp, cov);
              strcpy (tmp, prefix);
              strcat (tmp, "2010/5151-024.dwg");
              error += test_code (tmp, cov);
            }
          if (DWG_TYPE == DWG_TYPE_DATATABLE)
            {
              strcpy (tmp, prefix);
              strcat (tmp, "2004/fr05_b101_ref.dwg");
              error += test_code (tmp, cov);
              strcpy (tmp, prefix);
              strcat (tmp, "2000/TS1.dwg");
              error += test_code (tmp, cov);
              strcpy (tmp, prefix);
              strcat (tmp, "r13/TS1.dwg");
              error += test_code (tmp, cov);
            }
          if (DWG_TYPE == DWG_TYPE_SORTENTSTABLE)
            {
              strcpy (tmp, prefix);
              strcat (tmp, "2000/PolyLine2D.dwg");
              error += test_code (tmp, cov);
              strcpy (tmp, prefix);
              strcat (tmp, "2004/Surface.dwg");
              error += test_code (tmp, cov);
              strcpy (tmp, prefix);
              strcat (tmp, "r13/PolyLine2D.dwg");
              error += test_code (tmp, cov);
              strcpy (tmp, prefix);
              strcat (tmp, "2004/Publish.dwg");
              error += test_code (tmp, cov);
            }
          if (DWG_TYPE == DWG_TYPE_SUN)
            {
              strcpy (tmp, prefix);
              strcat (tmp, "2000/2.dwg");
              error += test_code (tmp, cov);
              strcpy (tmp, prefix);
              strcat (tmp, "2000/3.dwg");
              error += test_code (tmp, cov);
              strcpy (tmp, prefix);
              strcat (tmp, "2000/4.dwg");
              error += test_code (tmp, cov);
              strcpy (tmp, prefix);
              strcat (tmp, "2000/5.dwg");
              error += test_code (tmp, cov);
            }
          if (DWG_TYPE == DWG_TYPE_SECTIONOBJECT ||
              DWG_TYPE == DWG_TYPE_SECTION_MANAGER ||
              DWG_TYPE == DWG_TYPE_SECTION_SETTINGS)
            {
              strcpy (tmp, prefix);
              strcat (tmp, "2018/LiveSection.dwg");
              error += test_code (tmp, cov);
            }
        }
#ifdef DWG_TYPE
      if (cov && !numpassed () && !numfailed ())
        printf ("TODO no coverage for %s\n", dwg_type_name (DWG_TYPE));
#endif
    }
  else
    error = test_code (input, cov);

  return error;
}

/// test all DWG's in all subdirs. not on windows yet
int
test_subdirs (const char *dir, int cov)
{
  int error = 0, n;
  struct stat attrib;
  struct dirent **namelist;

#ifdef HAVE_SCANDIR
  n = scandir (dir, &namelist, NULL, NULL);
  if (n == -1)
    {
      perror ("scandir");
      exit (1);
    }
  while (n--)
    {
      char *elem = namelist[n]->d_name;
      char path[256];
      path[255] = '\0';
      strncpy (path, dir, 254);
      strncat (path, "/", 254);
      strncat (path, elem, 254);
      path[255] = '\0';

      if (namelist[n]->d_type == DT_DIR && *elem != '.')
        {
          error += test_subdirs (path, cov);
        }
      if (namelist[n]->d_type == DT_REG &&
          (strstr (elem, ".dwg") || strstr (elem, ".DWG")))
        {
          error += test_code (path, cov);
        }

      free (namelist[n]);
    }
  free (namelist);
#endif
  return error;
}

/// test a DWG file
int
test_code (const char *filename, int cov)
{
  int error;

#ifdef DWG_TYPE
  // only process if no coverage yet, or when we are crossing the unicode boundary.
  if (strstr (filename, "2018") ||
      strstr (filename, "2007") ||
      (!numpassed () && !numfailed ()))
    {
      if (cov)
        printf ("Testing with %s:\n", filename);
    }
  else if (cov)
    {
      printf ("Skipping %s:\n", filename);
      return 0;
    }
#endif
  error = dwg_read_file (filename, &g_dwg);
  if (error < DWG_ERR_CRITICAL)
    {
      output_test (&g_dwg);
      dwg_free (&g_dwg);
    }
  /* This value is the return value for `main',
     so clamp it to either 0 or 1.  */
  error = (error >= DWG_ERR_CRITICAL || numfailed () > 0) ? 1 : 0;
#ifdef DWG_TYPE
  if (is_type_unstable (DWG_TYPE) || is_type_debugging (DWG_TYPE))
    {
      if (cov && error)
        printf ("%s failed (TODO: unstable)\n", filename);
      return 0;
    }
#endif
  if (cov && error)
    printf ("%s failed\n", filename);
  return error;
}

/// Iterate over the objects in a block
void
output_BLOCK_HEADER (dwg_object_ref *ref)
{
  dwg_object *hdr, *obj;
  int error;
  g_counter = 0;

  if (!ref)
    return;
  hdr = dwg_ref_get_object (ref, &error);
  if (!hdr)
    return;

  obj = get_first_owned_entity (hdr);
  while (obj)
    {
      output_object (obj);
      if (dwg_obj_has_subentity (obj))
        {
          Dwg_Object *owner = obj;
          obj = get_first_owned_subentity (owner);
          while (obj)
            {
              output_object (obj);
              obj = get_next_owned_subentity (owner, obj);
            }
          obj = get_next_owned_entity (hdr, owner);
        }
      else
        obj = get_next_owned_entity (hdr, obj);
    }
}

/// Iterate over both modelspace and paperspace blocks
/// And if nothing found, over all other blocks.
void
output_test (dwg_data *dwg)
{
  int error;
  dwg_object *obj;
  dwg_obj_block_header *_hdr;
  dwg_obj_block_control *_ctrl;
  dwg_object_ref *ref, **hdr_refs;

  dwg_api_init_version (dwg);
  _hdr = dwg_get_block_header (dwg, &error);
  _ctrl = dwg_block_header_get_block_control (_hdr, &error);

#ifndef DWG_TYPE
  /* process all owned entities */
  ref = dwg_obj_block_control_get_model_space (_ctrl, &error);
  if (!error)
    {
      LOG_INFO ("mspace\n");
      output_BLOCK_HEADER (ref);
    }
  ref = dwg_obj_block_control_get_paper_space (_ctrl, &error);
  if (!error)
    {
      LOG_INFO ("pspace\n");
      output_BLOCK_HEADER (ref);
    }
#endif

#ifdef DWG_TYPE
  obj = &dwg->object[0];
  while ((obj = dwg_next_object (obj)))
    {
      LOG_INFO ("  %s [%d]\n", obj->name, obj->index);
      // printf ("%s [%d]\n", obj->name, obj->index);
      if (obj->fixedtype == DWG_TYPE)
        {
          g_counter++;
          output_process (obj);
        }
    }
    /* also process blocks? we better find DWGs with these */
  if (!numpassed () && !numfailed ())
    {
      /* and now also all subtypes and entities in blocks */
      unsigned int j;
      unsigned int num_hdr_objs = dwg_obj_block_control_get_num_entries(_ctrl, &error);
      if (error || !num_hdr_objs)
        return;
      hdr_refs = dwg_obj_block_control_get_block_headers(_ctrl, &error);
      if (error)
        return;
      for (j = 0; j < num_hdr_objs; j++)
        {
          if (hdr_refs[j])
            output_BLOCK_HEADER (hdr_refs[j]);
        }
      free (hdr_refs);
    }
#endif
}

/// Main output function that prints to the terminal
void
output_process (dwg_object *obj)
{
  // print_low_level (obj);
  print_api (obj);
}

#ifdef DWG_TYPE
void
output_object (dwg_object *obj)
{
  if (!obj)
    {
      printf ("object is NULL\n");
      return;
    }
  LOG_INFO ("  %s [%d]\n", obj->name, obj->index);
  if (obj->fixedtype == DWG_TYPE)
    {
      g_counter++;
      output_process (obj);
    }
}
#endif

/// Low level printing function (ignored)
void
print_low_level (dwg_object *obj)
{
  printf ("\n");
  printf ("Printed via low level access:\n");
  // low_level_process (obj);
  printf ("\n");
}

/// API based printing function
void
print_api (dwg_object *obj)
{
#ifdef DWG_TYPE
  printf ("Unit-testing type %d %s [%d]:\n", DWG_TYPE, obj->name, g_counter);
#else
  printf ("Test dwg_api and dynapi [%d]:\n", g_counter);
#endif
  api_process (obj);

  if (obj->supertype == DWG_SUPERTYPE_ENTITY && obj->fixedtype != DWG_TYPE_UNKNOWN_ENT)
    api_common_entity (obj);
  else if (obj->supertype == DWG_SUPERTYPE_OBJECT && obj->fixedtype != DWG_TYPE_UNKNOWN_OBJ)
    api_common_object (obj);
  if (g_counter <= g_countmax)
    printf ("\n");
}

#define CHK_COMMON_TYPE(ent, field, type)                                     \
  {                                                                           \
    if (dwg_dynapi_common_value (ent, #field, &field, NULL))                  \
      {                                                                       \
        if (field == ent->parent->field)                                      \
          pass (); /*(#field ":\t" FORMAT_##type, field);*/                   \
        else                                                                  \
          fail (#field ":\t" FORMAT_##type, field);                           \
      }                                                                       \
    else                                                                      \
      fail (#field);                                                          \
  }
#define CHK_COMMON_TYPE2(ent, field, type, value)                             \
  {                                                                           \
    if (dwg_dynapi_common_value (ent, #field, &value, NULL))                  \
      {                                                                       \
        if (value == ent->parent->field)                                      \
          pass (); /*(#field ":\t" FORMAT_##type, value);*/                   \
        else                                                                  \
          fail (#field ":\t" FORMAT_##type, value);                           \
      }                                                                       \
    else                                                                      \
      fail (#field);                                                          \
  }

#define CHK_COMMON_H(ent, field)                                              \
  {                                                                           \
    if (!dwg_dynapi_common_value (ent, #field, &field, NULL))                 \
      fail (#field);                                                          \
    else if (!field)                                                          \
      pass ();                                                                \
    else                                                                      \
      {                                                                       \
        char *_hdlname = dwg_dynapi_handle_name (dwg, field);                 \
        if (field == (BITCODE_H)ent->parent->field)                           \
          {                                                                   \
            if (g_counter > g_countmax)                                       \
              pass ();                                                        \
            else                                                              \
              ok (#field ": %s " FORMAT_REF, _hdlname ? _hdlname : "",        \
                  ARGS_REF (field));                                          \
          }                                                                   \
        else                                                                  \
          fail (#field ": %s " FORMAT_REF, _hdlname ? _hdlname : "",          \
                ARGS_REF (field));                                            \
        if (version >= R_2007)                                                \
          free (_hdlname);                                                    \
      }                                                                       \
  }

#define CHK_COMMON_HV(ent, field, num)                                        \
  if (!dwg_dynapi_common_value (ent, #field, &field, NULL))                   \
    fail (#field);                                                            \
  else if (!field)                                                            \
    pass ();                                                                  \
  else                                                                        \
    {                                                                         \
      for (int _i = 0; _i < (int)(num); _i++)                                 \
        {                                                                     \
          BITCODE_H _hdl = field[_i];                                         \
          char *_hdlname = _hdl ? dwg_dynapi_handle_name (dwg, _hdl) : NULL;  \
          if (_hdl == ent->parent->field[_i])                                 \
            {                                                                 \
              if (g_counter > g_countmax)                                     \
                pass ();                                                      \
              else                                                            \
                {                                                             \
                  if (_hdl)                                                   \
                    ok (#field "[%d]: %s " FORMAT_REF, _i, _hdlname ?: "",    \
                        ARGS_REF (_hdl));                                     \
                  else                                                        \
                    ok (#field "[%d]: NULL", _i);                             \
                }                                                             \
            }                                                                 \
          else                                                                \
            {                                                                 \
              if (_hdl)                                                       \
                fail (#field "[%d]: %s " FORMAT_REF, _i, _hdlname ?: "",      \
                      ARGS_REF (_hdl));                                       \
              else                                                            \
                {                                                             \
                  if (g_counter > g_countmax)                                 \
                    pass ();                                                  \
                  else                                                        \
                    ok (#field "[%d]: NULL", _i);                             \
                }                                                             \
            }                                                                 \
          if (_hdlname && version >= R_2007)                                  \
            free (_hdlname);                                                  \
        }                                                                     \
    }

void
api_common_entity (dwg_object *obj)
{
  BITCODE_BB entmode;
  BITCODE_H ownerhandle;
  BITCODE_BL num_reactors, num_eed;
  BITCODE_H *reactors;
  BITCODE_B is_xdic_missing, has_ds_data, preview_exists;
  BITCODE_RC linewt;
  BITCODE_RL preview_size_rl;
  BITCODE_BLL preview_size;
  BITCODE_BD ltype_scale;
  BITCODE_BS invisible;
  BITCODE_H xdicobjhandle;
  BITCODE_H prev_entity;
  BITCODE_H next_entity;
  BITCODE_H layer;
  BITCODE_H ltype;
  BITCODE_H material;
  BITCODE_H shadow;
  BITCODE_H plotstyle;
  BITCODE_H full_visualstyle;
  BITCODE_H face_visualstyle;
  BITCODE_H edge_visualstyle;
  Dwg_Data *dwg = obj->parent;
  Dwg_Version_Type version = obj->parent->header.version;
  Dwg_Object_Entity *_ent = obj->tio.entity;
  Dwg_Entity_LINE *ent = obj->tio.entity->tio.LINE;

  CHK_COMMON_TYPE (ent, entmode, BB)
  CHK_COMMON_TYPE (ent, preview_exists, B)
  if (preview_exists)
    {
      if (version >= R_2010)
        CHK_COMMON_TYPE (ent, preview_size, BLL)
      else
        CHK_COMMON_TYPE2 (ent, preview_size, RL, preview_size_rl);
    }
  if (entmode == 3 || entmode == 0)
    CHK_COMMON_H (ent, ownerhandle);
  CHK_COMMON_H (ent, layer);
  if (version < R_2000)
    {
      if (_ent->isbylayerlt)
        CHK_COMMON_H (ent, ltype);
      if (!_ent->nolinks)
        {
          CHK_COMMON_H (ent, prev_entity);
          CHK_COMMON_H (ent, next_entity);
        }
    }
  else
    {
      if (_ent->ltype_flags == 3)
        CHK_COMMON_H (ent, ltype);
    }
  CHK_COMMON_TYPE (ent, linewt, RC);
  CHK_COMMON_TYPE (ent, ltype_scale, BD);
  if (version >= R_2007)
    {
      if (_ent->material_flags == 3)
        CHK_COMMON_H (ent, material);
      if (_ent->shadow_flags == 3)
        CHK_COMMON_H (ent, shadow);
    }
  if (version >= R_2000)
    {
      if (_ent->plotstyle_flags == 3)
        CHK_COMMON_H (ent, plotstyle);
    }
  if (version >= R_2010)
    {
      if (_ent->has_full_visualstyle)
        CHK_COMMON_H (ent, full_visualstyle);
      if (_ent->has_face_visualstyle)
        CHK_COMMON_H (ent, face_visualstyle);
      if (_ent->has_edge_visualstyle)
        CHK_COMMON_H (ent, edge_visualstyle);
    }
  CHK_COMMON_TYPE (ent, invisible, BS);

  CHK_COMMON_TYPE (ent, is_xdic_missing, B);
  if (!is_xdic_missing)
    CHK_COMMON_H (ent, xdicobjhandle);
  CHK_COMMON_TYPE (ent, num_reactors, BL);
  CHK_COMMON_HV (ent, reactors, num_reactors);
  CHK_COMMON_TYPE (ent, has_ds_data, B);
  CHK_COMMON_TYPE (ent, num_eed, BL);
}

#define _CHK_ENTITY_UTF8TEXT(ent, name, field)                                \
  if (dwg_dynapi_entity_utf8text (ent, #name, #field, &field, &isnew, NULL))  \
    {                                                                         \
      if (g_counter > g_countmax)                                             \
        pass ();                                                              \
      else                                                                    \
        ok (#name "." #field ":\t\"%s\"", field);                             \
    }                                                                         \
  else                                                                        \
    {                                                                         \
      Dwg_Version_Type _dwg_version = ent->parent->dwg->header.version;       \
      if (_dwg_version < R_2007)                                              \
        fail (#name "." #field ":\t\"%s\"", field);                           \
      else                                                                    \
        fail (#name "." #field ":\t\"%s\"...", field);                        \
    }

#define CHK_ENTITY_UTF8TEXT(ent, name, field)                                 \
  _CHK_ENTITY_UTF8TEXT (ent, name, field);                                    \
  if (isnew)                                                                  \
    free (field)

#define CHK_ENTITY_TYPE(ent, name, field, type)                               \
  if (!dwg_dynapi_entity_value (ent, #name, #field, &field, NULL))            \
    fail (#name "." #field);                                                  \
  else                                                                        \
    {                                                                         \
      if (field == ent->field)                                                \
        {                                                                     \
          if (g_counter > g_countmax)                                         \
            pass ();                                                          \
          else                                                                \
            ok (#name "." #field ":\t" FORMAT_##type, field);                 \
        }                                                                     \
      else                                                                    \
        fail (#name "." #field ":\t" FORMAT_##type " [" #type "]", field);    \
    }

#define CHK_ENTITY_CMC(ent, name, field)                                      \
  if (!dwg_dynapi_entity_value (ent, #name, #field, &field, NULL))            \
    fail (#name "." #field);                                                  \
  else                                                                        \
    {                                                                         \
      if (memcmp (&field, &ent->field, sizeof (Dwg_Color)) == 0)              \
        {                                                                     \
          if (g_counter > g_countmax)                                         \
            pass ();                                                          \
          else                                                                \
            ok (#name "." #field ":\t" FORMAT_BSd, field.index);              \
        }                                                                     \
      else                                                                    \
        fail (#name "." #field ":\t" FORMAT_BSd " [CMC]", field.index);       \
    }

#define CHK_ENTITY_H(ent, name, field)                                        \
  {                                                                           \
    if (!dwg_dynapi_entity_value (ent, #name, #field, &field, NULL))          \
      fail (#name "." #field);                                                \
    else if (!field)                                                          \
      pass ();                                                                \
    else                                                                      \
      {                                                                       \
        Dwg_Version_Type _version = ent->parent->dwg->header.version;         \
        char *_hdlname = dwg_dynapi_handle_name (obj->parent, field);         \
        if (field == ent->field)                                              \
          {                                                                   \
            if (g_counter > g_countmax)                                       \
              pass ();                                                        \
            else                                                              \
              ok (#name "." #field ": %s " FORMAT_REF,                        \
                  _hdlname ? _hdlname : "", ARGS_REF (field));                \
          }                                                                   \
        else                                                                  \
          {                                                                   \
            fail (#name "." #field ": %s " FORMAT_REF,                        \
                  _hdlname ? _hdlname : "", ARGS_REF (field));                \
          }                                                                   \
        if (_version >= R_2007)                                               \
          free (_hdlname);                                                    \
      }                                                                       \
  }

#define CHK_ENTITY_HV(ent, name, field, num)                                  \
  if (!dwg_dynapi_entity_value (ent, #name, #field, &field, NULL))            \
    fail (#name "." #field);                                                  \
  else if (!field)                                                            \
    pass ();                                                                  \
  else                                                                        \
    {                                                                         \
      Dwg_Version_Type _version = ent->parent->dwg->header.version;           \
      for (int _i = 0; _i < (int)(num); _i++)                                 \
        {                                                                     \
          BITCODE_H _hdl = field[_i];                                         \
          char *_hdlname = dwg_dynapi_handle_name (obj->parent, _hdl);        \
          if (_hdl == ent->field[_i])                                         \
            {                                                                 \
              if (g_counter > g_countmax)                                     \
                pass ();                                                      \
              else                                                            \
                ok (#name "." #field "[%d]: %s " FORMAT_REF, _i,              \
                    _hdlname ? _hdlname : "", ARGS_REF (_hdl));               \
            }                                                                 \
          else                                                                \
            {                                                                 \
              fail (#name "." #field "[%d]: %s " FORMAT_REF, _i,              \
                    _hdlname ? _hdlname : "", ARGS_REF (_hdl));               \
            }                                                                 \
          if (_version >= R_2007)                                             \
            free (_hdlname);                                                  \
        }                                                                     \
    }

// i must be defined as type of num
#define CHK_ENTITY_VECTOR(ent, name, field, num)                              \
  if (!dwg_dynapi_entity_value (ent, #name, #field, &field, NULL))            \
    fail (#name "." #field);                                                  \
  else if (!field)                                                            \
    pass ();                                                                  \
  else                                                                        \
    {                                                                         \
      for (i = 0; i < (num); i++)                                             \
        {                                                                     \
          ok ("%s.%s[%d]:", #name, #field, i);                                \
          if (memcmp (&ent->field[i], &field[i], sizeof (field[i])))          \
            fail ("%s.%s[%d]:", #name, #field, i);                            \
        }                                                                     \
    }

// i must be defined as type of num
#define CHK_ENTITY_3DPOINTS(ent, name, field, num)                            \
  if (!dwg_dynapi_entity_value (ent, #name, #field, &field, NULL))            \
    fail (#name "." #field);                                                  \
  else if (!field)                                                            \
    pass ();                                                                  \
  else                                                                        \
    {                                                                         \
      for (i = 0; i < (num); i++)                                             \
        {                                                                     \
          ok ("%s.%s[%d]: (%f, %f, %f)", #name, #field, i, field[i].x,        \
              field[i].y, field[i].z);                                        \
          if (memcmp (&ent->field[i], &field[i], sizeof (field[i])))          \
            fail ("%s.%s[%d]: memcmp", #name, #field, i);                     \
        }                                                                     \
    }

#define CHK_ENTITY_2RD(ent, name, field)                                      \
  if (!dwg_dynapi_entity_value (ent, #name, #field, &field, NULL))            \
    fail (#name "." #field);                                                  \
  else                                                                        \
    {                                                                         \
      if (field.x == ent->field.x && field.y == ent->field.y)                 \
        {                                                                     \
          if (g_counter > g_countmax)                                         \
            pass ();                                                          \
          else                                                                \
            ok (#name "." #field ":\t(%f, %f)", field.x, field.y);            \
        }                                                                     \
      else                                                                    \
        fail (#name "." #field ":\t(%f, %f)", field.x, field.y);              \
    }

#define CHK_ENTITY_3RD(ent, name, field)                                      \
  if (!dwg_dynapi_entity_value (ent, #name, #field, &field, NULL))            \
    fail (#name "." #field);                                                  \
  else                                                                        \
    {                                                                         \
      if (field.x == ent->field.x && field.y == ent->field.y                  \
          && field.z == ent->field.z)                                         \
        {                                                                     \
          if (g_counter > g_countmax)                                         \
            pass ();                                                          \
          else                                                                \
            ok (#name "." #field ":\t(%f, %f, %f)", field.x, field.y,         \
                field.z);                                                     \
        }                                                                     \
      else                                                                    \
        fail (#name "." #field ":\t(%f, %f, %f)", field.x, field.y, field.z); \
    }

#define CHK_ENTITY_TIMEBLL(ent, name, field)                                  \
  if (!dwg_dynapi_entity_value (ent, #name, #field, &field, NULL))            \
    fail (#name "." #field);                                                  \
  else                                                                        \
    ok ("%s.%s " FORMAT_BL "." FORMAT_BL, #name, #field, field.days, field.ms);

#define CHK_ENTITY_MAX(ent, name, field, type, _max)                          \
  if (field > _max)                                                           \
    fail ("Invalid " #name "." #field " " FORMAT_##type " > " #_max, field)

#define _DWGAPI_ENT_NAME(name, field) dwg_ent_##name##_get_##field
#define DWGAPI_ENT_NAME(ent, field) _DWGAPI_ENT_NAME (ent, field)
#define _DWGAPI_OBJ_NAME(name, field) dwg_obj_##name##_get_##field
#define DWGAPI_OBJ_NAME(ent, field) _DWGAPI_OBJ_NAME (ent, field)
#define DWGAPI_ENTOBJ_NAME(ent, field)                                        \
  (ent->parent->dwg->object[ent->parent->objid].supertype == DWG_SUPERTYPE_OBJECT  \
    ? _DWGAPI_OBJ_NAME (ent, field)                                           \
    : _DWGAPI_ENT_NAME (ent, field))

#define CHK_ENTITY_UTF8TEXT_W_OLD(ent, name, field)                           \
  _CHK_ENTITY_UTF8TEXT (ent, name, field);                                    \
  {                                                                           \
    Dwg_Version_Type _dwg_version = ent->parent->dwg->header.version;         \
    if (ent->field)                                                           \
      {                                                                       \
        char *old = DWGAPI_ENT_NAME (ent, field) (ent, &error);               \
        if (error || (old && strcmp (old, field)))                            \
          fail ("old API dwg_ent_" #ent "_get_" #field ": \"%s\"", old);      \
        if (_dwg_version >= R_2007)                                           \
          free (old);                                                         \
      }                                                                       \
    if (isnew)                                                                \
      free (field);                                                           \
  }
#define CHK_ENTITY_UTF8TEXT_W_OBJ(ent, name, field)                           \
  _CHK_ENTITY_UTF8TEXT (ent, name, field);                                    \
  {                                                                           \
    Dwg_Version_Type _dwg_version = ent->parent->dwg->header.version;         \
    if (ent->field)                                                           \
      {                                                                       \
        char *old = DWGAPI_OBJ_NAME (ent, field) (ent, &error);               \
        if (error || (old && strcmp (old, field)))                            \
          fail ("old API dwg_obj_" #ent "_get_" #field ": \"%s\"", old);      \
        else                                                                  \
          pass ();                                                            \
        if (_dwg_version >= R_2007)                                           \
          free (old);                                                         \
      }                                                                       \
    if (isnew)                                                                \
      free (field);                                                           \
  }

#define CHK_ENTITY_TYPE_W_OLD(ent, name, field, type)                         \
  {                                                                           \
    BITCODE_##type old;                                                       \
    CHK_ENTITY_TYPE (ent, name, field, type);                                 \
    old = DWGAPI_ENT_NAME (ent, field) (ent, &error);                         \
    if (error || old != field)                                                \
      fail ("old API dwg_ent_" #ent "_get_" #field ": " FORMAT_##type         \
            " != " FORMAT_##type,                                             \
            old, field);                                                      \
    else                                                                      \
      pass ();                                                                \
  }

#define CHK_ENTITY_TYPE_W_OBJ(ent, name, field, type)                         \
  {                                                                           \
    BITCODE_##type old;                                                       \
    CHK_ENTITY_TYPE (ent, name, field, type);                                 \
    old = DWGAPI_OBJ_NAME (ent, field) (ent, &error);                         \
    if (error || old != field)                                                \
      fail ("old API dwg_obj_" #ent "_get_" #field ": " FORMAT_##type " != "  \
        FORMAT_##type, old, field);                                           \
    else                                                                      \
      pass ();                                                                \
  }

#define CHK_ENTITY_2RD_W_OLD(ent, name, field)                                \
  CHK_ENTITY_2RD (ent, name, field);                                          \
  {                                                                           \
    dwg_point_2d _pt2d;                                                       \
    DWGAPI_ENT_NAME (ent, field) (ent, &_pt2d, &error);                       \
    if (error || memcmp (&field, &_pt2d, sizeof (field)))                     \
      fail ("old API dwg_ent_" #ent "_get_" #field);                          \
    else                                                                      \
      pass ();                                                                \
  }

#define CHK_ENTITY_3RD_W_OLD(ent, name, field)                         	      \
  CHK_ENTITY_3RD (ent, name, field);                                          \
  {                                                                           \
    dwg_point_3d _pt3d;                                                       \
    DWGAPI_ENT_NAME (ent, field) (ent, &_pt3d, &error);                       \
    if (error || memcmp (&field, &_pt3d, sizeof (field)))                     \
      fail ("old API dwg_ent_" #ent "_get_" #field);                          \
    else                                                                      \
      pass ();                                                                \
  }

#define CHK_SUBCLASS_TYPE(strct, name, field, typ)                              \
  {                                                                           \
    BITCODE_##typ value;                                                      \
    if (!dwg_dynapi_subclass_value (&strct, #name, #field, &value, NULL))       \
      fail (#name "." #field);                                                \
    else                                                                      \
      {                                                                       \
        if (strct.field == value)                                               \
          {                                                                   \
            if (g_counter > g_countmax)                                       \
              pass ();                                                        \
            else                                                              \
              ok (#name "." #field ":\t" FORMAT_##typ, strct.field);            \
          }                                                                   \
        else                                                                  \
          fail (#name "." #field ":\t" FORMAT_##typ " [" #typ "]",            \
                strct.field);                                                   \
      }                                                                       \
  }
#define CHK_SUBCLASS_3RD(strct, name, field)                                    \
  {                                                                           \
    BITCODE_3RD value;                                                        \
    if (!dwg_dynapi_subclass_value (&strct, #name, #field, &value, NULL))       \
      fail (#name "." #field);                                                \
    else                                                                      \
      {                                                                       \
        if (value.x == strct.field.x && value.y == strct.field.y                  \
            && value.z == strct.field.z)                                        \
          {                                                                   \
            if (g_counter > g_countmax)                                       \
              pass ();                                                        \
            else                                                              \
              ok (#name "." #field ":\t(%f, %f, %f)", value.x, value.y,       \
                  value.z);                                                   \
          }                                                                   \
        else                                                                  \
          fail (#name "." #field ":\t(%f, %f, %f)", value.x, value.y,         \
                value.z);                                                     \
      }                                                                       \
  }
#define CHK_SUBCLASS_2RD(strct, name, field)                                    \
  {                                                                           \
    BITCODE_2RD value;                                                        \
    if (!dwg_dynapi_subclass_value (&strct, #name, #field, &value, NULL))       \
      fail (#name "." #field);                                                \
    else                                                                      \
      {                                                                       \
        if (value.x == strct.field.x && value.y == strct.field.y)                 \
          {                                                                   \
            if (g_counter > g_countmax)                                       \
              pass ();                                                        \
            else                                                              \
              ok (#name "." #field ":\t(%f, %f)", value.x, value.y);          \
          }                                                                   \
        else                                                                  \
          fail (#name "." #field ":\t(%f, %f)", value.x, value.y);            \
      }                                                                       \
  }
#define CHK_SUBCLASS_H(strct, name, field)                                      \
  {                                                                           \
    BITCODE_H value;                                                          \
    if (!dwg_dynapi_subclass_value (&strct, #name, #field, &value, NULL))       \
      fail (#name "." #field);                                                \
    else                                                                      \
      {                                                                       \
        char *_hdlname                                                        \
            = value ? dwg_dynapi_handle_name (obj->parent, value) : NULL;     \
        if (!value)                                                           \
          {                                                                   \
            if (!strct.field)                                                   \
              {                                                               \
                if (g_counter > g_countmax)                                   \
                  pass ();                                                    \
                else                                                          \
                  ok (#name "." #field ":\tNULL");                            \
              }                                                               \
            else                                                              \
              fail (#name "." #field ":\tNULL");                              \
          }                                                                   \
        else if (memcmp (&strct.field, &value, sizeof value) == 0)              \
          {                                                                   \
            if (g_counter > g_countmax)                                       \
              pass ();                                                        \
            else                                                              \
              ok (#name "." #field ":\t %s " FORMAT_REF, _hdlname ?: "",      \
                  ARGS_REF (value));                                          \
          }                                                                   \
        else                                                                  \
          fail (#name "." #field ":\t %s " FORMAT_REF, _hdlname ?: "",        \
                ARGS_REF (value));                                            \
        if (_hdlname && dwg_version >= R_2007)                                \
          free (_hdlname);                                                    \
      }                                                                       \
  }
#define CHK_SUBCLASS_UTF8TEXT(strct, name, field)                             \
  {                                                                           \
    BITCODE_TV value;                                                         \
    if (dwg_dynapi_subclass_value (&strct, #name, #field, &value, NULL))      \
      {                                                                       \
        if (g_counter > g_countmax)                                           \
          pass ();                                                            \
        else                                                                  \
          ok (#name "." #field ":\t\"%s\"", field);                           \
      }                                                                       \
    else                                                                      \
      {                                                                       \
        if (dwg_version < R_2007)                                             \
          fail (#name "." #field ":\t\"%s\"", field);                         \
        else                                                                  \
          fail (#name "." #field);                                            \
      }                                                                       \
  }
#define CHK_SUBCLASS_CMC(strct, name, field)                                    \
  if (!dwg_dynapi_subclass_value (&strct, #name, #field, &strct.field, NULL))     \
    fail (#name "." #field);                                                  \
  else                                                                        \
    {                                                                         \
      if (g_counter > g_countmax)                                             \
        pass ();                                                              \
      else                                                                    \
        ok (#name "." #field ":\t%d", strct.field.index);                       \
    }
#define CHK_SUBCLASS_MAX(strct, name, field, type, _max)                        \
  if (strct.field > _max)                                                       \
    fail ("Invalid " #name "." #field " " FORMAT_##type " > " #_max, strct.field)


void
api_common_object (dwg_object *obj)
{
  BITCODE_H ownerhandle, xdicobjhandle, handleref;
  BITCODE_BL num_reactors, num_eed;
  BITCODE_H *reactors;
  BITCODE_B is_xdic_missing, has_ds_data;
  Dwg_Data *dwg = obj->parent;
  Dwg_Version_Type version = obj->parent->header.version;
  Dwg_Object_Object *obj_obj = obj->tio.object;
  Dwg_Object_LAYER *_obj = obj->tio.object->tio.LAYER;

  CHK_COMMON_H (_obj, ownerhandle);
  CHK_COMMON_TYPE (_obj, is_xdic_missing, B);
  if (!is_xdic_missing)
    CHK_COMMON_H (_obj, xdicobjhandle);
  CHK_COMMON_TYPE (_obj, num_reactors, BL);
  CHK_COMMON_HV (_obj, reactors, num_reactors);
  CHK_COMMON_H (_obj, handleref); //?
  CHK_COMMON_TYPE (_obj, has_ds_data, B);
  CHK_COMMON_TYPE (_obj, num_eed, BL);
}

// allow old deprecated API
GCC31_DIAG_IGNORE (-Wdeprecated-declarations)
GCC46_DIAG_IGNORE (-Wdeprecated-declarations)
