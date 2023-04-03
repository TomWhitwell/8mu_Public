
<script>
  import { logger } from "../logger.js";

  import {
    configuration,
  } from "../stores.js";

  import CheckOption from "./CheckOption.svelte";

</script>

<style>
  p.note {
    width: 600px;
    line-height: 1.2;
  }

  p.note.small {
    font-size: 85%;
  }
  label {
    font-weight: bold;
  }
</style>



<p>

<p>
{#if $configuration.faderBlink}
&#9989;
{:else if !$configuration.faderBlink}
&#10062; 
{/if}
Blink LEDs on fader movements
</p>

<p>
{#if $configuration.accelBlink}
&#9989;
{:else if !$configuration.accelBlink}
&#10062; 
{/if}
Blink LEDs on accelerometer movements
</p>

<p>
{#if $configuration.controllerFlip}
&#9989;
{:else if !$configuration.controllerFlip}
&#10062; 
{/if}
Controller Flip
</p>

<p>
{#if $configuration.midiThru}
&#9989;
{:else if !$configuration.midiThru}
&#10062; 
{/if}
Midi Thru
</p>

<p>
{#if $configuration.midiMode}
Midi Type B (Arturia)
{:else if !$configuration.midiMode}
Midi Type A (Make Noise, Intellijel)
{/if}
</p>


<p>
{#if $configuration.dxMode}
&#9989;
{:else if !$configuration.dxMode}
&#10062; 
{/if}
DX7 Mode 
</p>

<!-- 
     faderBlink: obj.faderBlink,
      accelBlink: obj.accelBlink,
      controllerFlip: obj.controllerFlip,
      usbControls: [],
      trsControls: [],
      deviceId: obj.deviceId,
      firmwareVersion: obj.firmwareVersion,
      i2cMaster: obj.i2cMaster,
      fadermin: obj.fadermin,
      fadermax: obj.fadermax,
      midiThru: obj.midiThru,
      midiMode: obj.midiMode, 
      dxMode: obj.dxMode,
      pageNumber: obj.pageNumber
 
 -->