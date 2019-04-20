/*
*
*   Josh Taylor's Universal Utilities
*
*/

#include <stdio.h>
#include <stdarg.h>

#include "jt_util.h"

///////////////////////////////////////////
//Function to print to console and a FILE
/////////////////////////////////////////
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
