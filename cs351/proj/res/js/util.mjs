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

// The following encode/decode an ArrayBuffer as a hex string.
export function AbToHex(ab) {
  return (new Uint8Array(ab)).reduce(
    (acc, b) => acc + (b < 16 ? '0' : '') + b.toString(16),
    '');
}
export function HexToAb(hex) {
  let ab = new ArrayBuffer(hex.length / 2);
  let u8_view = new Uint8Array(ab);

  for (let i = 0; i < ab.byteLength; ++i) {
    console.log(hex.substr(2*i, 2));
    console.log(parseInt(hex.substr(2*i, 2), 16));
    u8_view[i] = parseInt(hex.substr(2*i, 2), 16);
  }

  return ab;
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

// Some helpers for making async web requests with fetch.

async function JsonFromResponse(response) {
  if (!response.ok) {
    throw new Error('Respose has error status ' + response.status +
                    ': ' + await response.text());
  }

  return await response.json();
}

export async function GetJson(url) {
  let response = await fetch(url, {
    method: 'GET',
    cache: 'no-cache',
  });

  return await JsonFromResponse(response);
}

export async function PostJson(url, data = {}) {
  let response = await fetch(url, {
    method: 'POST',
    cache: 'no-cache',
    headers: {
      'Content-Type': 'application/json',
    },
    body: JSON.stringify(data),
  });

  return await JsonFromResponse(response);
}
