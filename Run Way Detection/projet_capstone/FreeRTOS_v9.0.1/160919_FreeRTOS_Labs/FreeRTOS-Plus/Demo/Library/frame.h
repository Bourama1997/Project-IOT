#ifndef FRAME_H
#define FRAME_H

#include "../Library/canny.h"

typedef struct frame
{
	bitmap_info_header_t ih;
	pixel_t* bitmap_data;
} frame_t;

typedef struct frame_size
{
	unsigned short header;
	unsigned int bitmap;
} frame_size_t;

frame_t read_sample_frame(const char* sample_frame_path);
frame_size_t calculate_frame_size(frame_t frame);

#endif // !FRAME_H
