import { sveltekit } from "@sveltejs/kit/vite";
import { defineConfig } from "vite";
import { version, config } from "./package.json";

export default defineConfig({
  define: {
    __BUILD_VERSION__: `'${version}'`,
    __FIRMWARE_VERSION__: `'${config.firmwareVersion}'`,
  },
  plugins: [sveltekit()],
});
