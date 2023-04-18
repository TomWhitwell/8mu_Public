<script lang="ts">
  import { editConfiguration } from "$lib/stores";
  import { inputNames, channelColours, channelNames } from "$lib/utils";
  import type { Control } from "$lib/types";

  export let index: number;
  export let editControl: Control;

  const possibleChannels = Array.from(Array(17).keys());
  possibleChannels.forEach((c, i) => (possibleChannels[i] = c));

  const touchChannel = () => {
    // trigger reactivity
    editConfiguration.set($editConfiguration);
  };

  const touchCC = (e: Event) => {
    const targ = e.target as HTMLInputElement;

    if (parseInt(targ.value) < 0) {
      editControl.cc = 0;
    }
    if (parseInt(targ.value) > 127) {
      editControl.cc = 127;
    }

    // trigger reactivity
    editConfiguration.set($editConfiguration);
  };
</script>

<dl class="config-column">
  <dt
    class="index"
    style="background-color: rgb({channelColours[index]},125,125)"
  >
    {inputNames[index]}
  </dt>
  <dt>Channel</dt>
  <dd>
    <select bind:value={editControl.channel} on:change={touchChannel}>
      {#each possibleChannels as channel}
        <option value={channel}>{channelNames[channel]}</option>
      {/each}
    </select>
  </dd>
  <dt>CC</dt>
  <dd>
    <input
      type="number"
      bind:value={editControl.cc}
      on:change={touchCC}
      on:blur={touchCC}
      min="0"
      max="127"
    />
  </dd>
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

  dd input:invalid {
    background: #f99;
  }
</style>
