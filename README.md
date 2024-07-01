# Zephyr Simple Smart Home Application

Simple Smart Home Application that reads temperature from mock temp sensor and printing via serial.

### Building and running

To build the application, run the following command:

```shell
west build -b $BOARD app
```

where `$BOARD` is the target board.

There is overlay for the `qemu_xtensa` board, so you can use this board.