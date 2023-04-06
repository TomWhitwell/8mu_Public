import { get } from "svelte/store";
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
  controllerMightNeedFactoryReset,
  midiInputs,
  midiOutputs,
  selectedMidiInput,
  selectedMidiOutput,
  webMidiEnabled,
} from "$lib/stores";

import type { Control } from "$lib/types";

type MidiInterface = Input | Output;

let configTimeout = -1;

selectedMidiInput.subscribe((newInput) => {
  if (newInput) {
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
    doMidiHeartBeat();
  });
  setInterval(() => {
    doMidiHeartBeat();
  }, 5000);
};

const doMidiHeartBeat = () => {
  const selectedInput = get(selectedMidiInput);
  const selectedOutput = get(selectedMidiOutput);

  if (!selectedInput && get(midiInputs).length > 0) {
    const sixteenN = get(midiInputs).find((input) =>
      input.name.match(/.*16n.*/),
    );
    if (sixteenN) {
      selectedMidiInput.set(sixteenN);
    }
  }
  if (!selectedOutput && get(midiOutputs).length > 0) {
    const sixteenN = get(midiOutputs).find((output) =>
      output.name.match(/.*16n.*/),
    );
    if (sixteenN) {
      selectedMidiOutput.set(sixteenN);
    }
  }

  if (!get(configuration) && selectedInput && selectedOutput) {
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
      configuration.set(configFromSysexArray(data));
      logger("Received config", get(configuration));

      configTimeout = -1;
      controllerMightNeedFactoryReset.set(false);
    }
  });
  logger("Attached sysex listener to ", input.name);
};

const doRequestConfig = () => {
  if (configTimeout < 0) {
    configTimeout = Date.now();
  }

  if (Date.now() - configTimeout > 8000) {
    if (!get(controllerMightNeedFactoryReset)) {
      controllerMightNeedFactoryReset.set(true);
    }
  }

  const selectedInput = get(selectedMidiInput);
  const selectedOutput = get(selectedMidiOutput);

  if (selectedInput && selectedOutput) {
    logger("Requesting config over " + selectedOutput.name);
    logger("Hoping to receive on " + selectedInput.name);
    requestConfig(selectedOutput);
  }
};
