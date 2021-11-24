#ifndef DATA_ENCRYPTION_TASK
#define DATA_ENCRYPTION_TASK

#include "../../../Library/frame.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "../../../Library/rsa.h"

void start_data_encryption_task(
	TaskHandle_t* task_handle,
	key_pair_t key_pair,
	QueueHandle_t input_queue,
	QueueHandle_t output_queue,
	frame_size_t frame_size,
	const char* unencrypted_frame_path,
	const char* decrypted_frame_path
);

#endif // !DATA_ENCRYPTION_TASK
