<script>
  import WebMidi from "webmidi";
  import { onMount } from 'svelte'
  import { ConfigurationObject } from "../Configuration.js";
  import { logger } from "../logger.js";
  import { OxionMidi } from "../OxionMidi.js";

  import {
    configuration,
    editConfiguration,
    midiInputs,
    midiOutputs,
    selectedMidiInput,
    selectedMidiOutput,
    webMidiEnabled, 
    editMode
  } from "../stores.js";

  WebMidi.enable(function(err) {
    if (err) {
      logger("WebMidi could not be enabled.", err);
    } else {
      logger("WebMidi enabled!");
      webMidiEnabled.update(old => true);
      setupMidiHeartBeat();
    }
  }, true); // enable sysex

  function setupMidiHeartBeat() {
    logger("Setting up heartbeat")

    doMidiHeartBeat();

    WebMidi.addListener("connected", (e) => {
      // logger('connected event', e)

      midiInputs.set(OxionMidi.allInputs(WebMidi));
      midiOutputs.set(OxionMidi.allOutputs(WebMidi));

      // ToDO TODO
      // if there's not a midi input, and there is now one with a name of 16n, set it
      // if there's not a midi output, and there is now one with a name of 16n, set it

      selectedMidiInput.update(i => $selectedMidiInput);
      selectedMidiOutput.update(i => $selectedMidiOutput);
      doMidiHeartBeat();
    });

    WebMidi.addListener("disconnected", (e) => {
      // logger('disconnected event', e)
      midiInputs.set(OxionMidi.allInputs(WebMidi));
      midiOutputs.set(OxionMidi.allOutputs(WebMidi));

      // if($midiInputs.length > 0) {
        // selectedMidiInput.update(i => $midiInputs[0]);
        // logger($midiInputs);
      // } else {
        selectedMidiInput.update(i => null);
      // }

      // if($midiOutputs.length > 0) {
        // selectedMidiOutput.update(i => $midiOutputs[0]);
        // logger($midiOutputs);
      // } else {
        selectedMidiOutput.update(i => null);
      // }
      
      $configuration = null;
      doMidiHeartBeat();
    });
    setInterval(() => {
      doMidiHeartBeat();
    }, 1000); // Changed interval from 5000 to 1000 to make more responsive to bank changes
  }

  function doMidiHeartBeat() {
    if (!$selectedMidiInput && $midiInputs.length > 0) {
      // TODO TODO if 16
      // Now checks for either 16n or 'Music' midi input 
      let sixteenN = $midiInputs.find(input => input.name.match(/.*16n.*/));
      let mtmN = $midiInputs.find(input => input.name.match(/.*Music.*/));
      if(sixteenN) {
        selectedMidiInput.update(i => sixteenN);
      }
      if(mtmN) {
        selectedMidiInput.update(i => mtmN);
      }
    }
    if (!$selectedMidiOutput && $midiOutputs.length > 0) {
      // TODO TODO if 16n
      // Now checks for either 16n or 'Music' midi output 
      let sixteenN = $midiOutputs.find(output => output.name.match(/.*16n.*/));
      let mtmN = $midiOutputs.find(output => output.name.match(/.*Music.*/));
      if(sixteenN) {
        selectedMidiOutput.update(i => sixteenN);
      }
      if(mtmN) {
        selectedMidiOutput.update(i => mtmN);
      }
    }
// this change allows heartbeat to continue - repeatedly asking the device for configuration so long as it's attached. 
//     if (!$configuration && $selectedMidiInput && $selectedMidiOutput) {
    if ($selectedMidiInput && $selectedMidiOutput) {
      listenForCC($selectedMidiInput);
      listenForSysex($selectedMidiInput);
      logger("Hearbeat requesting config.");
      requestConfig();
    }
  }

  selectedMidiInput.subscribe(newInput => {
    if (newInput) {
      $midiInputs.forEach(input => {
        input.removeListener();
      });
      listenForCC(newInput);
      listenForSysex(newInput);
      configuration.update(n => null);
      requestConfig();
    }
  });

  selectedMidiOutput.subscribe(newOutput => {
    if (newOutput) {
      configuration.update(n => null);
      requestConfig();
    }
  });

  function controllerMoved(event) {
    if ($configuration) {
      $configuration.usbControls.forEach(c => {
        if (c.channel == event.channel && c.cc == event.controller.number) {
          c.val = event.value;
        }
      });
      configuration.update(c => $configuration); // trigger reactivity
    }
  }

  function listenForCC(input) {
    input.addListener("controlchange", "all", e => {
      controllerMoved(e);
    });
  }

  function listenForSysex(input) {
    input.addListener("sysex", "all", e => {
      let data = e.data;
      if (!OxionMidi.isOxionSysex(data)) {
        logger("Sysex not for us:", data);
        return;
      }
      if (data[4] == 0x0f) {
        // it's an c0nFig message!
        logger("it's a c0nFig message");
        let configBytes = [];
        data.slice(5, data.length - 1).forEach(d => {
          configBytes.push(d.toString(16).padStart(2, "0"));
        });
        configuration.update(n =>
          ConfigurationObject.returnConfigHashFromSysex(data)
        );
        
      if ($editMode) {
        let oldConfig = ConfigurationObject.clone($configuration);
        editConfiguration.update(c => oldConfig);
      }

        
        
        logger("Received config", $configuration);
        if (document.getElementById("current_config")) {
          document.getElementById("current_config").value = configBytes.join(
            " "
          );
        }
      }
    });
    logger("Attached sysex listener to ", input.name);
  }

  function requestConfig() {
    if ($selectedMidiInput && $selectedMidiOutput) {
      logger("Requesting config over " + $selectedMidiOutput.name);
      logger("Hoping to receive on " + $selectedMidiInput.name);
      OxionMidi.requestConfig($selectedMidiOutput);
    }
  }

</script>

<div>
  <slot></slot>
</div>