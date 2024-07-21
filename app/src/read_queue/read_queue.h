#pragma once

#include <zephyr/kernel.h>

struct sensor_data {
	uint8_t sens_id;
	int temp;
};

struct sensor_sampling {
	uint8_t sens_id;
	uint16_t period_ms;
	uint32_t time;
} __attribute__((__packed__));

struct k_msgq *
read_queue_init(struct device **_sensor, struct sensor_sampling *_sampling_arr, int sens_num);
