export type Control = {
  channel: number;
  cc: number;
  val?: number;
};

export type Button = {
  channel: number;
  paramA: number;
  paramB: number;
  mode: number;
};

export type ControllerConfiguration = {
  faderBlink: boolean;
  accelBlink: boolean;
  controllerFlip: boolean;
  midiThru: boolean;
  deviceId: number;
  i2cLeader: boolean;
  faderMin: number;
  faderMax: number;
  pageNumber: number;
  dxMode: boolean;
  midiMode: boolean;
  firmwareVersion: string;
  usbControls: Control[];
  trsControls: Control[];
  usbButtons: Button[];
  trsButtons: Button[];
};
