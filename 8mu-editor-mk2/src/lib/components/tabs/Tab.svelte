<script>
  import { getContext } from "svelte";
  import { TABS } from "./Tabs.svelte";

  const tab = {};
  const { registerTab, selectTab, selectedTab } = getContext(TABS);

  registerTab(tab);
</script>

<button class:selected={$selectedTab === tab} on:click={() => selectTab(tab)}>
  <slot />
</button>

<style>
  button {
    background: none;
    border: none;
    border-bottom: 1px solid #888;
    border-radius: 0;
    margin: 0;
    color: #aaa;
    padding-left: 6px;
    padding-right: 6px;
  }

  .selected {
    /* border-bottom: 2px solid teal; */
    border-top: 1px solid #888;
    border-right: 1px solid #888;
    border-left: 1px solid #888;
    border-bottom: 1px solid #f0f0f0;
    border-radius: 2px;
    color: #333;
    padding-left: 5px;
    padding-right: 5px;
  }

  button:hover {
    cursor: pointer;
    border-top: 1px solid #888;
    border-right: 1px solid #888;
    border-left: 1px solid #888;
  }
</style>
