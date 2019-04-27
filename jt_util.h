#ifndef JT_UTIL_H
#define JT_UTIL_H

//Macro for jlog(). Change this to a file name of your choosing
#define LOGFILE_NAME "ttedit.log"

//Macros for the convert_index_to_pixel_xy() function
#define RETURN_X 0
#define RETURN_Y 1

void jlog(char *format, ...);
int convert_index_to_pixel_xy(unsigned char index_source, int num_columns, int size_in_pixels, int return_x_or_y);

#endif // JT_UTIL_H
