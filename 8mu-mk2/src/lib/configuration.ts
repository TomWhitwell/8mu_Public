import { logger } from "$lib/logger";
import type { Button, Control, ControllerConfiguration } from "$lib/types";

export const isEquivalent = (
  configA: ControllerConfiguration,
  configB: ControllerConfiguration,
) => {
  let optionEquivalents =
    configA.faderBlink == configB.faderBlink &&
    configA.accelBlink == configB.accelBlink &&
    configA.controllerFlip == configB.controllerFlip &&
    configA.midiThru == configB.midiThru &&
    configA.midiMode == configB.midiMode &&
    configA.dxMode == configB.dxMode &&
    configA.pageNumber == configB.pageNumber;

  if ("i2cLeader" in configA || "i2cLeader" in configB) {
    optionEquivalents =
      optionEquivalents && configA.i2cLeader == configB.i2cLeader;
  }

  if ("fadermax" in configA || "fadermax" in configB) {
    optionEquivalents =
      optionEquivalents &&
      configA.faderMax == configB.faderMax &&
      configA.faderMin == configB.faderMin;
  }

  let usbEquivalent = true;
  let trsEquivalent = true;

  configA.usbControls.forEach((control: Control, i: number) => {
    const otherControl = configB.usbControls[i];
    if (
      control.channel != otherControl.channel ||
      control.cc != otherControl.cc
    ) {
      usbEquivalent = false;
    }
  });

  configA.trsControls.forEach((control: Control, i: number) => {
    const otherControl = configB.trsControls[i];
    if (
      control.channel != otherControl.channel ||
      control.cc != otherControl.cc
    ) {
      trsEquivalent = false;
    }
  });

  let usbButtonEquivalent = true;
  let trsButtonEquivalent = true;

  configA.usbButtons.forEach((button:Button, i) => {
    const otherButton = configB.usbButtons[i];
    if (
      button.channel != otherButton.channel ||
      button.mode != otherButton.mode ||
      (button.paramA != otherButton.paramA) ||
        (button.paramB != otherButton.paramB)
    ) {
      usbButtonEquivalent = false;
    }
  });

  configA.trsButtons.forEach((button:Button, i) => {
    const otherButton = configB.trsButtons[i];
    if (
      button.channel != otherButton.channel ||
      button.mode != otherButton.mode ||
      (button.paramA != otherButton.paramA) ||
        (button.paramB != otherButton.paramB)
    ) {
      trsButtonEquivalent = false;
    }
  });

  return optionEquivalents && 
         usbEquivalent && 
         trsEquivalent && 
         usbButtonEquivalent && 
         trsButtonEquivalent;
};

export const toSysexArray = (config: ControllerConfiguration) => {
  const array = Array.from({ length: 116 }, () => 0);
  const versionArray = config.firmwareVersion
    .trim()
    .split(".")
    .map((n) => parseInt(n));

  array[0] = config.deviceId;
  array[1] = versionArray[0];
  array[2] = versionArray[1];
  array[3] = versionArray[2];

  array[4] = config.faderBlink ? 1 : 0;
  array[5] = config.accelBlink ? 1 : 0;
  array[6] = config.controllerFlip ? 1 : 0;

  array[7] = config.i2cLeader ? 1 : 0;

  const faderminMSB = config.faderMin >> 7;
  const faderminLSB = config.faderMin - (faderminMSB << 7);
  array[8] = faderminLSB;
  array[9] = faderminMSB;
  const fadermaxMSB = config.faderMax >> 7;
  const fadermaxLSB = config.faderMax - (fadermaxMSB << 7);
  array[10] = fadermaxLSB;
  array[11] = fadermaxMSB;
  array[12] = config.midiThru ? 1 : 0;
  array[13] = config.midiMode ? 1 : 0;
  array[14] = config.dxMode ? 1 : 0;
  array[15] = config.pageNumber;



  const usbChannelOffset = 20;
  const trsChannelOffset = 36;
  const usbControlOffset = 52;
  const trsControlOffset = 68;

  config.usbControls.forEach((control, index) => {
    array[index + usbChannelOffset] = control.channel;
    array[index + usbControlOffset] = control.cc;
  });
  config.trsControls.forEach((control, index) => {
    array[index + trsChannelOffset] = control.channel;
    array[index + trsControlOffset] = control.cc;
  });

  let usbButtonChannelOffset = 84;
  let trsButtonChannelOffset = 88;
  let usbButtonModeOffset = 92;
  let trsButtonModeOffset = 96;
  let usbButtonParamAOffset = 100;
  let trsButtonParamAOffset = 104;
  let usbButtonParamBOffset = 108;
  let trsButtonParamBOffset = 112;

  config.usbButtons.forEach((button, index) => {
    array[index + usbButtonChannelOffset] = button.channel;
    array[index + usbButtonModeOffset] = button.mode;
    array[index + usbButtonParamAOffset] = button.paramA;
    array[index + usbButtonParamBOffset] = button.paramB;
  });

  config.trsButtons.forEach((button, index) => {
    array[index + trsButtonChannelOffset] = button.channel;
    array[index + trsButtonModeOffset] = button.mode;
    array[index + trsButtonParamAOffset] = button.paramA;
    array[index + trsButtonParamBOffset] = button.paramB;
  });

  return array;
};

export const toDeviceOptionsSysexArray = (config: ControllerConfiguration) => {
  return toSysexArray(config).slice(4, 20);
};

export const toUSBOptionsSysexArray = (config: ControllerConfiguration) => {
  const fullArray = toSysexArray(config);
  const channels = fullArray.slice(20, 36);
  const ccs = fullArray.slice(52, 68);
  return channels.concat(ccs);
};

export const toTRSOptionsSysexArray = (config: ControllerConfiguration) => {
  const fullArray = toSysexArray(config);
  const channels = fullArray.slice(36, 52);
  const ccs = fullArray.slice(68, 84);
  return channels.concat(ccs);
};

export const configToJsonString = (config: ControllerConfiguration) => {
  const o = { ...config };
  // truncate all controllers to length $length;
  const controllerCount = deviceForId(config.deviceId).controlCount;
  const buttonsCount = deviceForId(config.deviceId).buttonCount;

  o.usbControls = o.usbControls.splice(0, controllerCount);
  o.usbControls.forEach((c) => delete c.val);

  o.trsControls = o.trsControls.splice(0, controllerCount);
  o.usbButtons = o.usbButtons.splice(0, buttonsCount);
  o.trsButtons = o.trsButtons.splice(0, buttonsCount);

  return JSON.stringify(o, null, 2);
};

export const isConfigForDevice = (
  config: ControllerConfiguration,
  json: ControllerConfiguration,
) => {
  if (json.deviceId != config.deviceId) {
    return `Cannot update - this data file is for a ${
      deviceForId(json.deviceId).name
    }, but you are trying to install it on a ${
      deviceForId(config.deviceId).name
    } `;
  }

  // if(json.firmwareVersion != config.firmwareVersion) {
  //   return(`Cannot update - this data file is for firmware version ${json.firmwareVersion}, but you are trying to install it on a device running firmware version ${config.firmwareVersion}`);
  // }

  return false;
};

export const updateFromJson = (
  config: ControllerConfiguration,
  json: ControllerConfiguration,
) => {
  Object.keys(json).forEach((key) => {
    if(key !== 'pageNumber') {
      config[key] = json[key];
    }
  });

  return config;
};

export const configFromSysexArray = (data: number[]) => {
  logger("Generating config from", data);
  const offset = 8;

  const deviceId = data[5];
  const firmwareVersion = data[6] + "." + data[7] + "." + data[8];

  const faderBlink = data[1 + offset] == 1;
  const accelBlink = data[2 + offset] == 1;
  const controllerFlip = data[3 + offset] == 1;

  const i2cLeader = data[4 + offset] == 1;

  const faderminLSB = data[5 + offset];
  const faderminMSB = data[6 + offset];
  const faderMin = (faderminMSB << 7) + faderminLSB;

  const fadermaxLSB = data[7 + offset];
  const fadermaxMSB = data[8 + offset];
  const faderMax = (fadermaxMSB << 7) + fadermaxLSB;

  const midiThru = data[9 + offset] == 1;
  const midiMode = data[10 + offset] == 1;
  const dxMode = data[11 + offset] == 1;
  const pageNumber = data[12 + offset];

  const usbControls: Partial<Control>[] = [];
  const trsControls: Partial<Control>[] = [];
  const usbButtons: Partial<Button>[] = [];
  const trsButtons: Partial<Button>[] = [];

  data.slice(17 + offset, 33 + offset).forEach((chan, i) => {
    if (chan != 0x7f) {
      usbControls[i] = {
        channel: chan,
      };
    }
  });

  data.slice(33 + offset, 49 + offset).forEach((chan, i) => {
    if (chan != 0x7f) {
      trsControls[i] = {
        channel: chan,
      };
    }
  });

  data.slice(49 + offset, 65 + offset).forEach((cc, i) => {
    if (cc != 0x7f) {
      usbControls[i].cc = cc;
    }
  });

  data.slice(65 + offset, 81 + offset).forEach((cc, i) => {
    if (cc != 0x7f) {
      trsControls[i].cc = cc;
    }
  });

  data.slice(81 + offset, 85 + offset).forEach((chan, i) => {
    if (chan != 0x7f) {
      usbButtons[i] = {
        channel: chan,
      };
    }
  });

  data.slice(85 + offset, 89 + offset).forEach((chan, i) => {
    if (chan != 0x7f) {
      trsButtons[i] = {
        channel: chan,
      };
    }
  });

  data.slice(89 + offset, 93 + offset).forEach((mod, i) => {
    if (mod != 0x7f) {
      usbButtons[i].mode = mod;
    }
  });

  data.slice(93 + offset, 97 + offset).forEach((mod, i) => {
    if (mod != 0x7f) {
      trsButtons[i].mode = mod;
    }
  });

  data.slice(97 + offset, 101 + offset).forEach((par, i) => {
    if (par != 0x7f) {
      usbButtons[i].paramA = par;
    }
  });

  data.slice(101 + offset, 105 + offset).forEach((par, i) => {
    if (par != 0x7f) {
      trsButtons[i].paramA = par;
    }
  });

  data.slice(105 + offset, 109 + offset).forEach((par, i) => {
    if (par != 0x7f) {
      usbButtons[i].paramB = par;
    }
  });

  data.slice(109 + offset, 113 + offset).forEach((par, i) => {
    if (par != 0x7f) {
      trsButtons[i].paramB = par;
    }
  });

  usbControls.forEach((c) => (c.val = 0));

  return {
    faderBlink,
    accelBlink,
    controllerFlip,
    midiThru,
    midiMode,
    dxMode,
    usbControls,
    trsControls,
    deviceId,
    firmwareVersion,
    i2cLeader,
    faderMin,
    faderMax,
    usbButtons,
    trsButtons,
    pageNumber
  } as ControllerConfiguration;
};

export const deviceForId = (id: number) => allKnownDevices[id];

type Capability = "led" | "i2c";

export const deviceHasCapability = (
  config: ControllerConfiguration,
  capability: Capability,
) => {
  const device = deviceForId(config.deviceId);
  return !!device.capabilities[capability];
};

export const allKnownDevices = [
  {
    name: "unknown",
    controlCount: 0,
    capabilities: {},
  },
  {
    name: "Oxion development board",
    controlCount: 4,
    capabilities: {
      led: true,
    },
  },
  {
    name: "16n",
    controlCount: 16,
    capabilities: {
      i2c: true,
      led: true,
    },
  },
  {
    name: "16n (LC)",
    controlCount: 16,
    capabilities: {
      i2c: true,
      led: true,
    },
    sendShortMessages: true,
  },
  {
    name: "Music Thing 8mu",
    controlCount: 16,
    buttonCount: 4,
    capabilities: {
      i2c: false,
      led: true,
    },
  },
];
