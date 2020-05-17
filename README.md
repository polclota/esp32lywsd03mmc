Basic Arduino C ESP32 code for Mijia LYWSD03MMC cheap BLE temperature and humidity sensors to MQTT. Self device configuration for Home assitant so that you don't need to configure them manually. 

It basically loops over all the sensors you configure in include/userconfig.h. Here is where your are to change the parameters to fit your needs.

After each and every loop step the device goes into deep sleep for what ever the time period defined and after waking up again it requests next sensor. That helps save power but it also helps clean memory as I didn't find a reliable way to move from one sensor to another without crashing after 2 or 3 attempts.

It's developed using platformio. Two different esp32 boards are configured so if you use a diferent board you must make consequent changes in platformio.ini file.

About platformio.ini.
Two very important considerations:

board_build.partitions = partitions_custom2.csv

Three partition table files are provided, the default one will not allow code to fit as BLE, MQTT and Wifi combined all together take too much memory. So you may play with the other two alternatives. I used custom2 and it worked fine.

build_flags =
  -D MQTT_MAX_PACKET_SIZE=600

Play with this flag at will. Adjust it if your json config files are bigger or smaller. By default, pubsubclient uses 128 which is never enough for large json MQTT configuration messages.

Every time the device turns on or off a message is submitted to a status topic, helping you keep track of the results remotely via MQTT without the need to check the serial console and keeping in mind your device might be sleeping most of the time.

I suggest using http://mqtt-explorer.com/ to track all messages.

Enjoy!"# esp32lywsd03mmc" 
