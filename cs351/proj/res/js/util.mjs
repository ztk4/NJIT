// This is a module that export some useful JS utility functions.

// Constants.
// These date values are taken from:
// https://stackoverflow.com/questions/11526504/minimum-and-maximum-date
export const kMaxDate = new Date(8640000000000000);
export const kMinDate = new Date(-8640000000000000);

// Methods.

// The following Ab to/from string conversions were taken from:
// https://developers.google.com/web/updates/2012/06/How-to-convert-ArrayBuffer-to-and-from-String
// Copies a string into a new ArrayBuffer.
export function StrToAb(str) {
  let ab = new ArrayBuffer(str.length*2);
  let u16_view = new Uint16Array(ab);

  for (let i = 0; i < str.length; ++i) {
    u16_view[i] = str.charCodeAt(i);
  }

  return ab;
}
// Copies ArrayBuffer contents to a string.
export function AbToStr(ab) {
  return String.fromCharCode.apply(null, new Uint16Array(ab));
}

// Concatenates multiple array buffers into one array buffer.
export function AbConcat(...abs) {
  // Let concat be a new array buffer,
  // whose length is the sum of the length of buffers in abs.
  let concat = new ArrayBuffer(abs.reduce((acc, ab) => acc + ab.byteLength, 0));
  let concat_view = new Uint8Array(concat);

  // Copy each array buffer to the proper offset in concat using views.
  let offset = 0;
  for (const ab in abs) {
    concat_view.set(new Uint8Array(ab), offset);
    offset += ab.byteLength;
  }

  return concat;
}
