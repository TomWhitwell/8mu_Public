<script lang="ts">
  import Button from "$lib/components/Button.svelte";
  import { logger } from "$lib/logger";
  import { requestConfig, sendFactoryResetRequest } from "$lib/midi/sysex";
  import { editMode, selectedMidiOutput } from "$lib/stores";

  const confirmMessage =
    "Confirm that you wish to reset your controller configuration to the default values. You will lose any configuration stored on the controller.";

  const transmitFactoryReset = () => {
    if (confirm(confirmMessage)) {
      logger("Sending factory reset request");

      $editMode = false;

      sendFactoryResetRequest($selectedMidiOutput);

      setTimeout(() => {
        requestConfig($selectedMidiOutput);
      }, 50);
    }
  };
</script>

<p>
  "Factory Reset" will reset the configuration of your 16n back to 'default'
  values; primarily, MIDI CCs 32-48, Channel 1, for both TRS and USB MIDI.
</p>

<p>
  Note that this will also reset any calibration data or other options you may
  have adjusted on the "Device Options" tab. You will need to repeat this
  configuration after performing a reset.
</p>

<Button
  label="Reset controller configuration to defaults"
  icon="history"
  on:message
  click={transmitFactoryReset}
/>
