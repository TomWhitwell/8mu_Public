import {
  deviceForId,
  toDeviceOptionsSysexArray,
  toSysexArray,
  toTRSOptionsSysexArray,
  toUSBOptionsSysexArray,
} from "$lib/configuration";

import type { ControllerConfiguration } from "$lib/types";
import type { Output } from "webmidi";

const SYSEX_CONSTANTS = {
  sysexMfgId: [0x7d, 0x00, 0x00],
  requestInfoMsg: 0x1f,
  updateConfigMsg: 0x0e,
  updateDeviceOptionsMsg: 0x0d,
  updateUSBOptionsMessage: 0x0c,
  updateTRSOptionsMessage: 0x0b,
};

export const isOxionSysex = (data: number[]) => {
  return (
    data[1] == SYSEX_CONSTANTS.sysexMfgId[0] &&
    data[2] == SYSEX_CONSTANTS.sysexMfgId[1] &&
    data[3] == SYSEX_CONSTANTS.sysexMfgId[2]
  );
};

export const sendConfiguration = (
  configuration: ControllerConfiguration,
  output: Output,
) => {
  if (deviceForId(configuration.deviceId).sendShortMessages) {
    sendShortConfiguration(configuration, output);
  } else {
    sendFullConfiguration(configuration, output);
  }
};

export const sendFullConfiguration = (
  configuration: ControllerConfiguration,
  output: Output,
) => {
  output.sendSysex(SYSEX_CONSTANTS.sysexMfgId, [
    SYSEX_CONSTANTS.updateConfigMsg,
    ...toSysexArray(configuration),
  ]);
};

export const sendShortConfiguration = (
  configuration: ControllerConfiguration,
  output: Output,
) => {
  output.sendSysex(SYSEX_CONSTANTS.sysexMfgId, [
    SYSEX_CONSTANTS.updateDeviceOptionsMsg,
    ...toDeviceOptionsSysexArray(configuration),
  ]);

  output.sendSysex(SYSEX_CONSTANTS.sysexMfgId, [
    SYSEX_CONSTANTS.updateUSBOptionsMessage,
    ...toUSBOptionsSysexArray(configuration),
  ]);

  output.sendSysex(SYSEX_CONSTANTS.sysexMfgId, [
    SYSEX_CONSTANTS.updateTRSOptionsMessage,
    ...toTRSOptionsSysexArray(configuration),
  ]);
};

export const requestConfig = (output: Output) => {
  output.sendSysex(SYSEX_CONSTANTS.sysexMfgId, [
    SYSEX_CONSTANTS.requestInfoMsg,
  ]);
};
