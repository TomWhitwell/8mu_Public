<script>
  export let group = undefined;
	export let value = undefined;
</script>
<label>
  <input type="radio" bind:group value={value}>
	<slot />
</label>

