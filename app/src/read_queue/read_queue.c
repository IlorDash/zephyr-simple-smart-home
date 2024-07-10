#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>
#include <zephyr/pm/device.h>
#include <zephyr/pm/device_runtime.h>
#include <string.h>

#include "read_queue.h"

#define QUEUE_SIZE 64

#define STACKSIZE 1024
#define PRIORITY 7

LOG_MODULE_REGISTER(read_queue, CONFIG_SENSOR_LOG_LEVEL);
K_MSGQ_DEFINE(sensor_queue, sizeof(struct sensor_data), QUEUE_SIZE, 1);
K_THREAD_STACK_DEFINE(sens_stack, STACKSIZE);

struct k_thread sens_read_thread;
static struct device **sensors_arr;
static struct sensor_sampling *sampling_arr;

void swap(struct sensor_sampling *a, struct sensor_sampling *b) {
	struct sensor_sampling temp = *a;
	*a = *b;
	*b = temp;
}

int partition_time(struct sensor_sampling arr[], int low, int high) {
	int64_t pivot = arr[low].time;
	int i = low;
	int j = high;

	while (i < j) {
		while (arr[i].time <= pivot && i <= high - 1) {
			i++;
		}
		while (arr[j].time > pivot && j >= low + 1) {
			j--;
		}
		if (i < j) {
			swap(&arr[i], &arr[j]);
		}
	}
	swap(&arr[low], &arr[j]);
	return j;
}

void quick_sort_time(struct sensor_sampling arr[], int low, int high) {
	if (low < high) {
		int partitionIndex = partition_time(arr, low, high);
		quick_sort_time(arr, low, partitionIndex - 1);
		quick_sort_time(arr, partitionIndex + 1, high);
	}
}

int read_temp(const struct device *sensor, int id) {
	int ret;
	struct sensor_value val;
	struct sensor_data data;

	ret = sensor_sample_fetch(sensor);
	if (ret < 0) {
		LOG_ERR("Could not fetch sample (%d)", ret);
		return ret;
	}

	ret = sensor_channel_get(sensor, SENSOR_CHAN_PROX, &val);
	if (ret < 0) {
		LOG_ERR("Could not get sample (%d)", ret);
		return ret;
	}

	data.sens_id = id;
	data.temp = val.val1;

	ret = k_msgq_put(&sensor_queue, &data, K_NO_WAIT);
	if (ret < 0) {
		LOG_ERR("Failed to send data to queue.\n");
		return ret;
	}

	LOG_DBG("[%d ms] Sensor ID: %d, Temperature: %d\n", k_uptime_get_32(), id, data.temp);

	return 0;
}

void print_sampling(int num) {
	for (int i = 0; i < num; i++) {
		LOG_DBG("%d) ID: %d Period: %d Time: %d",
				i,
				sampling_arr[i].sens_id,
				sampling_arr[i].period_ms,
				sampling_arr[i].time);
	}
}

void insert_sampling() {
	int i = 1;
	struct sensor_sampling temp_sample;
	while (sampling_arr[0].time > sampling_arr[i].time) {
		i++;
	}
	temp_sample = sampling_arr[0];
	memmove(&sampling_arr[0], &sampling_arr[1], i - 1);
	sampling_arr[i - 1] = temp_sample;
}

void sensor_read_task(void *p1, void *p2, void *p3) {
	uint32_t time = 0;
	int sens_num = (int)p1;
	int ret;

	while (1) {
		k_sleep(K_MSEC(sampling_arr[0].time - time));
		time = k_uptime_get_32();

		ret = pm_device_runtime_get(sensors_arr[sampling_arr[0].sens_id]);
		if (ret < 0) {
			LOG_ERR("Failed to resume sensor %d\n", sampling_arr[0].sens_id);
		}

		read_temp(sensors_arr[sampling_arr[0].sens_id], sampling_arr[0].sens_id);
		sampling_arr[0].time = time + sampling_arr[0].period_ms;

		ret = pm_device_runtime_put(sensors_arr[sampling_arr[0].sens_id]);
		if (ret < 0) {
			LOG_ERR("Failed to suspend sensor %d\n", sampling_arr[0].sens_id);
		}

		insert_sampling();

		print_sampling(sens_num);
	}
}

struct k_msgq *
read_queue_init(struct device **_sensor, struct sensor_sampling *_sampling_arr, int sens_num) {

	int64_t time;

	sensors_arr = _sensor;
	sampling_arr = _sampling_arr;

	time = k_uptime_get();

	LOG_DBG("DEBUG LOG\n");

	for (int i = 0; i < sens_num; i++) {
		sampling_arr[i].time = time + sampling_arr[i].period_ms;
	}

	k_thread_create(&sens_read_thread,
					sens_stack,
					STACKSIZE,
					sensor_read_task,
					INT_TO_POINTER(sens_num),
					NULL,
					NULL,
					PRIORITY,
					0,
					K_NO_WAIT);

	return &sensor_queue;
}