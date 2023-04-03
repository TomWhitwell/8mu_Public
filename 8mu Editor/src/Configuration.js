import { logger } from "./logger.js";

export class ConfigurationObject {
  constructor({
    faderBlink = true,
    accelBlink = true,
    controllerFlip = false,
    deviceId = 0,
    i2cMaster = false,
    midiThru = false,
    midiMode = true,
    dxMode = false,
    pageNumber = 0,
    fadermin = 0,
    fadermax = 4096,
    firmwareVersion = "unknown",
    usbControls = [],
    trsControls = [],
    usbButtons = [],
    trsButtons = [],
  }) {
    this.faderBlink = faderBlink;
    this.accelBlink = accelBlink;
    this.controllerFlip = controllerFlip;
    this.deviceId = deviceId;
    this.firmwareVersion = firmwareVersion;
    this.i2cMaster = i2cMaster;
    this.midiThru = midiThru;
    this.midiMode = midiMode;
    this.dxMode = dxMode;
    this.pageNumber = pageNumber;
    this.fadermin = fadermin;
    this.fadermax = fadermax;
    this.usbControls = usbControls;
    this.trsControls = trsControls;
    this.usbButtons = usbButtons;
    this.trsButtons = trsButtons;
  }

  isEquivalent(otherConfig) {
    let optionEquivalents =
      this.faderBlink == otherConfig.faderBlink &&
      this.accelBlink == otherConfig.accelBlink &&
      this.controllerFlip == otherConfig.controllerFlip &&
      this.midiThru == otherConfig.midiThru &&
      this.midiMode == otherConfig.midiMode &&
      this.dxMode == otherConfig.dxMode &&
      this.pageNumber == otherConfig.pageNumber;

    if ("i2cMaster" in this || "i2cMaster" in otherConfig) {
      optionEquivalents =
        optionEquivalents && this.i2cMaster == otherConfig.i2cMaster;
    }

    if ("fadermax" in this || "fadermax" in otherConfig) {
      optionEquivalents =
        optionEquivalents &&
        this.fadermax == otherConfig.fadermax &&
        this.fadermin == otherConfig.fadermin;
    }

    let usbEquivalent = true;
    let trsEquivalent = true;

    this.usbControls.forEach((control, i) => {
      let otherControl = otherConfig.usbControls[i];
      if (
        control.channel != otherControl.channel ||
        control.cc != otherControl.cc
      ) {
        usbEquivalent = false;
      }
    });

    this.trsControls.forEach((control, i) => {
      let otherControl = otherConfig.trsControls[i];
      if (
        control.channel != otherControl.channel ||
        control.cc != otherControl.cc
      ) {
        usbEquivalent = false;
      }
    });

    let usbButtonEquivalent = true;
    let trsButtonEquivalent = true;
    this.usbButtons.forEach((control, i) => {
      let otherControl = otherConfig.usbButtons[i];
      if (
        control.channel != otherControl.channel ||
        control.mode != otherControl.mode ||
        (control.paramA != otherControl.paramA) |
          (control.paramB != otherControl.paramB)
      ) {
        usbButtonEquivalent = false;
      }
    });
    this.trsButtons.forEach((control, i) => {
      let otherControl = otherConfig.trsButtons[i];
      if (
        control.channel != otherControl.channel ||
        control.mode != otherControl.mode ||
        (control.paramA != otherControl.paramA) |
          (control.paramB != otherControl.paramB)
      ) {
        usbButtonEquivalent = false;
      }
    });

    return (
      optionEquivalents &&
      usbEquivalent &&
      trsEquivalent &&
      usbButtonEquivalent &&
      trsButtonEquivalent
    );
  }

  toSysexArray() {
    let array = Array.from({ length: 116 }, (v) => 0);

    array[0] = this.deviceId;
    array[1] = this.firmwareArray()[0];
    array[2] = this.firmwareArray()[1];
    array[3] = this.firmwareArray()[2];

    array[4] = this.faderBlink ? 1 : 0;
    array[5] = this.accelBlink ? 1 : 0;
    array[6] = this.controllerFlip ? 1 : 0;

    array[7] = this.i2cMaster;

    let faderminMSB = this.fadermin >> 7;
    let faderminLSB = this.fadermin - (faderminMSB << 7);
    array[8] = faderminLSB;
    array[9] = faderminMSB;
    let fadermaxMSB = this.fadermax >> 7;
    let fadermaxLSB = this.fadermax - (fadermaxMSB << 7);
    array[10] = fadermaxLSB;
    array[11] = fadermaxMSB;
    array[12] = this.midiThru ? 1 : 0;
    array[13] = this.midiMode ? 1 : 0;
    array[14] = this.dxMode ? 1 : 0;
    array[15] = this.pageNumber;

    let usbChannelOffset = 20;
    let trsChannelOffset = 36;
    let usbControlOffset = 52;
    let trsControlOffset = 68;

    this.usbControls.forEach((control, index) => {
      array[index + usbChannelOffset] = control.channel;
      array[index + usbControlOffset] = control.cc;
    });
    this.trsControls.forEach((control, index) => {
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

    this.usbButtons.forEach((control, index) => {
      array[index + usbButtonChannelOffset] = control.channel;
      array[index + usbButtonModeOffset] = control.mode;
      array[index + usbButtonParamAOffset] = control.paramA;
      array[index + usbButtonParamBOffset] = control.paramB;
    });

    this.trsButtons.forEach((control, index) => {
      array[index + trsButtonChannelOffset] = control.channel;
      array[index + trsButtonModeOffset] = control.mode;
      array[index + trsButtonParamAOffset] = control.paramA;
      array[index + trsButtonParamBOffset] = control.paramB;
    });

    return array;
  }

  toDeviceOptionsSysexArray() {
    const fullArray = this.toSysexArray();
    return fullArray.slice(4, 20);
  }

  toUSBOptionsSysexArray() {
    const fullArray = this.toSysexArray();
    const channels = fullArray.slice(20, 36);
    const ccs = fullArray.slice(52, 68);
    return channels.concat(ccs);
  }

  toTRSOptionsSysexArray() {
    const fullArray = this.toSysexArray();
    const channels = fullArray.slice(36, 52);
    const ccs = fullArray.slice(68, 84);
    return channels.concat(ccs);
  }

  toJsonString() {
    let o = ConfigurationObject.clone(this);
    // truncate all controllers to length $length;
    let controllerCount = this.device().controlCount;
     let buttonsCount = this.device().buttonCount;

    o.usbControls = o.usbControls.splice(0, controllerCount);
    o.usbControls.forEach((c) => delete c.val);

    o.trsControls = o.trsControls.splice(0, controllerCount);
    o.usbButtons = o.usbButtons.splice(0, buttonsCount);
    o.trsButtons = o.trsButtons.splice(0, buttonsCount);
    

    return JSON.stringify(o, false, 2);
  }

  isNewConfigInvalid(json) {
    if (json.deviceId != this.deviceId) {
      return `Cannot update - this data file is for a ${
        ConfigurationObject.devices[json.deviceId].name
      }, but you are trying to install it on a ${this.device().name} `;
    }

    // if(json.firmwareVersion != this.firmwareVersion) {
    //   return(`Cannot update - this data file is for firmware version ${json.firmwareVersion}, but you are trying to install it on a device running firmware version ${this.firmwareVersion}`);
    // }

    return false;
  }

  updateFromJson(json) {

    Object.keys(json).forEach((key) => {
    
      if (key != 'pageNumber') { // do not import the page number key when loading from json 
      this[key] = json[key];
      }
    });

    return this;
  }

  device() {
    return ConfigurationObject.devices[this.deviceId];
  }

  firmwareArray() {
    let arr = this.firmwareVersion.trim().split(".");
    return arr.map((n) => parseInt(n));
  }

  static clone(obj) {
    let newObj = new ConfigurationObject({
      faderBlink: obj.faderBlink,
      accelBlink: obj.accelBlink,
      controllerFlip: obj.controllerFlip,
      usbControls: [],
      trsControls: [],
      deviceId: obj.deviceId,
      firmwareVersion: obj.firmwareVersion,
      i2cMaster: obj.i2cMaster,
      fadermin: obj.fadermin,
      fadermax: obj.fadermax,
      midiThru: obj.midiThru,
      midiMode: obj.midiMode,
      dxMode: obj.dxMode,
      pageNumber: obj.pageNumber,
    });

    obj.usbControls.forEach((control, i) => {
      newObj.usbControls[i] = { ...control };
    });
    obj.trsControls.forEach((control, i) => {
      newObj.trsControls[i] = { ...control };
    });

    obj.usbButtons.forEach((control, i) => {
      newObj.usbButtons[i] = { ...control };
    });

    obj.trsButtons.forEach((control, i) => {
      newObj.trsButtons[i] = { ...control };
    });

    return newObj;
  }

  static returnConfigHashFromSysex(data) {
    logger("Generating config from", data);
    let offset = 8;

    let deviceId = data[5];
    let firmwareVersion = data[6] + "." + data[7] + "." + data[8];

    let faderBlink = data[1 + offset];
    let accelBlink = data[2 + offset];
    let controllerFlip = data[3 + offset];

    let i2cMaster = data[4 + offset] == 1;

    let faderminLSB = data[5 + offset];
    let faderminMSB = data[6 + offset];
    let fadermaxLSB = data[7 + offset];
    let fadermaxMSB = data[8 + offset];

    let fadermin = (faderminMSB << 7) + faderminLSB;
    let fadermax = (fadermaxMSB << 7) + fadermaxLSB;

    let midiThru = data[9 + offset];
    let midiMode = data[10 + offset];
    let dxMode = data[11 + offset];

    let pageNumber = data[12 + offset];
    let usbControls = [];
    let trsControls = [];
    let usbButtons = [];
    let trsButtons = [];

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

    return new ConfigurationObject({
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
      i2cMaster,
      fadermin,
      fadermax,
      usbButtons,
      trsButtons,
      midiThru,
      midiMode,
      dxMode,
      pageNumber,
    });
  }
}

ConfigurationObject.devices = [
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
