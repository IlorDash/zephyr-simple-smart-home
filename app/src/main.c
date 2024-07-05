#include <zephyr/kernel.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>

#include <app_version.h>

LOG_MODULE_REGISTER(main, CONFIG_APP_LOG_LEVEL);

#define FREQ_MHZ_TO_MS(f) (int)(1e6 / f)

#define MOCK_SENSOR_ALIAS(i) DT_ALIAS(_CONCAT(mock, i))
#define MOCK_SENSOR_DEVICE(i, _)                                                                   \
	IF_ENABLED(DT_NODE_EXISTS(MOCK_SENSOR_ALIAS(i)), (DEVICE_DT_GET(MOCK_SENSOR_ALIAS(i)), ))

static const struct device *const sensors[] = {LISTIFY(256, MOCK_SENSOR_DEVICE, ())};

int main(void) {
	int sample_delays_ms[ARRAY_SIZE(sensors)];
	int sample_freq_mhz;
	struct sensor_value val;

	printk("Zephyr Temperature sensor Application %s\n", APP_VERSION_STRING);

	for (int i = 0; i < ARRAY_SIZE(sensors); i++) {
		if (!device_is_ready(sensors[i])) {
			LOG_ERR("Sensor %d not ready", i);
			return 0;
		}
		printk("Sensor %d is ready\n", i);
		sample_freq_mhz = sensor_attr_get(sensors[i],
										  SENSOR_CHAN_PROX,
										  (enum sensor_attribute)SENSOR_ATTR_SAMPLING_FREQUENCY,
										  &val);
		sample_delays_ms[i] = FREQ_MHZ_TO_MS(sample_freq_mhz);
		printk("sample_delays_ms[%d]: %d\n", i, sample_delays_ms[i]);
	}

	return 0;
}
