
# ZPHS01B air quality module example with UART and Bluetooth. 

Manually tested on ESP-WROOM-32 and POCO X6 Pro 5G.

## How to use example

### Hardware Required

The example can be run on any having bluetooth ESP32, ESP32-S and ESP32-C series based development board connected to a computer with a single USB cable for flashing. 
Connection to computer can be replaced to connection to powerbank to add portability for DIY device. 
Main idea of this example is to collect dataabout air pollution with help of ZPHS01B sensor and send it 
to Android device with the app "Serial Bluetooth Terminal" for further use and processing. 

### Setup the Hardware

Connect the ZPHS01B serial interface to the ESP32 board as follows.

```
  ---------------------------------------------------------------------
  | ZPHS01B Interface     | Kconfig Option     | Default ESP Pin      |
  | ----------------------|--------------------|----------------------|
  | Voltage (Vc)          | n/a                | 5V                   |
  | Receive Data (RxD)    | EXAMPLE_UART_TXD   | GPIO4                |
  | Transmit Data (TxD)   | EXAMPLE_UART_RXD   | GPIO5                |
  | Ground                | n/a                | GND                  |
  ---------------------------------------------------------------------
```
Note: Some GPIOs can not be used with certain chips because they are reserved for internal use. Please refer to UART documentation for selected target.

### Configure the project

Use the command below to configure project using Kconfig menu as showed in the table above.
The default Kconfig values can be changed such as: EXAMPLE_TASK_STACK_SIZE, EXAMPLE_UART_BAUD_RATE, EXAMPLE_UART_PORT_NUM (Refer to Kconfig file).
```
idf.py menuconfig
```
For additional details of configuration you can check next examples in ESP-IDF v5.0.7:
`uart_echo_example`, `bt_spp_acceptor`.

For more security and to make data private you should switch off SSP in menuconfig of project before build and flash Components -> Bluetooth -> Bluedroid. It will make mandatory PIN check before first connection to ESP32 from external device via bluetooth. The PIN can be set in code up to 16 digits in length.

### Build and Flash

Build the project and flash it to the board, then run monitor tool to view serial output:

```
idf.py -p PORT flash monitor
```
This will show you logs with sensor data and bluetooth information of connected device in case if all is ok and is done right.

(To exit the serial monitor, type ``Ctrl-]``.)


## Example Output

There is an example of a string with air pollution contamination as measured by ZPHS01B air: 
`pm1.0 7 ug/m3, pm2.5 9 ug/m3, pm10 9 ug/m3, CO2 873 ppm, TVOC 0 lvl, CH2O 0.400 ug/m3, CO 0.5 ppm, O3 0.02 ppm, NO2 10 ppb, 27.3 *C, 79 RH;`

## Troubleshooting

Double check connection of wires to ESP32 module, between ESP32 module and sensor module, check settings in menuconfig for bluetooth (SSP off, BLE off, classic bluetooth and SPP are on, Bluetooth controller mode is BR/EDR Only) and for UART (9600 baudrate).
