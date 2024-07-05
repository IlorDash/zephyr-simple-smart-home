#define DT_DRV_COMPAT zephyr_mock_sensor

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/random/random.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(mock_sensor, CONFIG_SENSOR_LOG_LEVEL);

struct mock_sensor_data {
	int val;
};

static int mock_sensor_sample_fetch(const struct device *dev, enum sensor_channel chan) {
	struct mock_sensor_data *data = dev->data;

	data->val = sys_rand32_get() % 10 + 20;

	return 0;
}

static int mock_sensor_channel_get(const struct device *dev,
								   enum sensor_channel chan,
								   struct sensor_value *val) {
	struct mock_sensor_data *data = dev->data;

	if (chan != SENSOR_CHAN_PROX) {
		return -ENOTSUP;
	}

	val->val1 = data->val;

	return 0;
}

static const struct sensor_driver_api mock_sensor_api = {
	.sample_fetch = &mock_sensor_sample_fetch,
	.channel_get = &mock_sensor_channel_get,
};

static int mock_sensor_init(const struct device *dev) {
	return 0;
}

#define MOCK_SENSOR_INIT(i)                                                                        \
	static struct mock_sensor_data mock_sensor_data_##i;                                           \
                                                                                                   \
	DEVICE_DT_INST_DEFINE(i,                                                                       \
						  mock_sensor_init,                                                        \
						  NULL,                                                                    \
						  &mock_sensor_data_##i,                                                   \
						  NULL,                                                                    \
						  POST_KERNEL,                                                             \
						  CONFIG_SENSOR_INIT_PRIORITY,                                             \
						  &mock_sensor_api);

DT_INST_FOREACH_STATUS_OKAY(MOCK_SENSOR_INIT)
