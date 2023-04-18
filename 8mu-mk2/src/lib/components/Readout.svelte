<script lang='ts'>
  import { configuration } from '$lib/stores'
  import { randomTip } from '$lib/tips';
</script>

{#if $configuration}
<p>
  Fader blink 
  {#if $configuration.faderBlink}
  on
  {:else if !$configuration.faderBlink}
  off 
  {/if}
  | 
   
  Gesture blink
  {#if $configuration.accelBlink}
  on
  {:else if !$configuration.accelBlink}
  off
  {/if}
  | 
   
  Usb on
  {#if $configuration.controllerFlip}
  left
  {:else if !$configuration.controllerFlip}
  right 
  {/if}
  | 
   
  Midi thru
  {#if $configuration.midiThru}
  on
  {:else if !$configuration.midiThru}
  off 
  {/if}
  | 
   
  {#if $configuration.midiMode}
  Midi Type B
  {:else if !$configuration.midiMode}
  Midi Type A
  {/if}
  |
   
  <!--
  DX mode
  {#if $configuration.dxMode}
  on
  {:else if !$configuration.dxMode}
  off 
  {/if}
  |
  -->
   
  Tip: {randomTip()}
   
  </p>
{/if}
