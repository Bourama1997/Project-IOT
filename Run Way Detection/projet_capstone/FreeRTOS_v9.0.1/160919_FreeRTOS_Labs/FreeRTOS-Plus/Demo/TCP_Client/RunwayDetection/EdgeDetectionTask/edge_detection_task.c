#include <stdio.h>
#include "edge_detection_task.h"
#include "../../../Library/console.h"
#include "../../../Library/frame.h"

typedef struct task_init_data
{
	frame_t frame;
	char* filtered_frame_sample_path;
	QueueHandle_t output_queue;
	frame_size_t frame_size;
} task_init_data_t;

static task_init_data_t task_init_data;

static void edge_detection_task(void* pv_parameters)
{	
	log("executed edge_detection_task(0x%x)", (unsigned int)pv_parameters);

	task_init_data_t* init_data = (task_init_data_t*)pv_parameters;
	unsigned short saved_sample = 0;

	pixel_t* out_bitmap_data = calloc(
		init_data->frame.ih.bmp_bytesz,
		sizeof(pixel_t)
	);

	frame_t frame;

	while (1) {
		TickType_t current_tick_count = xTaskGetTickCount();

		canny_edge_detection(
			init_data->frame.bitmap_data,
			&init_data->frame.ih,
			40,
			80,
			1.0,
			out_bitmap_data
		);

		frame.ih = init_data->frame.ih;
		frame.bitmap_data = out_bitmap_data;

		if (out_bitmap_data == NULL) {
			log("failed canny_edge_detection");
		}
		else {
			unsigned short frame_was_pushed = 0;
			unsigned short full_queue = 0;

			do {
				frame_was_pushed = xQueueSend(
					init_data->output_queue,
					(void*)&frame,
					(TickType_t)0
				);

				if (frame_was_pushed) {
					//log("edge_detection_task has pushed a frame into the encryption queue");
				}
				else if (!full_queue) {
					full_queue = 1;
					//log("the encryption queue is now full");
				}
			} while (!frame_was_pushed);

			if (!saved_sample) {
				if (save_bmp(
					init_data->filtered_frame_sample_path,
					&init_data->frame.ih, out_bitmap_data)) {
					log("filtered frame sample not saved");
				}
				else {
					saved_sample = 1;
					log("stored filtered frame sample in: %s", init_data->filtered_frame_sample_path);
				}
			}
		}

		//log("edge_detection_task took %lu ticks to execute", xTaskGetTickCount() - current_tick_count);
	}

	free((pixel_t*)out_bitmap_data);
}

void start_edge_detection_task(
	TaskHandle_t* task_handle,
	frame_t sample_frame,
	const char* filtered_frame_sample_path,
	QueueHandle_t ouput_queue,
	frame_size_t frame_size
)
{
	log("executed start_edge_detection_task(...)");

	task_init_data.frame = sample_frame;
	task_init_data.filtered_frame_sample_path = (char*)filtered_frame_sample_path;
	task_init_data.output_queue = ouput_queue;
	task_init_data.frame_size = frame_size;

	xTaskCreate(
		edge_detection_task,
		"edge_detection_task",
		configMINIMAL_STACK_SIZE,
		(void*)&task_init_data,
		2,
		task_handle
	);
}
