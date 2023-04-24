<script lang="ts">
  export let checked: boolean;
</script>

<p>
  <label>
    <input type="checkbox" bind:checked />
    <slot />
  </label>
</p>
