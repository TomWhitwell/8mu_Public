# 8mu v1 SAMD21 firmware

This directory contains firmware 1.5.2 for the production SAMD21 version of
the Music Thing Modular 8mu.

Do not install it on early prototype boards with different pin assignments.
Release UF2 files and installation instructions are published on the
[Smith-Kakehashi releases page](https://github.com/TomWhitwell/Smith-Kakehashi/releases).

Version 1.5.2 adds the Radio Music mk2 SysEx protocol and compatibility with
the current 16n Editor. It also fixes LED 1, restores the four-button and SysEx
factory resets, validates external-flash data, and safely falls back to default
settings if storage is unavailable.

The firmware uses a stable `-8MU` suffix on the SAMD chip serial number. This
gives the updated firmware a distinct USB identity so that operating systems do
not retain the old `Music Thing m0 Plus` MIDI name. The suffix must remain
unchanged in future SAMD21 releases.

The RP2040-compatible configuration is stored in `8MU2040.CFG` and
`8MUBANK2.CFG`. Legacy `EEPROM.cfg` and `BANK.cfg` files are left untouched and
are not migrated.

See [BUILD.md](BUILD.md) for the tested toolchain and reproducible build steps.
