/*
*
*   Josh Taylor's Universal Utilities
*
*/

#include <stdio.h>
#include <stdarg.h>

#include "memwatch/memwatch.h"
#include "jt_util.h"

/**********************************************
 * Print to console and a FILE with new-lines *
 **********************************************/
void jlog(char *format, ...)
{
   va_list v_ptr;
   FILE *fp;

   fp = fopen(LOGFILE_NAME, "at");
   if (fp)
   {
      va_start(v_ptr, format);

      vfprintf(fp, format, v_ptr);
      fprintf(fp, "\n");

      vprintf(format, v_ptr);
      printf("\n");

      va_end(v_ptr);

      fclose(fp);
    }
}

/***********************************************
 * Returns an X or Y from an index value       *
 * multiplied by size of each column in pixels *
 ***********************************************/
int convert_index_to_pixel_xy(unsigned char index_source, int num_columns, int size_in_pixels, int return_x_or_y)
{
   int result = 0;

   if (return_x_or_y == RETURN_X)
   {
      result = (index_source % num_columns) * size_in_pixels;

   }
   else if (return_x_or_y == RETURN_Y)
   {
      result = (index_source / num_columns) * size_in_pixels;
   }

   return result;
}
