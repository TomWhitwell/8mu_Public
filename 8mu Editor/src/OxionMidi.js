import WebMidi from "webmidi";

export class OxionMidi {
  static isOxionSysex(data) {
    return (
      data[1] == this.sysexMfgId[0] &&
      data[2] == this.sysexMfgId[1] &&
      data[3] == this.sysexMfgId[2]
    );
  }

  static sortMidiInterfaces(a, b) {
    let aName = `${a.manufacturer} ${a.name}`;
    let bName = `${b.manufacturer} ${b.name}`;

    if (aName < bName) {
      return -1;
    } else if (aName > bName) {
      return 1;
    } else {
      return 0;
    }
  }

  static allInputs() {
    return WebMidi.inputs.sort(OxionMidi.sortMidiInterfaces);
  }

  static allOutputs() {
    return WebMidi.outputs.sort(OxionMidi.sortMidiInterfaces);
  }

  static sendConfiguration(configuration, output) {
    if (configuration.device().sendShortMessages) {
      this.sendShortConfiguration(configuration, output);
    } else {
      this.sendFullConfiguration(configuration, output);
    }
  }

  static sendFullConfiguration(configuration, output) {
    output.sendSysex(
      OxionMidi.sysexMfgId,
      [OxionMidi.updateConfigMsg].concat(configuration.toSysexArray())
    );
  }

  static sendShortConfiguration(configuration, output) {
    output.sendSysex(
      OxionMidi.sysexMfgId,
      [OxionMidi.updateDeviceOptionsMsg].concat(
        configuration.toDeviceOptionsSysexArray()
      )
    );

    output.sendSysex(
      OxionMidi.sysexMfgId,
      [OxionMidi.updateUSBOptionsMessage].concat(
        configuration.toUSBOptionsSysexArray()
      )
    );

    output.sendSysex(
      OxionMidi.sysexMfgId,
      [OxionMidi.updateTRSOptionsMessage].concat(
        configuration.toTRSOptionsSysexArray()
      )
    );
  }

  static requestConfig(output) {
    output.sendSysex(OxionMidi.sysexMfgId, [OxionMidi.requestInfoMsg]);
  }
}

OxionMidi.sysexMfgId = [0x7d, 0x00, 0x00];
OxionMidi.requestInfoMsg = 0x1f;
OxionMidi.updateConfigMsg = 0x0e;
OxionMidi.updateDeviceOptionsMsg = 0x0d;
OxionMidi.updateUSBOptionsMessage = 0x0c;
OxionMidi.updateTRSOptionsMessage = 0x0b;
