#include "../../../Library/console.h"
#include "data_encryption_task.h"
#include "FreeRTOS.h"

typedef struct task_init_data
{
	key_pair_t key_pair;
	QueueHandle_t input_queue;
	QueueHandle_t output_queue;
	frame_size_t frame_size;
	const char* unencrypted_frame_path;
	const char* decrypted_frame_path;
} task_init_data_t;

static task_init_data_t task_init_data;

void data_encryption_task(void* pv_parameters)
{
	log("executed data_encryption_task(0x%x)", (unsigned int)pv_parameters);

	unsigned short saved_unencrypted_sample = 0;
	unsigned short saved_decrypted_sample = 1;

	task_init_data_t* init_data = (task_init_data_t*)pv_parameters;
	const unsigned int complete_frame_size = init_data->frame_size.header
		+ init_data->frame_size.bitmap;

	long
		* unencrypted_frame_wrapper = calloc(complete_frame_size, sizeof(long)),
		* encrypted_frame_wrapper = calloc(complete_frame_size, sizeof(long));

	while (1) {
		//TickType_t current_tick_count = xTaskGetTickCount();

		frame_t unencrypted_frame;

		unsigned int i;
		unsigned short frame_was_read = 0;
		unsigned short empty_queue = 0;

		do {
			frame_was_read = xQueueReceive(
				init_data->input_queue,
				(void*)&unencrypted_frame,
				(TickType_t)0
			);

			if (frame_was_read) {
				//log("data_encryption_task has read a frame from the encryption queue");
			}
			else if (!empty_queue) {
				empty_queue = 1;
				//log("the encryption queue is now empty");
			}
		} while (!frame_was_read);

		if (!saved_unencrypted_sample) {
			if (save_bmp(
				init_data->unencrypted_frame_path,
				&unencrypted_frame.ih, unencrypted_frame.bitmap_data)) {
				log("unencrypted frame sample not saved");
			}
			else {
				saved_unencrypted_sample = 1;
				log("stored unencrypted frame sample in: %s", init_data->unencrypted_frame_path);
			}
		}

		for (i = 0; i < init_data->frame_size.header; ++i)
			unencrypted_frame_wrapper[i] = ((unsigned char*)&unencrypted_frame.ih)[i];

		for (; (i - init_data->frame_size.header) < init_data->frame_size.bitmap; ++i)
			unencrypted_frame_wrapper[i] = ((unsigned char*)unencrypted_frame.bitmap_data)[i - init_data->frame_size.header];

		encrypt(
			encrypted_frame_wrapper,
			unencrypted_frame_wrapper,
			init_data->key_pair.public_key,
			init_data->key_pair.rsa_modulus,
			complete_frame_size,
			0, 0
		);

		if (!saved_decrypted_sample) {
			frame_t frame;

			decrypt(
				encrypted_frame_wrapper,
				unencrypted_frame_wrapper,
				init_data->key_pair.private_key,
				init_data->key_pair.rsa_modulus,
				complete_frame_size,
				0, 0
			);

			for (i = 0; i < init_data->frame_size.header; ++i)
				((unsigned char*)&frame.ih)[i] = unencrypted_frame_wrapper[i];

			frame.bitmap_data = malloc(init_data->frame_size.bitmap);

			for (; (unsigned)(i - init_data->frame_size.header) < init_data->frame_size.bitmap; ++i)
				((unsigned char*)frame.bitmap_data)[i - init_data->frame_size.header] = unencrypted_frame_wrapper[i];

			if (save_bmp(
				init_data->decrypted_frame_path,
				&frame.ih, frame.bitmap_data)) {
				log("decrypted frame sample not saved");
			}
			else {
				saved_decrypted_sample = 1;
				log("stored decrypted frame sample in: %s", init_data->decrypted_frame_path);
			}

			free(frame.bitmap_data);
		}

		unsigned short encrypted_data_was_pushed = 0;
		unsigned short full_queue = 0;

		do {
			encrypted_data_was_pushed = xQueueSend(
				init_data->output_queue,
				(void*)&encrypted_frame_wrapper,
				(TickType_t)0
			);

			if (encrypted_data_was_pushed) {
				log("data_encryption_task has pushed encrypted data into the transmission queue");
			}
			else if (!full_queue) {
				full_queue = 1;
				//log("the transmission queue is now full");
			}
		} while (!encrypted_data_was_pushed);

		//log("data_encryption_task took %lu ticks to execute", xTaskGetTickCount() - current_tick_count);
		log("FreeRTOS sent a frame to the TCP server");
	}

	free(encrypted_frame_wrapper);
	free(unencrypted_frame_wrapper);
}

void start_data_encryption_task(
	TaskHandle_t* task_handle,
	key_pair_t key_pair,
	QueueHandle_t input_queue,
	QueueHandle_t output_queue,
	frame_size_t frame_size,
	const char* unencrypted_frame_path,
	const char* decrypted_frame_path
) {
	log("executed start_data_encryption_task()");

	task_init_data.key_pair = key_pair;
	task_init_data.input_queue = input_queue;
	task_init_data.output_queue = output_queue;
	task_init_data.frame_size = frame_size;
	task_init_data.unencrypted_frame_path = unencrypted_frame_path;
	task_init_data.decrypted_frame_path = decrypted_frame_path;

	xTaskCreate(
		data_encryption_task,
		"data_encryption_task",
		configMINIMAL_STACK_SIZE,
		(void*)&task_init_data,
		1,
		task_handle
	);
}
