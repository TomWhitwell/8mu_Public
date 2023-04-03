<script>
  import { setContext } from 'svelte'
  import cmp from 'semver-compare';
  import { ConfigurationObject } from "./Configuration.js";
  import { ImportExport } from "./ImportExport.js";
  import { logger } from "./logger.js";
  import { OxionMidi } from "./OxionMidi.js";

  import {
    configuration,
    editConfiguration,
    selectedMidiInput,
    selectedMidiOutput,
    webMidiEnabled,
    editMode
  } from "./stores.js";

  import Button from "./components/Button.svelte";
  import CheckOption from "./components/CheckOption.svelte";
  import Control from "./components/Control.svelte";
  import EditControl from "./components/EditControl.svelte";
  import ControlButton from "./components/ControlButton.svelte";
  import EditControlButton from "./components/EditControlButton.svelte";
  import DebugConsole from "./components/DebugConsole.svelte";
  import DeviceOptions from "./components/DeviceOptions.svelte";
  import DeviceOptionsShow from "./components/DeviceOptionsShow.svelte";
  import Icon from "./components/Icon.svelte";
  import MidiContext from "./components/MidiContext.svelte";
  import MidiEnabled from "./components/MidiEnabled.svelte";
  import MidiSelector from "./components/MidiSelector.svelte";
//   import imuNote from "./components/imuNote.svelte";
  import Subhead from "./components/Subhead.svelte";
  import { Tabs, TabList, TabPanel, Tab } from './components/tabs';

  window.debugMode = true;

  $editMode = false;
  let configDirty = false;
  let upgradeString = "";
  let tips = ["Set a channel to 0 to disable it","Read the instructions on the back!", "Click Edit Config for device options", "Ignore those eject warnings", "The boot folder is called MT_BOOT"]
  let tip = tips[Math.floor(Math.random() * tips.length)]


  editConfiguration.subscribe(c => {
    if (c && $configuration) {
      configDirty = !c.isEquivalent($configuration);
    }
  });

  configuration.subscribe(c => {
    if (c && c.firmwareVersion && window.latestVersion && !window.versionCompared) {
      let delta = cmp(window.latestVersion,c.firmwareVersion);
      window.versionCompared = true;
      if(delta > 0) {
        upgradeString = `A new version of 8mu firmware (${window.latestVersion}) is available.`;
      } else {
        upgradeString = "";
      }
    }
  })

  function handleMessage(message) {
    switch (message.detail.name) {
      case "toggleEditMode":
        toggleEditMode();
        break;
      case "transmitConfig":
        transmitConfig();
        break;
      case "requestConfig":
        OxionMidi.requestConfig($selectedMidiOutput);
        break;
      case "importConfig":
        ImportExport.import($editConfiguration, $configuration, editConfiguration);
        break;
      case "exportConfig":
        ImportExport.export($configuration);
        break;
      default:
        break;
    }
  }

  function toggleEditMode() {
    $editMode = !$editMode;
    if ($editMode) {
      let oldConfig = ConfigurationObject.clone($configuration);
      editConfiguration.update(c => oldConfig);
    }
  }

  function transmitConfig() {
    let sysexArray = $editConfiguration.toSysexArray();
    logger("Sending sysex:", sysexArray);

    OxionMidi.sendConfiguration($editConfiguration, $selectedMidiOutput);

    $configuration = $editConfiguration;

    $editMode = !$editMode;
  }

  fetch("https://api.github.com/repos/TomWhitwell/Smith-Kakehashi/releases")
    .then(r => r.json())
    .then(d => {
      window.latestVersion = d[0].tag_name.replace(/[a-zA-z]/g, "");
    });
</script>

<style>

  #controls {
    display: flex;
    min-width: calc(16 * 60px);
  }

  #head {
    margin-bottom: 1rem;
    border-bottom: 1px solid #ccc;
    display: flex;
  }

  #head h1 {
    flex: 1 0;
    margin: 0 0 2rem;
  }

  #head .details {
    flex: 1 0;
  }

  p.device {
    text-align: right;
  }

  main {
    width: 75%;
    margin: 0 auto;
    min-width: calc(16*75px);
    /* background: #fff; */
  }

  #foot {
    font-size: 80%;
    border-top: 1px solid #ccc;
    padding-top: 5px;
  }

 span.upgrade {
   display: block;
   margin-top: 25px;
     border: 10px solid pink;

     border-image: repeating-linear-gradient(
    -55deg,
    #000,
    #000 5px,
    #ffb101 5px,
    #ffb101 10px
  ) 10;
   color: #000;
   text-align: center;
   padding: 5px;
   background: #ffcc33; 
 } 
</style>

<MidiContext>
  <main>
    <div id="head">
      <h1>8mu Controller Editor</h1>
      <MidiEnabled>
        <div class="details">
          <!-- <MidiSelector /> -->
          {#if $configuration}
            <p class='device'>
              Connected: <strong>{$configuration.device().name}</strong> running firmware <strong>{$configuration.firmwareVersion}</strong>
              {#if upgradeString.trim() != ""}
              <span class='upgrade'>
                {upgradeString}
                <a href="https://github.com/TomWhitwell/Smith-Kakehashi/releases">Download</a>
              </span>
              {/if}
            </p>
          {:else}
            <p class='device'>No device connected.</p>
          {/if}
        </div>
      </MidiEnabled>
    </div>
    <MidiEnabled fallback="WebMIDI could not be enabled. Please use a web browser that supports WebMIDI, such as Google Chrome, and allow the browser to use MIDI Devices.">
      {#if $configuration}
        {#if $editMode}
          <Subhead title="Bank {$configuration.pageNumber + 1}: Edit configuration">
            <Button label="Cancel" icon="times" clickMessageName="toggleEditMode" on:message={handleMessage} />
            <Button label="Import config" icon="file-import" clickMessageName="importConfig" on:message={handleMessage}/>
            <Button label="Update controller" icon="download" clickMessageName="transmitConfig" disabled={!configDirty} on:message={handleMessage}  />
          </Subhead>
          <Tabs>
            <TabList>
              <Tab>USB</Tab> 
              <Tab>TRS Midi</Tab> 
              <Tab>USB Buttons</Tab> 
              <Tab>TRS Buttons</Tab> 
              <Tab>Device Options</Tab> 
            </TabList>

         <TabPanel>
              <div id="controls">
                {#each $editConfiguration.usbControls as editControl, index}
                  {#if index < $configuration.device().controlCount}
                    <EditControl {editControl} {index} />
                  {/if}
                {/each}
              </div>
            </TabPanel>
          
            <TabPanel>
              <div id="controls">
                {#each $editConfiguration.trsControls as editControl, index}
                  {#if index < $configuration.device().controlCount}
                    <EditControl {editControl} {index} />
                  {/if}
                {/each}
              </div>
            </TabPanel>

            <TabPanel>
               <div id="controls">
                {#each $editConfiguration.usbButtons as editControl, index}
                  {#if index < $configuration.device().buttonCount}
                    <EditControlButton {editControl} {index} />
                    
                  {/if}
                {/each}
              </div>
            </TabPanel>
            
            <TabPanel>
               <div id="controls">
                {#each $editConfiguration.trsButtons as editControl, index}
                  {#if index < $configuration.device().buttonCount}
                    <EditControlButton {editControl} {index} />
                  {/if}
                {/each}
              </div>
            </TabPanel>

            <TabPanel>
              <DeviceOptions />
            </TabPanel>
            
          </Tabs>
        {:else}
          <Subhead title="Bank {$configuration.pageNumber + 1}: Current Configuration">
            <Button label="Export current config" icon="file-export" clickMessageName="exportConfig" on:message={handleMessage}/> 
            <Button label="Edit config" icon="pencil-alt" clickMessageName="toggleEditMode" on:message={handleMessage} />
            <Button label="Reload config from controller" icon="sync" clickMessageName="requestConfig" on:message={handleMessage}/> 
          </Subhead>
          <Tabs>
            <TabList>
              <Tab>USB</Tab> 
              <Tab>TRS Midi</Tab> 
                <Tab>USB Buttons</Tab> 
                <Tab>TRS Buttons</Tab> 

            </TabList>
            <TabPanel>
              <div id="controls">
                {#each $configuration.usbControls as control, index}
                  {#if index < $configuration.device().controlCount}
                    <Control {control} {index} />
                  {/if}
                {/each}
              </div>
            </TabPanel>

            <TabPanel>
              <div id="controls">
                {#each $configuration.trsControls as control, index}
                  {#if index < $configuration.device().controlCount}
                    <Control {control} {index} disableValue={true} />
                  {/if}
                {/each}
              </div>
              <p>There is no realtime preview of the TRS outputs.</p>
            </TabPanel>
            
            
                      <TabPanel>
               <div id="controls">
                {#each $configuration.usbButtons as control, index}
                  {#if index < $configuration.device().buttonCount}
                    <ControlButton {control} {index} />
                  {/if}
                {/each}
              </div>
            </TabPanel>
            
            <TabPanel>
               <div id="controls">
                {#each $configuration.trsButtons as control, index}
                  {#if index < $configuration.device().buttonCount}
                    <ControlButton {control} {index} />
                  {/if}
                {/each}
              </div>
              
            </TabPanel>


            
            
          </Tabs>
<p>
Fader blink {#if $configuration.faderBlink}
on
{:else if !$configuration.faderBlink}
off 
{/if}
 | 
 

 
Gesture blink {#if $configuration.accelBlink}
on
{:else if !$configuration.accelBlink}
off
{/if}
 | 
 

 
Usb on {#if $configuration.controllerFlip}
left
{:else if !$configuration.controllerFlip}
right 
{/if}
 | 
 
 
Midi thru {#if $configuration.midiThru}
on
{:else if !$configuration.midiThru}
off 
{/if}
 | 
 

 
{#if $configuration.midiMode}
Midi Type B
{:else if !$configuration.midiMode}
Midi Type A
{/if}  |
 
 
<!-- 
DX mode {#if $configuration.dxMode}
on
{:else if !$configuration.dxMode}
off 
{/if}
 -->
 
 Tip: {tip}
 
</p>



          
        {/if}
        <p />
      {:else}
        <p>Searching for a controller via USB, hang on a second or ten. <br><p>If you haven't plugged in your 8mu, do it now.</p>
      {/if}
    </MidiEnabled>
    <div id="foot">8mu Editor v{"__buildversion__"}</div>
  </main>

</MidiContext>
