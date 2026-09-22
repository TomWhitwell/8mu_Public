<script lang="ts">
  /* global __BUILD_VERSION__, __FIRMWARE_VERSION__ */

  import { browser } from "$app/environment";
  import { onMount } from "svelte";
  import { WebMidi } from "webmidi";

  import { onMIDISuccess } from "$lib/midi/midi";

  import {
    configuration,
    editMode,
    modernEditorDevice,
    webMidiEnabled,
  } from "$lib/stores";

  import DeviceDetails from "$lib/components/DeviceDetails.svelte";
  import Editing from "./Editing.svelte";
  import Viewing from "./Viewing.svelte";

  const buildVersion = __BUILD_VERSION__;
  const latestFirmwareVersion = __FIRMWARE_VERSION__;

  const supportsWebMidi = browser ? !!navigator.requestMIDIAccess : true;

  onMount(() => {
    // Initialise all the MIDI things.
    if (supportsWebMidi) {
      WebMidi.enable({ sysex: true })
        .then(onMIDISuccess)
        .catch((err) => alert(err));
    }
  });
</script>

<main>
  <div id="head">
    <h1>8mu configuration tool</h1>
    <DeviceDetails />
  </div>

  <div id="inner">
    {#if $webMidiEnabled}
      {#if $modernEditorDevice}
        {#if $modernEditorDevice.needsFirmwareUpdate}
          <p class="notice modern-editor-notice">
            <strong>
              Please update your 8mu v1 to firmware {latestFirmwareVersion}
              before using the 16n Editor.
            </strong>
            <br /><br />
            <a href="https://github.com/TomWhitwell/Smith-Kakehashi/releases"
              >Download the latest firmware</a
            >
          </p>
        {:else}
          <p class="notice modern-editor-notice">
            <strong>
              Please use the
              <a href="https://16n-faderbank.github.io/editor/">16n Editor</a>
            </strong>
          </p>
        {/if}
      {:else if $configuration}
        <!-- webmidi enabled, config setup, we're good to edit -->
        {#if $editMode}
          <Editing />
        {:else}
          <Viewing />
        {/if}
        <p />
      {:else}
        <!-- webmidi compatible browser, but no device -->
        <p class="notice">
          Searching for a controller via USB, hang on a second or ten.<br /><br
          />
          If you haven't plugged in your 8mu, do it now.<br /><br /><br />
          <img
            src="https://www.musicthing.co.uk/images/8mu_editor_crop.png"
            alt="Music Thing Modular 8mu controller"
          />
        </p>
      {/if}
    {:else}
      <!-- webmidi incompatible device -->
      <p class="notice">
        WebMIDI could not be enabled. Please use a web browser that supports
        WebMIDI, such as Google Chrome.
      </p>
    {/if}
  </div>

  <div id="foot">
    <div class="foot-left">
      8mu Editor v{buildVersion}<br />
      <a href="https://github.com/TomWhitwell/Smith-Kakehashi/releases"
        >Latest Firmware for 8mu v1</a
      >
    </div>
  </div>
</main>

<style>
  #head {
    margin-bottom: 1rem;
    border-bottom: 1px solid #ccc;
    display: flex;
    flex: initial;
  }

  #head h1 {
    flex: 1 0;
    margin: 0 0 1rem;
  }

  main {
    width: 75%;
    margin: 0 auto;
    min-width: calc(16 * 75px);
    /* background: #fff; */
    display: flex;
    flex-direction: column;
    height: calc(100vh - 5rem);
  }

  #inner {
    flex: 1;
  }

  #foot {
    flex: initial;
    font-size: 80%;
    border-top: 1px solid #ccc;
    padding: 1rem 0;
    display: flex;
  }

  .foot-left {
    flex: 1;
  }

  .notice {
    text-align: center;
    margin-top: 6rem;
  }

  .modern-editor-notice {
    font-size: 1.5rem;
  }
</style>
