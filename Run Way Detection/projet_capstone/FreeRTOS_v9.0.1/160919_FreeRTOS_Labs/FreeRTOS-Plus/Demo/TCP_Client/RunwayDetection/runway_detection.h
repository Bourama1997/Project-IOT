#ifndef RUNWAY_DETECTION_H
#define RUNWAY_DETECTION_H

void start_runway_detection(
	const char* sample_frame_path,
	const char* sample_filtered_frame_path,
	const char* unencrypted_frame_path,
	const char* decrypted_frame_path,
	unsigned long rsa_prime_p,
	unsigned long rsa_prime_q,
	const char* demo_message
);

void vApplicationTickHook();

#endif // !RUNWAY_DETECTION_H
