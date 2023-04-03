<script>
  import { editConfiguration } from "../stores.js";

  export let index;
  export let editControl;

  let possibleChannels = Array.from(Array(17).keys());
  possibleChannels.forEach((c, i) => (possibleChannels[i] = c ));
//   possibleChannels.forEach((c, i) => (possibleChannels[i] = c + 1));

  let channelNames = ["OFF","1","2","3","4","5","6","7","8","9","10","11","12","13","14","15","16"]

  let names = ["Fader 1", "Fader 2", "Fader 3", "Fader 4", "Fader 5", "Fader 6","Fader 7", "Fader 8", " ↑ Front", "↑ Back", "↑ Right", "↑ Left", "⟳", "⟲",  "ʎɐʍ sᴉɥʇ", "this way"];
  let channelColours=[125,125,125,125,125,125,125,125,155,155,155,155,155,155,155,155]
  let possibleCCs = Array.from(Array(128).keys());

  function touchControl() {
    editConfiguration.update(old => $editConfiguration);
  }
</script>

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

<dl class="config-column">
  <dt class='index' style='background-color: rgb({channelColours[index]},125,125)'>{names[index]}</dt>
  <dt>Channel</dt>
  <dd>
    <select bind:value={editControl.channel} on:change={touchControl}>
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
      on:change={touchControl}
      min="1"
      max="127" />
  </dd>
</dl>
