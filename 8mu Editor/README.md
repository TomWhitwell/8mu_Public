# 8mu editor

The 8mu editor allows you to edit the configuration of your 8mu from within a web browser. 

The 8mu editor is a rework of the 16n editor,  a Javascript app based around [Svelte](svelte) built by Tom Armitage. The rest of these notes are his. 

## Build Requirements

- A recent version of node.js. 

## Usage Requirements

- As a WebMIDI app, you need a client that can support it. That basically means Chrome right now.
- A 16n running firmware v2.0.0 or higher.

## Installation

    npm install

## Running the development environment

    npm run dev

This will run a development environment at `localhost:5000`, with live reloading enabled.

By default, the server will only respond to requests from localhost.

## Building and running in production mode

To create an optimised version of the app:

    npm run build

You can run the newly built app with `npm run start`. This uses [sirv](https://github.com/lukeed/sirv), which is included in your package.json's `dependencies` so that the app will work when you deploy to platforms like [Heroku](https://heroku.com).

## Project Structure

* `src/App.Svelte` is quite dense, but is the main entrypoint for the application and contains the app's structure.
* `src/components` contains all components. Primarily UI, but `MidiContext` serves to wrap WebMidi and keep the various stores that describe the available Midi interfaces up-to-date.
* `src/Configuration.js` describes a class `ConfigurationObject`, that represents a single instance of a 16n configuration. These are what are passed around, diffed to work out if config has changed, and converted to Sysex to send to the device.
* `src/ImportExport.js` handles converting a Configuration to JSON, and back again.
* `src/OxionMidi.js` is some convenience classes for MIDI.

## Icons

Icons are from [FontAwesome](https://fontawesome.com/license/free). FA Code is MIT licensed; font files are licensed under the SIL Open Font License.

## Licensing

Editor code, like the rest of 16n sourcecode, is released under the MIT License; see `LICENSE` for more details.

