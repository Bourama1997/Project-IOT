#ifndef EDGE_DETECTION_TASK_H
#define EDGE_DETECTION_TASK_H

#include "../../../Library/frame.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

void start_edge_detection_task(
	TaskHandle_t* task_handle,
	frame_t sample_frame,
	const char* filtered_frame_sample_path,
	QueueHandle_t output_queue,
	frame_size_t frame_size
);

#endif // !EDGE_DETECTION_TASK_H
