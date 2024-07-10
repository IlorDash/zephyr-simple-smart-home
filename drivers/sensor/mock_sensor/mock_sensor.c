#define DT_DRV_COMPAT zephyr_mock_sensor

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/random/random.h>
#include <zephyr/logging/log.h>
#include <zephyr/pm/device.h>
#include <zephyr/pm/device_runtime.h>

#define SENSOR_ATTR_MOCK_SAMPLE_PERIOD SENSOR_ATTR_PRIV_START

LOG_MODULE_REGISTER(mock_sensor, CONFIG_SENSOR_LOG_LEVEL);

struct mock_sensor_data {
	int val;
};

struct mock_sensor_config {
	int sample_period_ms;
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
		LOG_ERR("channel %d not supported", chan);
		return -ENOTSUP;
	}

	val->val1 = data->val;

	return 0;
}
static int mock_sensor_attr_get(const struct device *dev,
								enum sensor_channel chan,
								enum sensor_attribute attr,
								struct sensor_value *val) {
	const struct mock_sensor_config *config = dev->config;

	if (chan != SENSOR_CHAN_PROX) {
		LOG_ERR("channel %d not supported", chan);
		return -ENOTSUP;
	}

	if (attr != SENSOR_ATTR_MOCK_SAMPLE_PERIOD) {
		LOG_ERR("attr %d not supported", attr);
		return -ENOTSUP;
	}

	return config->sample_period_ms;
}

static const struct sensor_driver_api mock_sensor_api = {
	.sample_fetch = &mock_sensor_sample_fetch,
	.channel_get = &mock_sensor_channel_get,
	.attr_get = &mock_sensor_attr_get,
};

static int mock_sensor_pm_action(const struct device *dev, enum pm_device_action action) {
	switch (action) {
		case PM_DEVICE_ACTION_RESUME:
			LOG_PRINTK("%s resuming..\n", dev->name);
			break;
		case PM_DEVICE_ACTION_SUSPEND:
			LOG_PRINTK("%s suspending..\n", dev->name);
			break;
		default: {
			LOG_ERR("action %d not supported", action);
			return -ENOTSUP;
		}
	}

	return 0;
}

static int mock_sensor_init(const struct device *dev) {

	int ret = 0;

	pm_device_init_suspended(dev);

	ret = pm_device_runtime_enable(dev);
	if ((ret < 0) && (ret != -ENOSYS)) {
		LOG_ERR("Failed to enable sensor %d", ret);
		return ret;
	}

	return ret;
}

#define MOCK_SENSOR_INIT(i)                                                                        \
	static struct mock_sensor_data mock_sensor_data_##i;                                           \
                                                                                                   \
	static const struct mock_sensor_config config_##i = {                                          \
		.sample_period_ms = DT_INST_PROP_OR(i, sample_period, 0U),                                 \
	};                                                                                             \
                                                                                                   \
	PM_DEVICE_DT_INST_DEFINE(i, mock_sensor_pm_action);                                            \
                                                                                                   \
	DEVICE_DT_INST_DEFINE(i,                                                                       \
						  mock_sensor_init,                                                        \
						  PM_DEVICE_DT_INST_GET(i),                                                \
						  &mock_sensor_data_##i,                                                   \
						  &config_##i,                                                             \
						  POST_KERNEL,                                                             \
						  CONFIG_SENSOR_INIT_PRIORITY,                                             \
						  &mock_sensor_api);

DT_INST_FOREACH_STATUS_OKAY(MOCK_SENSOR_INIT)
