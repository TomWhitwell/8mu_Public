<script lang='ts'>
  import { CHROMATIC, buttonModeNames, buttonNames } from "$lib/utils";
  import type { Button } from "$lib/types";

  export let button: Button;
  export let index:number;

  function fromMidi (midi:number) {
    const name = CHROMATIC[midi % 12]
    const oct = Math.floor(midi / 12) - 1
    return `${name}${oct}`
  }

  $: modeName = buttonModeNames[button.mode]
  $: buttonName = buttonNames[index]
</script>

<dl class='config-column'>
  <dt class='index'>{buttonName}</dt>
  <dt>Channel</dt>
  <dd>{button.channel}</dd>
  <dt>Mode</dt>
  <dd>{modeName}</dd>

  {#if button.mode == 1}
    <dt>Note Number</dt>
    <dd>{button.paramA} ({fromMidi(button.paramA)})</dd>
    <dt>Velocity</dt>
    <dd>{button.paramB}</dd>
  {:else}
    <dt>CC</dt>
    <dd>{button.paramA}</dd>
      <dt>On Value</dt>
    <dd>{button.paramB}</dd>
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

  dd.display {
    height: 150px;
    position: relative;
    padding: 0 0 0.5rem 0;
  }
  .bar {
    background: black;
    display: block;
    width: 35px;
    margin: 0 auto;
  }

  .inner {
    bottom: 0.5rem;
    position: absolute;
    width: 100%;
  }

  span {
    display: block;
    position: absolute;
    color: white;
    padding: 0;
    top: 6px;
    left: 0;
    width: 100%;
  }

  span.lowvalue {
    top: -20px;
    color: black;
  }
</style>
