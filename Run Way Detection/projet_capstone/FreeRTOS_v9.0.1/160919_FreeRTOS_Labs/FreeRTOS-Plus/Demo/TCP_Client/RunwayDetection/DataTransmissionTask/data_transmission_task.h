#ifndef DATA_TRANSMISSION_TASK_H
#define DATA_TRANSMISSION_TASK_H

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "../../../Library/frame.h"

void start_data_transmission_task(
	TaskHandle_t* task_handle,
	QueueHandle_t input_queue,
	frame_size_t frame_size
);

#endif // !DATA_TRANSMISSION_TASK_H
