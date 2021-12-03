# DataVault - a simple, cheap, and effective backup unit for cloud storage services

## preface
Nowadays, we all, more or less, store our data on some *cloud storage service*. Some people (like me), in addition to using [...]

## the device
The device is rather simple: just an old Raspberry Pi model B+, a SATA SSD drive, an USB/SATA adapter, and an ESP8266 MCU board (NodeMCU).

![](docs/photos/inner_components.png "inner components")

## working principle
Everything is powered by an external 12V DC adapter. In the box there is a DC stepdown regulator for +5V line. These two voltage levels are connected on the "power circuit" board.

The ESP8266 unit is always powered (consuming almost nothing), it's connected to home network via wifi, and it acts as a "timer" for the Raspberry Pi unit. Once a day (in my case during the night), it switches a relay giving power to the Raspberry Pi, that is connected to the network via ethernet cable. Once the RaspberryPi system it's booted, it starts a simple python application that sequentially runs some tasks defined in a json configuration file. Each task spawns an **RClone** (https://rclone.org/) subprocess that connects to the remote cloud repository and *syncs* (in one way mode) all remote files to local SATA SSD drive.

## the ESP8266 unit
Firmware and webservice sources are under **sources/esp8266** folder. The code was written using **PlatformIO** (https://platformio.org/) on VSCode.
Unlike *almost* every ESP8266 firmwares you can find on the network, to connect this device to the WiFi network you don't need to write your network credentials in the sources, since I've added an handy **WPS** feature. You just need to enable WPS on your router, and push the WPS button on the ESP8266 unit (connected to pin D6). Once successfully paired, the device will connect to NTP server, taking current date/time. I've also added a **mDNS** feature, so you can reach its settings page opening the address **http://dvctrl.local/** with a modern browser from your local network.

![](docs/photos/esp8266_web_interface.jpg "esp8266 web interface")

From the web interface you can see date/time from the device, the power status of the SBC unit (the RaspberryPi), you can turn it on or off, and set/enable/disable the daily activation time.

This simple webpage is built with:
 - **Vue.js** v2 (https://vuejs.org/)
 - **Chota** micro CSS framework (https://github.com/jenil/chota)
 - **notyf** for toast notifications (https://carlosroso.com/notyf/)
 - **reconnectingWs** for websocket communication

## the RaspberryPi unit


