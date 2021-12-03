#ifndef DEFINES_H
#define DEFINES_H

// ----------------------------------------------------------------------------

#define DEVICE_STATUS_LED_PIN   13  // D7
#define RPI_STATUS_LED_PIN      16  // D0

#define WPS_BUTTON_PIN          14  // D5
#define POWER_BUTTON_PIN        12  // D6  

// #define MOSFET_CONTROL_PIN       2  // D4
#define MOSFET_CONTROL_PIN      15  // D8

#define SW_SERIAL_RX_PIN         5  // D1
#define SW_SERIAL_TX_PIN         4  // D2

// ----------------------------------------------------------------------------

#define HW_SERIAL_BAUDRATE         115200
#define SW_SERIAL_BAUDRATE         9600

// ----------------------------------------------------------------------------

#define EEPROM_DATA_START_ADDR	8

// ----------------------------------------------------------------------------

// #define DEVICE_STATUS_LED_MODE_NOT_CONNECTED 0
// #define DEVICE_STATUS_LED_MODE_TRYING_WPS    1
// #define DEVICE_STATUS_LED_MODE_CONNECTING    2
// #define DEVICE_STATUS_LED_MODE_CONNECTED     3

#define DEVICE_MDNS_HOSTNAME    "dvctrl"

// ----------------------------------------------------------------------------

#define NTP_SERVER_POOL         "europe.pool.ntp.org"
// #define NTP_TIMEOUT         1500
// #define NTP_PRINT_TIME_INTERVAL   1000
// #define NTP_SERVER_POOL         "europe.pool.ntp.org"
#define NTP_UTC_OFFSET_SECS        3600
// #define NTP_UPDATE_INTERVAL     60000

// ----------------------------------------------------------------------------

#define WEBSERVICE_PORT             80
#define WEBSERVICE_TIMEOUT_MSEC     2000

#define WEBSOCKET_SERVER_PORT       81

// ----------------------------------------------------------------------------

#define SBC_POWERED_BY_NONE         0
#define SBC_POWERED_BY_TIMER        1
#define SBC_POWERED_BY_USER         2

// ----------------------------------------------------------------------------

#endif