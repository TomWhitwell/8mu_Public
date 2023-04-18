<script lang="ts">
  import { logger } from "$lib/logger";

  import Button from "$lib/components/Button.svelte";
  import DeviceOptions from "$lib/components/DeviceOptions.svelte";
  import EditControl from "$lib/components/EditControl.svelte";
  import EditControlButton from "$lib/components/EditControlButton.svelte";
  import Subhead from "$lib/components/Subhead.svelte";
  import { Tab, TabList, TabPanel, Tabs } from "$lib/components/tabs";

  import { deviceForId, isEquivalent, toSysexArray } from "$lib/configuration";
  import { importConfig } from "$lib/import_export";
  import { sendConfiguration } from "$lib/midi/sysex";
  import {
    configuration,
    editConfiguration,
    editMode,
    selectedMidiOutput,
  } from "$lib/stores";

  let configDirty = false;

  editConfiguration.subscribe((c) => {
    if (c && $configuration) {
      configDirty = !isEquivalent(c, $configuration);
    }
  });

  const cancelEditMode = () => {
    $editMode = false;
  };

  const doImportConfig = () => {
    if ($editConfiguration && $configuration) {
      importConfig($editConfiguration, $configuration);
    }
  };

  const transmitConfig = () => {
    const sysexArray = toSysexArray($editConfiguration);
    logger("Sending sysex:", sysexArray);

    sendConfiguration($editConfiguration, $selectedMidiOutput);

    $configuration = $editConfiguration;

    $editMode = false;
  };

  $: device = $configuration ? deviceForId($configuration.deviceId) : null;
</script>

{#if $editConfiguration && $configuration}

<Subhead title="Bank {$configuration.pageNumber + 1}: Edit configuration">
  <Button label="Cancel" icon="times" click={cancelEditMode} />
  <Button label="Import config" icon="file-import" click={doImportConfig} />
  <Button
    label="Update controller"
    icon="download"
    click={transmitConfig}
    disabled={!configDirty}
  />
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
        {#if device && index < device.controlCount}
          <EditControl {editControl} {index} />
        {/if}
      {/each}
    </div>
  </TabPanel>

  <TabPanel>
    <div id="controls">
      {#each $editConfiguration.trsControls as editControl, index}
        {#if device && index < device.controlCount}
          <EditControl {editControl} {index} />
        {/if}
      {/each}
    </div>
  </TabPanel>

  <TabPanel>
    {#if device?.buttonCount}
    <div id="controls">
     {#each $editConfiguration.usbButtons as button, index}
       {#if index < device.buttonCount}
         <EditControlButton editButton={button} {index} />
       {/if}
     {/each}
   </div>
   {/if}
 </TabPanel>
 
 <TabPanel>
    {#if device?.buttonCount}
    <div id="controls">
     {#each $editConfiguration.trsButtons as button, index}
       {#if index < device.buttonCount}
         <EditControlButton editButton={button} {index} />
       {/if}
     {/each}
   </div>
    {/if}
 </TabPanel>

  <TabPanel>
    <DeviceOptions />
  </TabPanel>
</Tabs>

{/if}

<style>
  #controls {
    display: flex;
    min-width: calc(16 * 60px);
  }
</style>
