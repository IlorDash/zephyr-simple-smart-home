# Zephyr Simple Smart Home Application

Simple Smart Home Application is an example of the Zephyr Application. It reads the temperature from the mock sensors, which are defined in the DTS. The application supports power management and can handle up to 256 mock sensors. All the sensors' measurements are stored in a message queue, which can be retrieved later and displayed in the Logs (aka UART). The scheduling of sensor readings is done in the `read_queue/read_queue.c:sensor_read_task()`. This function launches the reading based on the sensor's `sample-period` property in the DTS. It also has a `samplings` queue, which stores the sensors' periods and sampling times and keeps them sorted by the second parameter, so zero element should always be read first.

### Building and running

To build the application, run the following command:

```shell
west build -b $BOARD app
```

where `$BOARD` is the target board.

There is overlay for the `qemu_xtensa` board, so you can use this board.
