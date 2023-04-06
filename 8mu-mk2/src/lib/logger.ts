import { dev } from "$app/environment";

// eslint-disable-next-line @typescript-eslint/no-explicit-any
export const logger = (...obj: any) => {
  if (dev) {
    console.log(...obj);
  }
};
