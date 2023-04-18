<script lang="ts">
  import { gte as semverGte } from "semver";

  import { configuration, editConfiguration } from "$lib/stores";

  import CheckOption from "$lib/components/CheckOption.svelte";
  import { deviceHasCapability } from "$lib/configuration";

  const touchControl = () => {
    editConfiguration.set($editConfiguration);
  };
</script>



{#if $configuration && $editConfiguration}

{#if deviceHasCapability($configuration, "led")}
<CheckOption bind:checked={$editConfiguration.faderBlink}>
  Blink LEDs on fader movements
</CheckOption>

<CheckOption bind:checked={$editConfiguration.accelBlink}>
  Blink LEDs on accelerometer movements
</CheckOption>
{/if}


<CheckOption bind:checked={$editConfiguration.controllerFlip}>
  Rotate controller 180º
</CheckOption>

<CheckOption bind:checked={$editConfiguration.midiThru}>
  Soft MIDI thru (echo MIDI clock/note data sent to USB out of the minijack)
</CheckOption>

<CheckOption bind:checked={$editConfiguration.midiMode}>
  {#if $editConfiguration.midiMode}
    Midi Type B (Arturia)
  {:else if !$editConfiguration.midiMode}
    Midi Type A (Make Noise, Intellijel)
  {/if}
</CheckOption>

<!-- 
<CheckOption bind:checked={$editConfiguration.dxMode}>
  DX7 Mode - sends DX sysex changes from TRS [Coming Soon]
</CheckOption>
-->

<hr />

<h3>Fader calibration</h3>

{#if $configuration.pageNumber > 0}

<b>NB: Fader Calibration is global, and can only be changed in Bank 1</b>

{:else}
<div>
  <label for="faderMin">Fader Minimum raw value</label>
  <input
    name="faderMin"
    type="number"
    bind:value={$editConfiguration.faderMin}
    on:blur={touchControl}
    min="0"
    max="{(1 << 13) - 1}" />
</div>
<div>
  <label for="faderMax">Fader Maximum raw value</label>
  <input
    name="faderMax"
    type="number"
    bind:value={$editConfiguration.faderMax}
    on:blur={touchControl}
    min="0"
    max="{(1 << 13) - 1}" />
</div>


<p class='note'>
  Every fader is slightly different - depending on conditions when it was manufactured. You shouldn't touch this unless you are having issues with your faders either reaching <code>127</code> before the end of their travel, or not at all. <br/><br/>
  NB: these instructions are quite counter-intuitive, so read this carefully!<br /> 

&#8226;  "Raw" analog values are read from the faders between 0 and 4096. <br/>
&#8226;  For Reasons, the Fader Maximum value relates to the BOTTOM of the fader range (assuming the USB cable is on the right so the controller is not in 'flipped' mode). The Fader Minimum relates to the TOP of the range.<br />
&#8226;  If when you pull the fader down, it sticks at 1 or 2 or more, so doesn't get all the way to 0, you should reduce the Fader Maximum value, maybe to 4080.<br />
&#8226;  If when you push the fader up, it stick at 126, 125 or less, so doesn't get all the way to 127, you should increase the Fader Minimum value, maybe to 100.<br />
&#8226;  Defaults are Min = 15, Max = 4080. 
</p>
  
{/if}



{#if deviceHasCapability($configuration, "i2c")}
  <hr />
  <h3>I2C Leader/Follower</h3>
  <select bind:value={$editConfiguration.i2cLeader} on:change={touchControl}>
    <option value={false}>Follower</option>
    <option value={true}>Leader</option>
  </select>

  <p class='note'><strong>Follower</strong> mode is for use with Teletype.</p>
  <p class='note'><strong>Leader</strong> mode is for use with Ansible, TXo, ER-301. 16n will not respond to Teletype when in leader mode.</p>
  <p class='note'>This will not take effect until you restart (disconnect/reconnect) your 16n.</p>
  <p class='note small'>("Leader" is sometimes also referred to as 'master' mode)</p>
{/if}


{/if}

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
