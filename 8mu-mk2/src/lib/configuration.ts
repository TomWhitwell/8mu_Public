import { logger } from "$lib/logger";
import type { Control, ControllerConfiguration } from "$lib/types";

export const isEquivalent = (
  configA: ControllerConfiguration,
  configB: ControllerConfiguration,
) => {
  let optionEquivalents =
    configA.ledOn == configB.ledOn &&
    configA.ledFlash == configB.ledFlash &&
    configA.controllerFlip == configB.controllerFlip &&
    configA.midiThru == configB.midiThru;

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
  const trsEquivalent = true;

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
      usbEquivalent = false;
    }
  });

  return optionEquivalents && usbEquivalent && trsEquivalent;
};

export const toSysexArray = (config: ControllerConfiguration) => {
  const array = Array.from({ length: 84 }, () => 0);
  const versionArray = config.firmwareVersion
    .trim()
    .split(".")
    .map((n) => parseInt(n));

  array[0] = config.deviceId;
  array[1] = versionArray[0];
  array[2] = versionArray[1];
  array[3] = versionArray[2];

  array[4] = config.ledOn ? 1 : 0;
  array[5] = config.ledFlash ? 1 : 0;
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

  o.usbControls = o.usbControls.splice(0, controllerCount);
  o.usbControls.forEach((c) => delete c.val);

  o.trsControls = o.trsControls.splice(0, controllerCount);

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
    config[key] = json[key];
  });

  return config;
};

export const configFromSysexArray = (data: number[]) => {
  logger("Generating config from", data);
  const offset = 8;

  const deviceId = data[5];
  const firmwareVersion = data[6] + "." + data[7] + "." + data[8];

  const ledOn = data[1 + offset] == 1;
  const ledFlash = data[2 + offset] == 1;
  const controllerFlip = data[3 + offset] == 1;

  const i2cLeader = data[4 + offset] == 1;

  const faderminLSB = data[5 + offset];
  const faderminMSB = data[6 + offset];
  const faderMin = (faderminMSB << 7) + faderminLSB;

  const fadermaxLSB = data[7 + offset];
  const fadermaxMSB = data[8 + offset];
  const faderMax = (fadermaxMSB << 7) + fadermaxLSB;

  const midiThru = data[9 + offset] == 1;

  const usbControls: Partial<Control>[] = [];
  const trsControls: Partial<Control>[] = [];

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

  usbControls.forEach((c) => (c.val = 0));

  return {
    ledOn,
    ledFlash,
    controllerFlip,
    midiThru,
    usbControls,
    trsControls,
    deviceId,
    firmwareVersion,
    i2cLeader,
    faderMin,
    faderMax,
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
];
