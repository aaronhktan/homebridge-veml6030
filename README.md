# Homebridge Plugin for VEML6030

This is a Homebridge plugin for VEML6030 light sensor, working on the Raspberry Pi 3.

It uses the Linux SMBus APIs.

<img src="/docs/eve.png?raw=true" style="margin: 5px"> <img src="/docs/home.png?raw=true" style="margin: 5px">

## Configuration

| Field name           | Description                                                   | Type / Unit    | Default value       | Required? |
| -------------------- |:--------------------------------------------------------------|:--------------:|:-------------------:|:---------:|
| name                 | Name of the accessory                                         | string         | —                   | Y         |
| i2cAdaptor           | I2C adaptor name, as listed under `/dev/`                     | int            | /dev/i2c-3          | N         |
| i2cAddress           | VEML6030 I2C address                                          | int (hex)      | 0x48                | N         |
| useWhite             | Control whether to show history for White or Ambient light    | bool           | false               | N         |
| enableFakeGato       | Enable storing data in Eve Home app                           | bool           | false               | N         |
| fakeGatoStoragePath  | Path to store data for Eve Home app                           | string         | (fakeGato default)  | N         |
| enableMQTT           | Enable sending data to MQTT server                            | bool           | false               | N         |
| mqttConfig           | Object containing some config for MQTT                        | object         | —                   | N         |

If `enableMQTT` is true, Eve will also show a barometric pressure for one of White or Ambient Light. The conversion is 1hPa = 1 lux. If useWhite is `true`, history for white light levels is kept; if it is false, then history for ambient light is kept.

The mqttConfig object is **only required if enableMQTT is true**, and is defined as follows:

| Field name           | Description                                      | Type / Unit  | Default value       | Required? |
| -------------------- |:-------------------------------------------------|:------------:|:-------------------:|:---------:|
| url                  | URL of the MQTT server, must start with mqtt://  | string       | —                   | Y         |
| alsTopic             | MQTT topic to which ALS lux data is sent         | string       | VEML6030/als        | N         |
| whiteTopic           | MQTT topic to which White lux data is sent       | string       | VEML6030/white      | N         |

### Example Configuration

```
{
  "bridge": {
    "name": "Homebridge",
    "username": "XXXX",
    "port": XXXX
  },

  "accessories": [
    {
      "accessory": "VEML6030",
      "name": "VEML6030",
      "i2cAdaptor": "/dev/i2c-3",
      "enableFakeGato": true,
      "useWhite": true,
      "enableMQTT": true,
      "mqtt": {
          "url": "mqtt://192.168.0.38",
          "alsTopic": "veml6030/als",
          "whiteTopic": "veml6030/white"
      }
    },
  ]
}
```

## Project Layout

- All things required by Node are located at the root of the repository (i.e. package.json and index.js).
- The rest of the code is in `src`, further split up by language.
  - `c` contains the C code that runs on the device to communicate with the sensor. It also contains a simple C program to communicate with the sensor.
  - `binding` contains the C++ code using node-addon-api to communicate between C and the Node.js runtime.
  - `js` contains a custom characteristic to enable logging lux values as barometric pressure sensor.
