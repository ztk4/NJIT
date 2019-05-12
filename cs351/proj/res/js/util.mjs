// This is a module that export some useful JS utility functions.

// Constants.
// These date values are taken from:
// https://stackoverflow.com/questions/11526504/minimum-and-maximum-date
export const kMaxDate = new Date(8640000000000000);
export const kMinDate = new Date(-8640000000000000);

// Methods.
// Copies a string into a new ArrayBuffer.
export function StrToAb(str) {
  let ab = new ArrayBuffer(str.length*2);
  let u16_view = new Uint16Array(ab);

  for (let i = 0; i < str.length; ++i) {
    u16_view[i] = str.charCodeAt(i);
  }

  return ab;
}
