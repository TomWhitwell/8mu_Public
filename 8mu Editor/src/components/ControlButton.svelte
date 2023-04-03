<script>
  export let control;
  export let index;
  export let disableValue = false;
  let modeNames = ["CC", "Keyboard"];
  let buttonNames = ["A", "B", "C", "D"]
  let CHROMATIC = [ 'C', 'Db', 'D', 'Eb', 'E', 'F', 'F#', 'G', 'Ab', 'A', 'Bb', 'B' ]
  function fromMidi (midi) {
    var name = CHROMATIC[midi % 12]
    var oct = Math.floor(midi / 12) - 1
    return name + oct}
  
</script>
<dl class='config-column'>
  <dt class='index'>{buttonNames[index]}</dt>
  <dt>Channel</dt>
  <dd>{control.channel}</dd>
  <dt>Mode</dt>
  
  
  <dd>{modeNames[control.mode]}</dd>
{#if control.mode == "1"}
  <dt>Note Number</dt>
  <dd>{control.paramA} ({fromMidi(control.paramA)})</dd>
      

    <dt>Velocity</dt>
  <dd>{control.paramB}</dd>
{:else}
  <dt>CC</dt>
  <dd>{control.paramA}</dd>
    <dt>On Value</dt>
  <dd>{control.paramB}</dd>


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