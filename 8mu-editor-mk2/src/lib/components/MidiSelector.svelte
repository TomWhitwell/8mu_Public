<script lang="ts">
  import {
    midiInputs,
    midiOutputs,
    selectedMidiInput,
    selectedMidiOutput,
  } from "$lib/stores";

  import { onDestroy, onMount } from "svelte";

  let changed = false;
  let firstRunIn = true;
  let firstRunOut = true;

  onMount(() => {
    const unsubscribeInputs = midiInputs.subscribe(() => {
      if (firstRunIn) {
        firstRunIn = false;
        return;
      }
      changed = true;
      setTimeout(() => {
        changed = false;
      }, 750);
    });

    const unsubscribeOutputs = midiOutputs.subscribe(() => {
      if (firstRunOut) {
        firstRunOut = false;
        return;
      }
      changed = true;
      setTimeout(() => {
        changed = false;
      }, 750);
    });

    onDestroy(() => {
      unsubscribeInputs();
      unsubscribeOutputs();
    });
  });
</script>

<div class="container {changed ? 'changed' : ''}">
  <div class="wrap">
    <p>
      <!-- <Icon classList="mid" i="sliders-h" /> -->
      <label for="midi-input-chooser">Input:</label>
      {#if $midiInputs.length > 0}
        <select
          name="midi-input-chooser"
          bind:value={$selectedMidiInput}
          on:change={() => selectedMidiInput.set($selectedMidiInput)}
        >
          {#each $midiInputs as input}
            <!-- <option value={input}>{input.manufacturer} {input.name}</option> -->
            <option value={input}>{input.name}</option>
          {/each}
        </select>
      {:else}No MIDI inputs detected.{/if}
    </p>

    <p>
      <label for="midi-output-chooser">Output:</label>
      {#if $midiOutputs.length > 0}
        <select
          name="midi-output-chooser"
          bind:value={$selectedMidiOutput}
          on:change={() => selectedMidiOutput.set($selectedMidiOutput)}
        >
          {#each $midiOutputs as output}
            <!-- <option value={output}>{output.manufacturer} {output.name}</option> -->
            <option value={output}>{output.name}</option>
            i
          {/each}
        </select>
      {:else}No MIDI outputs detected.{/if}
    </p>
  </div>
</div>

<style>
  .container {
    text-align: right;
  }

  .wrap {
    display: inline-block;
  }

  label {
    display: inline-block;
    font-weight: bold;
  }
  select {
    display: inline-block;
    width: 240px;
    transition: 0.75s ease;
  }

  .changed select {
    transition: 0.75s ease;
    box-shadow: 0 0 10px #800;
  }

  p {
    margin: 0;
  }
</style>
