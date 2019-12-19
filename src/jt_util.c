/*
*
*   Josh Taylor's Universal Utilities
*
*/

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "jt_util.h"

/**********************************************
 * Print to console and a FILE with new-lines *
 **********************************************/
void jlog(const char *format, ...)
{
   #ifdef DEBUG
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
    #endif // DEBUG
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



//Quick paste strlcpy I got from https://stackoverflow.com/questions/18547251/when-i-use-strlcpy-function-in-c-the-compilor-give-me-an-error
#ifndef HAVE_STRLCAT
/*
 * '_cups_strlcat()' - Safely concatenate two strings.
 */

size_t strlcat(char *dst, const char *src, size_t size)
{
  size_t srclen;         /* Length of source string */
  size_t dstlen;         /* Length of destination string */

 /*
  * Figure out how much room is left...
  */

  dstlen = strlen(dst);
  size -= dstlen + 1;

  if (!size)
    return (dstlen);        /* No room, return immediately... */

 /*
  * Figure out how much room is needed...
  */

  srclen = strlen(src);

 /*
  * Copy the appropriate amount...
  */

  if (srclen > size)
    srclen = size;

  memcpy(dst + dstlen, src, srclen);
  dst[dstlen + srclen] = '\0';

  return (dstlen + srclen);
}
#endif /* !HAVE_STRLCAT */

#ifndef HAVE_STRLCPY
/*
 * '_cups_strlcpy()' - Safely copy two strings.
 */

size_t strlcpy(char *dst, const char *src, size_t size)
{
  size_t srclen;         /* Length of source string */

 /*
  * Figure out how much room is needed...
  */

  size --;

  srclen = strlen(src);

 /*
  * Copy the appropriate amount...
  */

  if (srclen > size)
    srclen = size;

  memcpy(dst, src, srclen);
  dst[srclen] = '\0';

  return (srclen);
}
#endif /* !HAVE_STRLCPY */