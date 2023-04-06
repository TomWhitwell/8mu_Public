export type Control = {
  channel: number;
  cc: number;
  val?: number;
};

export type ControllerConfiguration = {
  ledOn: boolean;
  ledFlash: boolean;
  controllerFlip: boolean;
  midiThru: boolean;
  deviceId: number;
  i2cLeader: boolean;
  faderMin: number;
  faderMax: number;
  firmwareVersion: string;
  usbControls: Control[];
  trsControls: Control[];
};
