const VEML6030 = require('bindings')('homebridge-veml6030');

function measure() {
  console.log(VEML6030.accessALS());
  console.log(VEML6030.accessWhite());
}

function sleep(ms) {
  return new Promise(resolve => setTimeout(resolve, ms));
}

async function test() {
  VEML6030.init();

  for (i = 0; i < 60; i++) {
    measure();
    await(sleep(1000));
  }

  VEML6030.deinit();
}

test();

