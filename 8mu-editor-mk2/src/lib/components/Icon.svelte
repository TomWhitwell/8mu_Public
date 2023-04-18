<script lang="ts">
  export let i: string;
  export let classList = "";
</script>

<span class="fas fa-{i} {classList}" />

<style>
  span.fas.mid {
    color: #888;
  }
  span.fas.light {
    color: #ccc;
  }
</style>
