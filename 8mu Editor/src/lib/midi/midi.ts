import { get } from "svelte/store";
import { gte as semverGte } from "semver";
import type {
  ControlChangeMessageEvent,
  Input,
  MessageEvent,
  Output,
} from "webmidi";
import { WebMidi } from "webmidi";

import { configFromSysexArray } from "$lib/configuration";
import { logger } from "$lib/logger";
import { isOxionSysex, requestConfig } from "$lib/midi/sysex";
import {
  configuration,
  midiInputs,
  midiOutputs,
  modernEditorDevice,
  selectedMidiInput,
  selectedMidiOutput,
  webMidiEnabled,
} from "$lib/stores";

import type { Control } from "$lib/types";

type MidiInterface = Input | Output;

let configTimeout = -1;

selectedMidiInput.subscribe((newInput) => {
  if (newInput) {
    modernEditorDevice.set(null);
    get(midiInputs).forEach((input) => {
      input.removeListener();
    });
    listenForCC(newInput);
    listenForSysex(newInput);
    configuration.set(null);
    doRequestConfig();
  }
});

selectedMidiOutput.subscribe((newOutput) => {
  if (newOutput) {
    modernEditorDevice.set(null);
    configuration.set(null);
    doRequestConfig();
  }
});

export const allMidiInputs = () => {
  return WebMidi.inputs.sort(sortMidiInterfaces);
};

export const allMidiOutputs = () => {
  return WebMidi.outputs.sort(sortMidiInterfaces);
};

export const sortMidiInterfaces = (a: MidiInterface, b: MidiInterface) => {
  const aName = `${a.manufacturer} ${a.name}`;
  const bName = `${b.manufacturer} ${b.name}`;

  if (aName < bName) {
    return -1;
  } else if (aName > bName) {
    return 1;
  } else {
    return 0;
  }
};

export const onMIDISuccess = () => {
  logger("WebMidi enabled!");
  webMidiEnabled.set(true);
  setupMidiHeartBeat();
};

const setupMidiHeartBeat = () => {
  logger("Setting up heartbeat");

  midiInputs.set(allMidiInputs());
  midiOutputs.set(allMidiOutputs());

  doMidiHeartBeat();

  WebMidi.addListener("connected", () => {
    // logger('connected event', e)

    midiInputs.set(allMidiInputs());
    midiOutputs.set(allMidiOutputs());

    doMidiHeartBeat();
  });

  WebMidi.addListener("disconnected", () => {
    // logger('disconnected event', e)
    midiInputs.set(allMidiInputs());
    midiOutputs.set(allMidiOutputs());

    // TODO: this disables MIDI on any disconnect, which is crap
    //       you should only do this if the disconnected input IS the one you were selected
    selectedMidiInput.set(null);

    // TODO: this disables MIDI on any disconnect, which is crap
    //       you should only do this if the disconnected input IS the one you were selected
    selectedMidiOutput.set(null);

    configuration.set(null);
    modernEditorDevice.set(null);
    doMidiHeartBeat();
  });
  setInterval(() => {
    doMidiHeartBeat();
  }, 1000); // Changed interval from 5000 to 1000 to make more responsive to bank changes
};

const doMidiHeartBeat = () => {
  const selectedInput = get(selectedMidiInput);
  const selectedOutput = get(selectedMidiOutput);
  if (!selectedInput && get(midiInputs).length > 0) {
    const controller = get(midiInputs).find((input) =>
      `${input.manufacturer} ${input.name}`.match(/16n|8mu|Music Thing/i),
    );
    if (controller) {
      selectedMidiInput.set(controller);
    }
  }
  if (!selectedOutput && get(midiOutputs).length > 0) {
    const controller = get(midiOutputs).find((output) =>
      `${output.manufacturer} ${output.name}`.match(/16n|8mu|Music Thing/i),
    );
    if (controller) {
      selectedMidiOutput.set(controller);
    }
  }
  // this change allows heartbeat to continue - repeatedly asking the device for configuration so long as it's attached.
  // if (!get(configuration) && selectedInput && selectedOutput) {
  if (selectedInput && selectedOutput && !get(modernEditorDevice)) {
    listenForCC(selectedInput);
    listenForSysex(selectedInput);
    logger("Hearbeat requesting config.");
    doRequestConfig();
  }
};

const controllerMoved = (event: ControlChangeMessageEvent) => {
  const config = get(configuration);
  if (config) {
    config.usbControls.forEach((c: Control) => {
      if (
        c.channel == event.message.channel &&
        c.cc == event.controller.number
      ) {
        c.val = event.rawValue;
      }
    });
    configuration.set(config); // trigger reactivity
  }
};

const listenForCC = (input: Input) => {
  input.addListener("controlchange", controllerMoved);
};

const listenForSysex = (input: Input) => {
  input.addListener("sysex", (e: MessageEvent) => {
    const data = e.message.data;
    if (!isOxionSysex(data)) {
      logger("Sysex not for us:", data);
      return;
    }
    if (data[4] == 0x0f) {
      // it's an c0nFig message!
      const firmwareVersion = `${data[6]}.${data[7]}.${data[8]}`;
      const isSignedSamd = [0x53, 0x41, 0x4d, 0x44].every(
        (value, index) => data[18 + index] === value,
      );
      const isUnsignedSamd = ["1.5.0", "1.5.1"].includes(firmwareVersion);
      const modernDevice =
        data[5] === 6
          ? isSignedSamd || isUnsignedSamd
            ? "8mu v1"
            : "8mu v2"
          : data[5] === 4 && semverGte(firmwareVersion, "1.5.0")
          ? "8mu v1"
          : null;

      if (modernDevice) {
        const needsFirmwareUpdate =
          modernDevice === "8mu v1" &&
          !semverGte(firmwareVersion, __FIRMWARE_VERSION__);
        configuration.set(null);
        modernEditorDevice.set({
          name: modernDevice,
          firmwareVersion,
          needsFirmwareUpdate,
        });
        configTimeout = -1;
        logger(
          needsFirmwareUpdate
            ? `Detected ${modernDevice} firmware ${firmwareVersion}; update required`
            : `Detected ${modernDevice}; directing the user to the 16n Editor`,
        );
        return;
      }

      modernEditorDevice.set(null);
      configuration.set(configFromSysexArray(data));
      logger("Received config", get(configuration));

      configTimeout = -1;
    }
  });
  logger("Attached sysex listener to ", input.name);
};

const doRequestConfig = () => {
  if (configTimeout < 0) {
    configTimeout = Date.now();
  }

  const selectedInput = get(selectedMidiInput);
  const selectedOutput = get(selectedMidiOutput);

  if (selectedInput && selectedOutput) {
    logger("Requesting config over " + selectedOutput.name);
    logger("Hoping to receive on " + selectedInput.name);
    requestConfig(selectedOutput);
  }
};
