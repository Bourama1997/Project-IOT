#include "FreeRTOS.h"
#include "../../Library/console.h"
#include "../../Library/frame.h"
#include "../../Library/rsa.h"
#include "RunwayDetection/EdgeDetectionTask/edge_detection_task.h"
#include "RunwayDetection/DataEncryptionTask/data_encryption_task.h"
#include "RunwayDetection/DataTransmissionTask/data_transmission_task.h"
#include "queue.h"
#include "task.h"
#include "encrypt_show.h"

static QueueHandle_t
	encryption_queue = NULL,
	transmission_queue = NULL;

static TaskHandle_t
	edge_detection_task,
	data_encryption_task,
	data_transmission_task;

void start_runway_detection(
	const char* sample_frame_path,
	const char* sample_filtered_frame_path,
	const char* unencrypted_frame_path,
	const char* decrypted_frame_path,
	unsigned long rsa_prime_p,
	unsigned long rsa_prime_q,
	const char* demo_message
	
)
{
	log("executed start_runway_detection(...)");

	frame_t sample_frame = read_sample_frame(sample_frame_path);
	frame_size_t frame_size = calculate_frame_size(sample_frame);

	encryption_queue = xQueueCreate(1, sizeof(frame_t));
	transmission_queue = xQueueCreate(1, sizeof(long*));

	start_edge_detection_task(
		&edge_detection_task,
		sample_frame,
		sample_filtered_frame_path,
		encryption_queue,
		frame_size
	);

	const key_pair_t key_pair = generate_key_pair(rsa_prime_p, rsa_prime_q);

	encrypt_show(key_pair, demo_message);

	start_data_encryption_task(
		&data_encryption_task,
		key_pair,
		encryption_queue,
		transmission_queue,
		frame_size,
		unencrypted_frame_path,
		decrypted_frame_path
	);

	start_data_transmission_task(
		data_transmission_task,
		transmission_queue,
		frame_size	
	);
}

void vApplicationTickHook()
{
	if (uxQueueSpacesAvailable(encryption_queue) == 0)
		vTaskPrioritySet(edge_detection_task, 1);
	else
		vTaskPrioritySet(edge_detection_task, 6);
}
