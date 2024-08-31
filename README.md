
# Improve your productivity with the ZPHS01B air quality module. Assess your local air quality and take actions to improve your wellness.

Measure the concentrations of the harmful gases and particles in the air to approximately estimate air pollution and take actions for better health-dependent productivity.

ZPHS01B module has next sensors: pm1.0 pm2.5 pm10 particles laser sensor, infrared carbon dioxide (CO2) sensor, electrochemical formaldehyde (CH2O) sensor, electrochemical ozone (O3) sensor, electrochemical carbon monoxide (CO) sensor, VOC sensor, nitrogen dioxide (NO2) sensor, temperature and relative humidity sensor.

This project combines the module, a smartphone, and an ESP32 module. ESP32 reads raw values from the sensor module, processes them, and sends them to a smartphone via Bluetooth.

The documentation and specifications for the sensor module can be found here: https://www.winsen-sensor.com/product/zphs01b.html

The project was manually tested with ESP-WROOM-32 and POCO X6 Pro 5G.

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
`pm1.0 5 ug/m3, pm2.5 8 ug/m3, pm10 8 ug/m3, CO2 683 ppm, TVOC 0 lvl, CH2O 27 ug/m3, CO 0.5 ppm, O3 0.02 ppm, NO2 10 ppb, 26.3 *C, 72 RH;`

## Troubleshooting

Double check connection of wires to ESP32 module, between ESP32 module and sensor module, check settings in menuconfig for bluetooth (SSP off, BLE off, classic bluetooth and SPP are on, Bluetooth controller mode is BR/EDR Only) and for UART (9600 baudrate).
