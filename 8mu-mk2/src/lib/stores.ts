import { writable } from "svelte/store";
import type { ControllerConfiguration } from "$lib/types";
import type { Input, Output } from "webmidi";

export const configuration = writable(null as ControllerConfiguration | null);
export const controllerMightNeedFactoryReset = writable(false);
export const editConfiguration = writable(
  null as ControllerConfiguration | null,
);
export const editMode = writable(false);
export const midiInputs = writable([] as Input[]);
export const midiOutputs = writable([] as Output[]);
export const selectedMidiInput = writable(null as Input | null);
export const selectedMidiOutput = writable(null as Output | null);
export const webMidiEnabled = writable(false);
