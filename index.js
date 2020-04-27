const VEML6030 = require('bindings')('homebridge-veml6030');

const moment = require('moment'); // Time formatting
const mqtt = require('mqtt'); // MQTT client
const os = require('os'); // Hostname

var Service, Characteristic;
var CustomCharacteristic;
var FakeGatoHistoryService;

module.exports = (homebridge) => {
  Service = homebridge.hap.Service;
  Characteristic = homebridge.hap.Characteristic;
  CustomCharacteristic = require('./src/js/customcharacteristic.js')(homebridge);
  FakeGatoHistoryService = require('fakegato-history')(homebridge);

  homebridge.registerAccessory("homebridge-veml6030", "VEML6030", VEML6030Accessory);
}

function VEML6030Accessory(log, config) {
  // Load configuration from files
  this.log = log;
  this.displayName = config['name'];
  this.i2cAdaptor = config['i2cAdaptor'] || '/dev/i2c-3';
  this.i2cAddress = config['i2cAddress'] ? config['i2cAddress'] : 0x48;
  this.enableFakeGato = config['enableFakeGato'] || false;
  this.useWhite = config['useWhite'] || false;
  this.fakeGatoStoragePath = config['fakeGatoStoragePath'];
  this.enableMQTT = config['enableMQTT'] || false;
  this.mqttConfig = config['mqtt'];

  // Internal variables to keep track of current ALS and white lux measurements 
  this._als = null;
  this._white = null;
  this._alsSamples = [];
  this._whiteSamples = [];
  this._alsCumSum = 0;
  this._whiteCumSum = 0;
  this._alsCounter = 0;
  this._whiteCounter = 0;

  // Services
  let informationService = new Service.AccessoryInformation();
  informationService
    .setCharacteristic(Characteristic.Manufacturer, "Vishay")
    .setCharacteristic(Characteristic.Model, "VEML6030")
    .setCharacteristic(Characteristic.SerialNumber, `${os.hostname}-${this.i2cAdaptor.split('/').pop()}`)
    .setCharacteristic(Characteristic.FirmwareRevision, require('./package.json').version);

  let alsService = new Service.LightSensor("ALS", "als");
  if (this.enableFakeGato && !this.useWhite) {
    alsService.addCharacteristic(CustomCharacteristic.AtmosphericPressureLevel);
  }

  let whiteService = new Service.LightSensor("White", "white");
  if (this.enableFakeGato && this.useWhite) {
    whiteService.addCharacteristic(CustomCharacteristic.AtmosphericPressureLevel);
  }

  this.informationService = informationService;
  this.alsService = alsService;
  this.whiteService = whiteService;

  // Start FakeGato for logging historical data
  if (this.enableFakeGato) {
    this.fakeGatoHistoryService = new FakeGatoHistoryService("weather", this, {
      storage: 'fs',
      folder: this.fakeGatoStoragePath
    });
  }

  // Set up MQTT client
  if (this.enableMQTT) {
    this.setUpMQTT();
  }

  // Periodically update the values
  this.setupVEML6030();
  this.refreshData();
  setInterval(() => this.refreshData(), 1000);
}

// Error checking and averaging when saving als and white
Object.defineProperty(VEML6030Accessory.prototype, "als", {
  set: function(alsReading) {
    // Calculate running average of als over the last 30 samples
    this._alsCounter++;
    if (this._alsSamples.length == 30) {
      let firstSample = this._alsSamples.shift();
      this._alsCumSum -= firstSample;
    }
    this._alsSamples.push(alsReading);
    this._alsCumSum += alsReading;

    // Update current als value, and publish to MQTT/FakeGato once every 5 seconds
    if (this._alsCounter == 5) {
      this._alsCounter = 0;
      this._currentALS = this._alsCumSum / Math.min(this._whiteSamples.length, 30); 
      this.log.debug(`ALS Reading: ${this._currentALS}`);

      this.alsService.getCharacteristic(Characteristic.CurrentAmbientLightLevel)
        .updateValue(this._currentALS);
 
      if (this.enableFakeGato && !this.useWhite) {
        this.fakeGatoHistoryService.addEntry({
          time: moment().unix(),
          pressure: this._currentALS,
        });
      
        this.alsService.getCharacteristic(CustomCharacteristic.AtmosphericPressureLevel)
         .updateValue(this._currentALS);
      }
 
      if (this.enableMQTT) {
        this.publishToMQTT(this.alsTopic, this._currentALS);
      }
    }
  },

  get: function() {
    return this._currentALS;
  }
});

Object.defineProperty(VEML6030Accessory.prototype, "white", {
  set: function(whiteReading) {
    // Calculate running average of white over the last 30 samples
    this._whiteCounter++;
    if (this._whiteSamples.length == 30) {
      let firstSample = this._whiteSamples.shift();
      this._whiteCumSum -= firstSample;
    }
    this._whiteSamples.push(whiteReading);
    this._whiteCumSum += whiteReading;

    if (this._whiteCounter == 5) {
      this._whiteCounter = 0;
      this._currentWhite = this._whiteCumSum / Math.min(this._whiteSamples.length, 30);
      this.log.debug(`White Reading: ${this._currentWhite}`);

      this.whiteService.getCharacteristic(Characteristic.CurrentAmbientLightLevel)
       .updateValue(this._currentWhite);

      if (this.enableFakeGato && this.useWhite) {
        this.fakeGatoHistoryService.addEntry({
          time: moment().unix(),
          pressure: this._currentWhite,
        });
        this.whiteService.getCharacteristic(CustomCharacteristic.AtmosphericPressureLevel)
         .updateValue(this._currentWhite);
      }

      if (this.enableMQTT) {
        this.publishToMQTT(this.whiteTopic, this._currentWhite);
      }
    }
  },

  get: function() {
    return this._currentWhite;
  }
});

// Sets up MQTT client based on config loaded in constructor
VEML6030Accessory.prototype.setUpMQTT = function() {
  if (!this.enableMQTT) {
    this.log.info("MQTT not enabled");
    return;
  }

  if (!this.mqttConfig) {
    this.log.error("No MQTT config found");
    return;
  }

  this.mqttUrl = this.mqttConfig.url;
  this.alsTopic = this.mqttConfig.alsTopic || 'VEML6030/als';
  this.whiteTopic = this.mqttConfig.whiteTopic || 'VEML6030/white';

  this.mqttClient = mqtt.connect(this.mqttUrl);
  this.mqttClient.on("connect", () => {
    this.log(`MQTT client connected to ${this.mqttUrl}`);
  });
  this.mqttClient.on("error", (err) => {
    this.log(`MQTT client error: ${err}`);
    client.end();
  });
}

// Sends data to MQTT broker; must have called setupMQTT() previously
VEML6030Accessory.prototype.publishToMQTT = function(topic, value) {
  if (!this.mqttClient.connected || !topic) {
    this.log.error("MQTT client not connected, or no topic or value for MQTT");
    return;
  }
  this.mqttClient.publish(topic, String(value));
}

// Set up sensor; checks that I2C interface is available and device is ready
VEML6030Accessory.prototype.setupVEML6030 = function() {
  data = VEML6030.init(this.i2cAdaptor);
  if (data.hasOwnProperty('errcode')) {
    this.log(`Error: ${data.errmsg}`);
  }
}

// Read ALS and white lux from sensor
VEML6030Accessory.prototype.refreshData = function() {
  let data;
  data = VEML6030.accessALS();

  if (data.hasOwnProperty('errcode')) {
    this.log(`Error: ${data.errmsg}`);
    // Updating a value with Error class sets status in HomeKit to 'Not responding'
    this.alsService.getCharacteristic(Characteristic.CurrentAmbientLightLevel)
      .updateValue(Error(data.errmsg));
  }
  this.als = data.als;
  this.log.debug(`Read ALS: ${data.als} lux`); 

  data = VEML6030.accessWhite();

  if (data.hasOwnProperty('errcode')) {
    this.log(`Error: ${data.errmsg}`);
    this.whiteService.getCharacteristic(Characteristic.CurrentAmbientLightLevel)
      .updateValue(Error(data.errmsg));
  }
  this.white = data.white;
  this.log.debug(`Read white: ${data.white} lux`); 
}

VEML6030Accessory.prototype.getServices = function() {
  return [this.informationService,
          this.alsService,
          this.whiteService,
          this.fakeGatoHistoryService];
}

