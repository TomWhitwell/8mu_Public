# 16n editor v2

Basically: a port to:

- SvelteKit 2
- Typescript

Goals:

- Remove some of the horrible OO
- remove the horrible message handling...
- remove the horrible midiContext nonsense
- saner use of stores
- lots of nice types throughout.

Heavily built on That Other Thing.

## Worklog

- rewrote `configuration` to be functions and types
- rewrote `import_export` to be functions
- rewrote `logger` to be more Typescripty
- stores needs Types; we will get there.
- OxionMidi.ts is now midi.ts (for the time being)
  - might need to be something like midi/sysex.ts and midi/callbacks.ts
- removed MidiContext
  - replaced with midi/midi.ts and function in +page.ts
  - less clunky, I think.
- removed MidiEnabled, replaced with store and if statement
- Viewing and Editing are now subpages.
- rename `i2cMaster` in code as well as front-end.
- MessageHnadling replaced everwyere
- `__buildversion__` and `__firmwareversion__` have been replaced with Vite defines, see `vite.config.ts` for this.

---

## Notes

- setup default values for config/edit configuration, they should never be null

## Todo

- center on the page, get it looking a bit nicer
- get a deploy out so people can test it for me.
- fix up the README.
