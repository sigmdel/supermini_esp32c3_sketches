# ESP32SPISlave

SPI Slave library for ESP32

> [!CAUTION]
> Breaking API changes from v0.4.0 and above

## ESP32DMASPI

This is the simple SPI slave library and does NOT use DMA. Please use [ESP32DMASPI](https://github.com/hideakitai/ESP32DMASPI) to transfer more than 32 bytes with DMA.

## Feature

- Support SPI Slave mode based on [ESP32's SPI Slave Driver](https://docs.espressif.com/projects/esp-idf/en/latest/api-reference/peripherals/spi_slave.html#spi-slave-driver)
- Slave has several ways to send/receive transactions
  - `transfer()` to send/receive transaction one by one (blocking)
  - `queue()` and `wait()` to send/receive multiple transactions at once and wait for them (blocking but more efficient than `transfer()` many times)
  - `queue()` and `trigger()` to send/receive multiple transactions at once in the background (non-blocking)
- Various configurations based on driver APIs
- Register user-defined ISR callbacks

### Supported ESP32 Version

| IDE         | ESP32 Board Version |
| ----------- | ------------------- |
| Arduino IDE | `>= 2.0.11`         |
| PlatformIO  | `>= 5.0.0`          |

## Notes for Communication Errors

If you have communication errors when trying examples, please check the following points.

- Check that the SPI Mode is the same
- Check if the pin number and connection destination are correct
- Connect pins as short as possible
- Be careful of signal line crosstalk (Be careful not to tangle wires)
- If you are using two devices, ensure they share a common ground level
- If you still have communication problems, try a lower frequency (1MHz or less)

## Usage

Please refer examples for more information.

### Blocking big `transfer()` one by one

```C++
#include <ESP32SPISlave.h>

ESP32SPISlave slave;

static constexpr size_t BUFFER_SIZE = 8;
static constexpr size_t QUEUE_SIZE = 1;
uint8_t tx_buf[BUFFER_SIZE] {1, 2, 3, 4, 5, 6, 7, 8};
uint8_t rx_buf[BUFFER_SIZE] {0, 0, 0, 0, 0, 0, 0, 0};

void setup()
{
    slave.setDataMode(SPI_MODE0);   // default: SPI_MODE0
    slave.setQueueSize(QUEUE_SIZE); // default: 1

    // begin() after setting
    slave.begin();  // default: HSPI (please refer README for pin assignments)
}

void loop()
{
    // do some initialization for tx_buf and rx_buf

    // start and wait to complete one BIG transaction (same data will be received from slave)
    const size_t received_bytes = slave.transfer(tx_buf, rx_buf, BUFFER_SIZE);

    // do something with received_bytes and rx_buf if needed
}
```

### Blocking multiple transactions

You can use Master and Slave almost the same way (omit the Slave example here).

```c++
void loop()
{
    // do some initialization for tx_buf and rx_buf

    // queue multiple transactions
    // in this example, the master sends some data first,
    slave.queue(NULL, rx_buf, BUFFER_SIZE);
    // and the slave sends same data after that
    slave.queue(tx_buf, NULL, BUFFER_SIZE);

    // wait for the completion of the queued transactions
    const std::vector<size_t> received_bytes = slave.wait();

    // do something with received_bytes and rx_buf if needed
}

```

### Non-blocking multiple transactions

You can use Master and Slave almost the same way (omit the Slave example here).

```c++
void loop()
{
    // if no transaction is in flight and all results are handled, queue new transactions
    if (slave.hasTransactionsCompletedAndAllResultsHandled()) {
        // do some initialization for tx_buf and rx_buf

        // queue multiple transactions
        // in this example, the master sends some data first,
        slave.queue(NULL, rx_buf, BUFFER_SIZE);
        // and the slave sends same data after that
        slave.queue(tx_buf, NULL, BUFFER_SIZE);

        // finally, we should trigger transaction in the background
        slave.trigger();
    }

    // you can do some other stuff here
    // NOTE: you can't touch dma_tx/rx_buf because it's in-flight in the background

    // if all transactions are completed and all results are ready, handle results
    if (slave.hasTransactionsCompletedAndAllResultsReady(QUEUE_SIZE)) {
        // get received bytes for all transactions
        const std::vector<size_t> received_bytes = slave.numBytesReceivedAll();

        // do something with received_bytes and rx_buf if needed
    }
}
```

## SPI Pins

The pins for SPI buses are automatically attached as follows. "Default SPI Pins" means the pins defined there are the same as `MOSI`, `MISO`, `SCK`, and `SS`.

| Board     | SPI       | MOSI | MISO | SCK | SS  | Default SPI Pins |
| --------- | --------- | ---- | ---- | --- | --- | ---------------- |
| `esp32`   | HSPI      | 13   | 12   | 14  | 15  | No               |
| `esp32`   | VSPI/FSPI | 23   | 19   | 18  | 5   | Yes              |
| `esp32s2` | FSPI/HSPI | 35   | 37   | 36  | 34  | Yes              |
| `esp32s3` | FSPI/HSPI | 11   | 13   | 12  | 10  | Yes              |
| `esp32c3` | FSPI      | 6    | 5    | 4   | 7   | Yes              |

Depending on your board, the default SPI pins are defined in `pins_arduino.h`. For example, `esp32`'s default SPI pins are found [here](https://github.com/espressif/arduino-esp32/blob/e1f14331f173a00a9062f616bc9a62c358b9076f/variants/esp32/pins_arduino.h#L20-L23) (`MOSI: 23, MISO: 19, SCK: 18, SS: 5`). Please refer to [arduino-esp32/variants](https://github.com/espressif/arduino-esp32/tree/e1f14331f173a00a9062f616bc9a62c358b9076f/variants) for your board's default SPI pins.

## SPI Buses

This library's `bool begin(const uint8_t spi_bus)` function uses `HSPI` as the default SPI bus for ESP32 as same as `SPI` library of `arduino-esp32` ([reference](https://github.com/espressif/arduino-esp32/blob/099b432d10fb4ca1529c52241bcadcb8a4386f17/libraries/SPI/src/SPI.h#L61)). But for other chips (ESP32-S2, ESP32-S3, ESP32-C3, etc.), `FSPI` is used as the default SPI bus.

The supported SPI buses are different from the ESP32 chip. Please note that there may be a restriction to use `FSPI` of ESP32 for your SPI bus. Please refer to the following table for the supported SPI buses.

| Chip     | FSPI                | HSPI                | VSPI                 |
| -------- | ------------------- | ------------------- | -------------------- |
| ESP32    | SPI1_HOST(`0`) [^1] | SPI2_HOST(`1`) [^2] | SPI3_HOST (`2`) [^3] |
| ESP32-S2 | SPI2_HOST(`1`)      | SPI3_HOST(`2`)      | -                    |
| ESP32-S3 | SPI2_HOST(`1`)      | SPI3_HOST(`2`)      | -                    |
| ESP32-C3 | SPI2_HOST(`1`)      | -                   | -                    |
| ESP32-C5 | SPI2_HOST(`1`)      | -                   | -                    |
| ESP32-C6 | SPI2_HOST(`1`)      | -                   | -                    |
| ESP32-H2 | SPI2_HOST(`1`)      | -                   | -                    |
| ESP32-P4 | SPI2_HOST(`1`)      | -                   | -                    |

[^1]: SPI bus attached to the flash (can use the same data lines but different SS)
[^2]: SPI bus normally mapped to pins 12 - 15 on ESP32 but can be matrixed to any pins
[^3]: SPI bus normally attached to pins 5, 18, 19, and 23 on ESP32 but can be matrixed to any pins

<details>
<summary>Reference</summary>

- <https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/spi_master.html>
- <https://github.com/espressif/arduino-esp32/blob/099b432d10fb4ca1529c52241bcadcb8a4386f17/cores/esp32/esp32-hal-spi.h#L28-L37>
- <https://github.com/espressif/arduino-esp32/blob/099b432d10fb4ca1529c52241bcadcb8a4386f17/libraries/SPI/src/SPI.cpp#L346-L350>
- <https://github.com/espressif/arduino-esp32/blob/099b432d10fb4ca1529c52241bcadcb8a4386f17/cores/esp32/esp32-hal-spi.h#L28-L37>
- <https://github.com/espressif/arduino-esp32/blob/099b432d10fb4ca1529c52241bcadcb8a4386f17/cores/esp32/esp32-hal-spi.c#L719-L752>
- <https://github.com/espressif/arduino-esp32/blob/099b432d10fb4ca1529c52241bcadcb8a4386f17/tools/sdk/esp32/include/driver/include/driver/sdspi_host.h#L23-L29>
- <https://github.com/espressif/arduino-esp32/blob/099b432d10fb4ca1529c52241bcadcb8a4386f17/tools/sdk/esp32/include/hal/include/hal/spi_types.h#L26-L31>
- <https://github.com/espressif/arduino-esp32/blob/099b432d10fb4ca1529c52241bcadcb8a4386f17/tools/sdk/esp32/include/hal/include/hal/spi_types.h#L77-L87>

</details>

## APIs

```C++
/// @brief initialize SPI with the default pin assignment for FSPI, HSPI or VSPI
/// @param spi_bus FSPI, HSPI or VSPI (HSPI is default for ESP32, FSPI is default for other chips)
bool begin(uint8_t spi_bus);
/// @brief initialize SPI with HSPI/FSPI/VSPI, sck, miso, mosi, and ss pins
bool begin(uint8_t spi_bus, int sck, int miso, int mosi, int ss);
/// @brief initialize SPI with HSPI/FSPI/VSPI and Qued SPI pins
bool begin(uint8_t spi_bus, int sck, int ss, int data0, int data1, int data2, int data3);
/// @brief initialize SPI with HSPI/FSPI/VSPI and Octo SPI pins
bool begin(uint8_t spi_bus, int sck, int ss, int data0, int data1, int data2, int data3, int data4, int data5, int data6, int data7);
/// @brief stop spi slave (terminate spi_slave_task and deinitialize spi)
void end();

/// @brief execute one transaction and wait for the completion
size_t transfer(const uint8_t* tx_buf, uint8_t* rx_buf, size_t size, uint32_t timeout_ms = 0);
size_t transfer(uint32_t flags, const uint8_t* tx_buf, uint8_t* rx_buf, size_t size, uint32_t timeout_ms);

/// @brief  queue transaction to internal transaction buffer.
///         To start transaction, wait() or trigger() must be called.
bool queue(const uint8_t* tx_buf, uint8_t* rx_buf, size_t size);
bool queue(uint32_t flags, const uint8_t* tx_buf, uint8_t* rx_buf, size_t size);

/// @brief execute queued transactions and wait for the completion.
///        rx_buf is automatically updated after the completion of each transaction.
std::vector<size_t> wait(uint32_t timeout_ms = 0);

/// @brief execute queued transactions asynchronously in the background (without blocking)
///        numBytesReceivedAll() or numBytesReceived() is required to confirm the results of transactions
///        rx_buf is automatically updated after the completion of each transaction.
bool trigger();

/// @brief return the number of in-flight transactions
size_t numTransactionsInFlight();
/// @brief return the number of completed but not received transaction results
size_t numTransactionsCompleted();
/// @brief return the oldest result of the completed transaction (received bytes)
size_t numBytesReceived();
/// @brief return all results of the completed transactions (received bytes)
std::vector<size_t> numBytesReceivedAll();
/// @brief check if the queued transactions are completed and all results are handled
bool hasTransactionsCompletedAndAllResultsHandled();
/// @brief check if the queued transactions are completed
bool hasTransactionsCompletedAndAllResultsReady(size_t num_queued);

// ===== Main Configurations =====
// set these optional parameters before begin() if you want

/// @brief set spi data mode
void setDataMode(uint8_t mode);
/// @brief set queue size (default: 1)
void setQueueSize(size_t size);

// ===== Optional Configurations =====
// set these optional parameters before begin() if you want

/// @brief set default data io level
void setDataIODefaultLevel(bool level);
/// @brief Bitwise OR of SPI_SLAVE_* flags.
void setSlaveFlags(uint32_t flags);
/// @brief SPI mode, representing a pair of (CPOL, CPHA) configuration: 0: (0, 0), 1: (0, 1), 2: (1, 0), 3: (1, 1)
void setSpiMode(uint8_t m);
/// @brief Callback called after the SPI registers are loaded with new data.
void setPostSetupCb(const slave_transaction_cb_t &post_setup_cb);
/// @brief Callback called after a transaction is done.
void setPostTransCb(const slave_transaction_cb_t &post_trans_cb);
/// @brief set post_setup callback (ISR) and its argument that are called after transaction setup completed.
///        you can call this function before every transfer() / queue() to change the behavior per transaction.
///        see more details about callbacks at https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/spi_master.html#_CPPv4N29spi_device_interface_config_t6pre_cbE
void setUserPostSetupCbAndArg(const spi_slave_user_cb_t &cb, void *arg);
void setUserPostTransCbAndArg(const spi_slave_user_cb_t &cb, void *arg);
```

## License

MIT
