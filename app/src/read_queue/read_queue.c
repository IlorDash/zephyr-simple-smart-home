#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>
#include <zephyr/pm/device.h>
#include <zephyr/pm/device_runtime.h>
#include <string.h>

#include "read_queue.h"

#define QUEUE_SIZE 64

#define STACKSIZE 1024
#define PRIORITY 7

LOG_MODULE_REGISTER(read_queue, CONFIG_APP_LOG_LEVEL);
K_MSGQ_DEFINE(sensor_queue, sizeof(struct sensor_data), QUEUE_SIZE, 1);
K_THREAD_STACK_DEFINE(sens_stack, STACKSIZE);

struct k_thread sens_read_thread;
static struct device **sensors_arr;
static struct sensor_sampling *sampling_arr;

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

void insert_sampling(int sens_num) {
	int i = 1;
	struct sensor_sampling temp_sample;
	while ((sampling_arr[0].time > sampling_arr[i].time) && (i < sens_num)) {
		i++;
	}
	temp_sample = sampling_arr[0];
	memmove(&sampling_arr[0], &sampling_arr[1], sizeof(struct sensor_sampling) * (i - 1));
	sampling_arr[i - 1] = temp_sample;
}

void print_sampling(int sens_num) {
	for (int i = 0; i < sens_num; i++) {
		LOG_DBG("%d) Period: %d Time: %d", sampling_arr[i].sens_id, sampling_arr[i].period_ms, sampling_arr[i].time);
	}
}

void sensor_read_task(void *p1, void *p2, void *p3) {
	uint32_t time = 0;
	int sens_num = (int)p1;
	int sleep_time, ret;

	while (1) {
		sleep_time = sampling_arr[0].time - time;
		sleep_time = (sleep_time < 0) ? 0 : sleep_time;
		k_sleep(K_MSEC(sleep_time));
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

		insert_sampling(sens_num);
		print_sampling(sens_num);
	}
}

struct k_msgq *read_queue_init(struct device **_sensor, struct sensor_sampling *_sampling_arr, int sens_num) {

	int64_t time;

	sensors_arr = _sensor;
	sampling_arr = _sampling_arr;

	time = k_uptime_get();

	for (int i = 0; i < sens_num; i++) {
		sampling_arr[i].time = time + sampling_arr[i].period_ms;
	}

	k_thread_create(&sens_read_thread, sens_stack, STACKSIZE, sensor_read_task, INT_TO_POINTER(sens_num), NULL, NULL, PRIORITY, 0, K_NO_WAIT);

	return &sensor_queue;
}