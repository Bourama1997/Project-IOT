#include "console.h"
#include "frame.h"

frame_t read_sample_frame(const char* sample_frame_path)
{
	log("executed read_sample_frame(\"%s\")", sample_frame_path);

	frame_t sample_frame;

	sample_frame.bitmap_data = load_bmp(
		sample_frame_path,
		&sample_frame.ih
	);

	if (sample_frame.bitmap_data == NULL)
		log("sample frame not loaded");

	else
		log("sample frame info: %d x %d x %d",
			sample_frame.ih.width,
			sample_frame.ih.height,
			sample_frame.ih.bitspp
		);

	return sample_frame;
}

frame_size_t calculate_frame_size(frame_t frame)
{
	frame_size_t size;

	size.header = sizeof(frame.ih);
	size.bitmap = frame.ih.bmp_bytesz * sizeof(pixel_t);

	return size;
}
