export const tips = [
  "Set a channel to 0 to disable it",
  "Read the instructions on the back!",
  "Click Edit Config for device options",
  "Ignore those eject warnings",
  "The boot folder is called MT_BOOT",
];

export const randomTip = () => tips[Math.floor(Math.random() * tips.length)];
