<script lang='ts'>
  import { editConfiguration } from "$lib/stores.js";
  import { CHROMATIC, buttonNames, buttonModeNames } from "$lib/utils";

  import type { Button } from "$lib/types";

  export let index:number;
  export let editControl:Button;

  const possibleChannels = Array.from(Array(16).keys());
  possibleChannels.forEach((c, i) => (possibleChannels[i] = c + 1));

  const possibleCCs = Array.from(Array(128).keys());

  function touchControl() {
    editConfiguration.update(old => $editConfiguration);
  }

  const touchChannel = () => {
    // trigger reactivity
    editConfiguration.set($editConfiguration);
  };

  function fromMidi (midi:number) {
    const name = CHROMATIC[midi % 12]
    const oct = Math.floor(midi / 12) - 1
    return `${name}${oct}`
  }
  
</script>

<dl class="config-column">
  <dt class='index'>Button {buttonNames[index]}</dt>
  <dt>Channel</dt>
  <dd>
    <select bind:value={editControl.channel} on:change={touchChannel}>
      {#each possibleChannels as channel}
        <option value={channel}>{channel}</option>
      {/each}
    </select>
  </dd>

  <dt>Mode</dt>
  <dd>
   <select bind:value={editControl.mode} on:change={touchControl}>
    {#each buttonModeNames as mode, index}
      <option value={index}>{mode}</option>
    {/each}
    </select>
  </dd>

  {#if editControl.mode == 1}
  <dt>Note number</dt>
  <dd>
    <select bind:value={editControl.paramA} on:change={touchControl}>
    {#each possibleCCs as CC}
      <option value={CC}>{CC}</option>
    {/each}
    </select>
    ({fromMidi(editControl.paramA)})
  </dd>
  
  <dt>Velocity</dt>
  <dd>
    <select bind:value={editControl.paramB} on:change={touchControl}>
    {#each possibleCCs as CC}
      <option value={CC}>{CC}</option>
    {/each}
    </select>
  </dd>
  {:else}
  <dt>Controller</dt>
  <dd>
    <select bind:value={editControl.paramA} on:change={touchControl}>
    {#each possibleCCs as CC}
      <option value={CC}>{CC}</option>
    {/each}
    </select>
  </dd>
  
  <dt>On value</dt>
  <dd>
    <select bind:value={editControl.paramB} on:change={touchControl}>
    {#each possibleCCs as CC}
      <option value={CC}>{CC}</option>
    {/each}
    </select>
  </dd>
  {/if}
</dl>

<style>
  dl {
    flex: 1;
    text-align: center;
  }
  dt {
    font-weight: bold;
    border-top: 1px solid #aaa;
    padding-top: 0.5rem;
    margin-right: 5px;
  }

  dt.index {
    background: #666;
    color: #f0f0f0;
    padding: 0.5rem 0;
  }

  dd {
    padding: 0 0 0.5rem 0;
    border-bottom: 1px solid #aaa;
    margin: 0;
    margin-right: 5px;
  }
</style>
