#include <zephyr/kernel.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>

#include <app_version.h>

LOG_MODULE_REGISTER(main, CONFIG_APP_LOG_LEVEL);

int main(void)
{
	int ret;
	const struct device *sensor;
	struct sensor_value val;

	printk("Zephyr Temperature sensor Application %s\n", APP_VERSION_STRING);

	sensor = DEVICE_DT_GET(DT_NODELABEL(mock_sensor));
	if (!device_is_ready(sensor)) {
		LOG_ERR("Temperature sensor not ready");
		return 0;
	}

	printk("Use the sensor to change LED blinking period\n");

	while (1) {
		ret = sensor_sample_fetch(sensor);
		if (ret < 0) {
			LOG_ERR("Could not fetch sample (%d)", ret);
			return 0;
		}

		ret = sensor_channel_get(sensor, SENSOR_CHAN_PROX, &val);
		if (ret < 0) {
			LOG_ERR("Could not get sample (%d)", ret);
			return 0;
		}

		printk("Get sensor val %d\n", val.val1);


		k_sleep(K_MSEC(100));
	}

	return 0;
}

