#include <zephyr/kernel.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>

#include <app_version.h>
#include "read_queue/read_queue.h"

LOG_MODULE_REGISTER(main, CONFIG_APP_LOG_LEVEL);

#define STACKSIZE 1024
#define PRIORITY 7

#define MOCK_SENSOR_ALIAS(i) DT_ALIAS(_CONCAT(mock, i))
#define MOCK_SENSOR_DEVICE(i, _)                                                                   \
	IF_ENABLED(DT_NODE_EXISTS(MOCK_SENSOR_ALIAS(i)), (DEVICE_DT_GET(MOCK_SENSOR_ALIAS(i)), ))

const struct device *const sensors[] = {LISTIFY(256, MOCK_SENSOR_DEVICE, ())};

K_THREAD_STACK_DEFINE(processing_stack, STACKSIZE);

struct k_msgq *sensQueue;

void processing_task(void *unused1, void *unused2, void *unused3) {
	struct sensor_data data;

	while (1) {
		if (k_msgq_get(sensQueue, &data, K_NO_WAIT) == 0) {
			printk(
				"[%d ms] Sensor ID: %d, Temperature: %d\n", k_uptime_get_32(), data.sens_id, data.temp);
		}
		k_yield();
	}
}

int partition_period(struct sensor_sampling arr[], int low, int high) {
	uint16_t pivot = arr[low].period_ms;
	int i = low;
	int j = high;

	while (i < j) {
		while (arr[i].period_ms <= pivot && i <= high - 1) {
			i++;
		}
		while (arr[j].period_ms > pivot && j >= low + 1) {
			j--;
		}
		if (i < j) {
			swap(&arr[i], &arr[j]);
		}
	}
	swap(&arr[low], &arr[j]);
	return j;
}

void quick_sort_period(struct sensor_sampling arr[], int low, int high) {
	if (low < high) {
		int partition_index = partition_period(arr, low, high);
		quick_sort_period(arr, low, partition_index - 1);
		quick_sort_period(arr, partition_index + 1, high);
	}
}

int main(void) {
	struct sensor_sampling sampling[ARRAY_SIZE(sensors)];
	struct k_thread processing_thread;
	int sample_period_ms;
	struct sensor_value val;

	printk("Zephyr Temperature sensor Application %s\n", APP_VERSION_STRING);

	for (int i = 0; i < ARRAY_SIZE(sensors); i++) {
		if (!device_is_ready(sensors[i])) {
			LOG_ERR("Sensor %d not ready", i);
			return 0;
		}
		printk("Sensor %d is ready\n", i);
		sample_period_ms = sensor_attr_get(sensors[i],
										  SENSOR_CHAN_PROX,
										  (enum sensor_attribute)SENSOR_ATTR_SAMPLING_FREQUENCY,
										  &val);
		sampling[i].sens_id = i;
		sampling[i].period_ms = sample_period_ms;
		printk("sample_delays_ms[%d]: %d\n", i, sampling[i].period_ms);
	}

	quick_sort_period(sampling, 0, ARRAY_SIZE(sampling) - 1);

	sensQueue = read_queue_init(sensors, sampling, ARRAY_SIZE(sensors));

	k_thread_create(&processing_thread,
					processing_stack,
					STACKSIZE,
					processing_task,
					NULL,
					NULL,
					NULL,
					PRIORITY,
					0,
					K_NO_WAIT);

	while (1) {
		k_sleep(K_SECONDS(2));
	}

	return 0;
}
