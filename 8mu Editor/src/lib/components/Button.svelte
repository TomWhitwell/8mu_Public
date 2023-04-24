<script lang="ts">
  import Icon from "$lib/components/Icon.svelte";

  export let label: string;
  export let icon = "";
  export let disabled = false;
  export let click: () => void;
</script>

<div class="button-wrapper">
  <button {disabled} on:click={click}>
    {#if icon && icon.trim() !== ""}
      <Icon i={icon} />
    {/if}
    {label}
  </button>
</div>

<style>
  div {
    margin: 0;
    display: inline-block;
  }
  button {
    text-align: left;
    border-radius: 5px;
    display: inline-block;
    cursor: pointer;
  }

  button:disabled {
    cursor: not-allowed;
  }
</style>
