<script lang="ts">
  import { logger } from "$lib/logger";
  import { gte as semverGte } from "semver";

  import Button from "$lib/components/Button.svelte";
  import DeviceOptions from "$lib/components/DeviceOptions.svelte";
  import EditControl from "$lib/components/EditControl.svelte";
  import FactoryReset from "$lib/components/FactoryReset.svelte";
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

<Subhead title="Edit Configuration">
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
    <Tab>TRS Jack</Tab>
    <Tab>Device Options</Tab>
    {#if semverGte($configuration.firmwareVersion, "2.1.0")}
      <Tab>Factory Reset</Tab>
    {/if}
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
    <DeviceOptions />
  </TabPanel>

  {#if semverGte($configuration.firmwareVersion, "2.1.0")}
    <TabPanel>
      <FactoryReset on:message />
    </TabPanel>
  {/if}
</Tabs>

<style>
  #controls {
    display: flex;
    min-width: calc(16 * 60px);
  }
</style>
