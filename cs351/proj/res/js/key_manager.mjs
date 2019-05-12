// Exports a class wrapping key management logic for the client.
// DISCLAIMER:
//   I am a complete novice when it comes to cryptography.
//   This key management library is inspired by the Signal protocol,
//   BUT WebCrypto doesn't support many of the crypto features needed.
//   As such, I have improvied with what algorithms are available,
//   and with what keys are used for what.
//   This is BY NO MEANS a faithful reproduction of Signal,
//   and is probably not even close to secure.
//   It is merely a learning exercise for me!

// Utilities.
import { StrToAb } from '/web/res/js/util.mjs'

// Convenient handle to the window Crypto object.
const crypto = window.crypto;
// Convenient handle to the SubtleCrypto implementation.
const scrypto = crypto.subtle;

// Some constants for ease of use in this library.
// This is the algorithm object needed to specify
// generation of an ECDH key using curve P-521.
const kEcdhAlgo = {
  name: 'ECDH',
  namedCurve: 'P-521',
};
// Same as above, but ECDSA for key signing.
const kEcdsaAlgo = {
  name: 'ECDSA',
  namedCurve: 'P-521',
};
// These are the signing params for ECDSA signing.
const kEcdsaSigParams = {
  name: 'ECDSA',
  hash: 'SHA-512',
};
// These are the params for encrypt/decrypt with AES-GCM.
const kAesParams = {
  name: 'AES-GCM',
  length: 256,  // Length in bits.
};

// The AES type to use for wrapping.
const kAesWrapType = 'AES-GCM';

// The export format to use when exporting keys.
const kExportFormat = 'raw';
// The export format to use when wrapping keys.
const kWrapFormat = 'jwk';

// Gets the derivation parameters for a Pbkdf2 key,
// using a salt (of at least 16 bytes) and a number if iterations.
// TODO: Tune number of iterations.
function Pbkdf2DeriveParams(salt, it = 1000) {
  return {
    name: 'PBKDF2',
    salt: salt,
    iterations: it,
    hash: { name: 'SHA-512' },
  };
}

// Generates a random IV suitable for AES methods.
function GenerateIv(aes_name) {
  let iv_byte_length = 0;
  switch (aes_name) {
   case 'AES-CBC':
    iv_byte_length = 16;
    break;

   case 'AES-GCM':
    iv_byte_length = 96 / 8;  // 96 bits.
    break;

   default:
    return null;  // No IVs for other methods.
  }

  // NOTE: This method is in the Crypto API, and NOT in the SubtleCrypto API.
  return crypto.getRandomValues(new Uint8Array(iv_byte_length));
}

// Gets wrapping params for an aes key.
function WrapParams(aes_key, iv, wrap_auth) {
  let params = {
    name: aes_key.algorithm.name,
  };

  switch (params.name) {
   case 'AES-GCM':
    // Only set GCM auth data if it is supplied.
    if (wrap_auth) {
      params.additionalData = wrap_auth;
    }
    // fallthrough
   case 'AES-CBC':
    params.iv = iv;
    break;

   default:
    // There are other valid wrapping key types,
    // I just haven't supported them here.
    throw new Error('Unsupported Wrapping Key');
  }

  return params;
}

// Gets the import params that would be needed to import a key generated from this algo.
function ImportParams(algo_name) {
  switch (algo_name) {
   case 'ECDH':
    return kEcdhAlgo;

   case 'ECDSA':
    return kEcdsaAlgo;

   default:
    // I don't currently support importing any other key format.
    throw new Error('Unsupported Imported Key Type');
  }
}

// This class doesn't have any state,
// and is instead just a collection of static methods.
export default class KeyManager {
  // Generates a random salt of the specified length in bytes.
  static GenerateSalt(byte_length) {
    // NOTE: This function lives in the Crypto API, not the SubtleCrypto API.
    return crypto.getRandomValues(new Uint8Array(byte_length));
  }

  // Creates a wrapping key and wrapping auth from a password and salt.
  // Useful for storing keys locally in a secure format.
  // NOTE: salt must be at least 16 bytes.
  // Returns:
  //   key: symmetric wrapping key,
  //   auth: auth data to use for wrapping.
  static async MakeWrappingKeyAndAuth(password, salt) {
    // First, we'll create a PBKDF2 key for deriving bits.
    const pass_key = await scrypto.importKey('raw', StrToAb(password), { name: 'PBKDF2' },
                                             /*extractable=*/false, ['deriveBits']);
    // Now, we'll derive 512 bits:
    //   256 for a 256-bit AES key,
    //   256 for auth data.
    const derived = await scrypto.deriveBits(Pbkdf2DeriveParams(salt), pass_key, 512);

    // We then use some of the above bits to create an AES key.
    const key = await scrypto.importKey('raw', new DataView(derived, 0, 32), { name: kAesWrapType },
                                        /*extractable=*/false, ['wrapKey', 'unwrapKey']);
    // And the rest of derived as auth data.
    const auth = derived.slice(32);

    // Finally, return the AES key and a copy of the second half of derived as the auth data.
    return { key, auth };
  }

  // Creates identity key pairs.
  // Unlike the signal protocol, we're using curve P-521 which
  // isn't necessarily suitable for dual use (signing AND derivation).
  // So, we have TWO identity key pairs, one for signing and one for derivation.
  // NOTE: when exporting the key, we sign the dh key using the dsa key.
  // I have no idea if the above change decreases security in comparison to a single identity key pair.
  // The returned object contains two key pairs:
  //   dsa: a ECDSA keypair for siging/verifying signatures.
  //   dh: a ECDH keypair for deriving bits.
  static async MakeIdentityKeyPairs() {
    // These key pairs need to be extractable.
    const dsa = await scrypto.generateKey(kEcdsaAlgo, /*extractable=*/true, ['sign', 'verify']);
    const dh = await scrypto.generateKey(kEcdhAlgo, /*extractable=*/true, ['deriveKey', 'deriveBits']);

    return { dsa, dh };
  }
  // Takes an id keypairs object, and returns an objects with two exported keys and a signature:
  //   dsa_pub: exported bytes of dsa.publicKey.
  //   dh_pub: exported bytes of dh.publicKey.
  //   dh_sig: signature of above dh_pub export, signed by dsa.privateKey.
  static async ExportIdentityKeyPairs(id_keypairs) {
    const dsa_pub = await scrypto.exportKey(kExportFormat, id_keypairs.dsa.publicKey);
    const dh_pub = await scrypto.exportKey(kExportFormat, id_keypairs.dh.publicKey);
    // Sign dh_pub using dsa.privateKey.
    const dh_sig = await scrypto.sign(kEcdsaSigParams, id_keypairs.dsa.privateKey, dh_pub);

    return { dsa_pub, dh_pub, dh_sig };
  }
  // Takes an id keypairs object, a wrapping key, and a wrapping auth,
  // and returns an object with two sub objects:
  //   dsa_wrap: {
  //     pub: { data: wrapped dsa public key, ... },
  //     priv: { data: wrapped dsa private key, ... },
  //   },
  //   dh_wrap: {
  //     pub: { data: wrapped dh public key, ... },
  //     priv: { data: wrapped dh private key, ... },
  //   }
  static async WrapIdentityKeyPairs(id_keypairs, wrap_key, wrap_auth) {
    const dsa_wrap = await this.WrapKeyPair(id_keypairs.dsa, wrap_key, wrap_auth);
    const dh_wrap = await this.WrapKeyPair(id_keypairs.dh, wrap_key, wrap_auth);

    return { dsa_wrap, dh_wrap };
  }
  // Unwraps keys wrapped above.
  // Returns:
  //   dsa: dsa key pair,
  //   dh: dh key pair.
  static async UnwrapIdentityKeyPairs(wrapped, wrap_key, wrap_auth) {
    const dsa = await this.UnwrapKeyPair(wrapped.dsa_wrap, wrap_key, wrap_auth);
    const dh = await this.UnwrapKeyPair(wrapped.dh_wrap, wrap_key, wrap_auth);

    return { dsa, dh };
  }

  // Creates signed key pair (ECDH).
  // NOTE: This key pair is only signed when exported.
  static async MakeSignedKeyPair() {
    return await scrypto.generateKey(kEcdhAlgo, /*extractable=*/true, ['deriveKey', 'deriveBits']);
  }
  // Exports and signs the above key, returning:
  //   pub: exported bytes of public signing key,
  //   sig: signature of public signing key, signed by id_keypairs.dsa.privateKey.
  static async ExportSignedKeyPair(signed_keypair, id_keypairs) {
    const pub = await scrypto.exportKey(kExportFormat, signed_keypair.publicKey);
    // Sign pub with dsa.privateKey.
    const sig = await scrypto.sign(kEcdsaSigParams, id_keypairs.dsa.privateKey, pub);

    return { pub, sig };
  }
  // Wraps the signed key pair, returning:
  //   pub: { data: wrapped public key, ... },
  //   priv: { data: wrapped private key, ... },
  static async WrapSignedKeyPair(signed_keypair, wrap_key, wrap_auth) {
    return await this.WrapKeyPair(signed_keypair, wrap_key, wrap_auth);
  }
  // Unwraps keypair wrapped above, returning the key pair.
  static async UnwrapSignedKeyPair(wrapped, wrap_key, wrap_auth) {
    return await this.UnwrapKeyPair(wrapped, wrap_key, wrap_auth);
  }

  // Creates an ephemeral key pair.
  static async MakeEphemeralKeyPair() {
    return await scrypto.generateKey(kEcdhAlgo, /*extractable=*/true, ['deriveKey', 'deriveBits']);
  }
  // Exports an ephemeral key, returning the exported public key.
  static async ExportEphemeralKeyPair(ephemeral_keypair) {
    return await scrypto.exportKey(kExportFormat, ephemeral_keypair.publicKey);
  }
  // Wraps the ephemeral key pair, returning:
  //   pub: { data: wrapped public key, ... },
  //   priv: { data: wrapped private key, ... },
  static async WrapEphemeralKeyPair(ephemeral_keypair, wrap_key, wrap_auth) {
    return await this.WrapKeyPair(ephemeral_keypair, wrap_key, wrap_auth);
  }
  // Unwraps keypair wrapped above, returning the key pair.
  static async UnwrapEphemeralKeyPair(wrapped, wrap_key, wrap_auth) {
    return await this.UnwrapKeyPair(wrapped, wrap_key, wrap_auth);
  }

  // Creates count one-time key pairs,
  // returning an array of key pairs.
  static async MakeOneTimeKeyPairs(count) {
    // Just generate count ECDH key pairs.
    return await Promise.all(Array.from(Array(count),
      _ => scrypto.generateKey(kEcdhAlgo, /*extractable=*/true, ['deriveKey', 'deriveBits'])
    ));
  }
  // Exports a one-time key, returning the exported public key.
  static async ExportOneTimeKey(one_time_keypair) {
    return await scrypto.exportKey(kExportFormat, one_time_keypair.publicKey);
  }
  // Wraps a one time key, returning:
  //   pub: { data: wrapped public key, ... },
  //   priv: { data: wrapped private key, ... },
  static async WrapOneTimeKeyPair(one_time_keypair, wrap_key, wrap_auth) {
    return await this.WrapKeyPair(one_time_keypair, wrap_key, wrap_auth);
  }
  // Unwraps keypair wrapped above, returning the key pair.
  static async UnwrapOneTimeKeyPair(wrapped, wrap_key, wrap_auth) {
    return await this.UnwrapKeyPair(wrapped, wrap_key, wrap_auth);
  }

  // This method imports public keys in the format from the export methods above.
  // Typically, this is used to import another person's public keys in the initiation of a session.
  // Asserts that the id dh and signed key are correctly signed by the id dsa key.
  // NOTE: Imported keys will NOT be extractable (shouldn't need to re-export them).
  // Returns:
  //   id_pks: {
  //     dsa: public dsa identity key.
  //     dh: public dh identity key.
  //   },
  //   signed_pk: public signed key.
  //   eph_pk: public ephemeral key.
  //   ot_pk: one-time public key, or null if an exported one-time key was not passed.
  static async ImportPublicKeys(id_export, signed_export, ephemeral_export, one_time_export) {
    // First get the dsa key for verifying signatures.
    const id_dsa = await scrypto.importKey(kExportFormat, id_export.dsa_pub, kEcdsaAlgo,
                                           /*extractable=*/false, ['verify']);
    
    // Now verify signatures of dh and signed key before importing others.
    const dh_verify = await scrypto.verify(kEcdsaSigParams, id_dsa, id_export.dh_sig, id_export.dh_pub);
    if (!dh_verify) {
      throw new Error('Unable to verify signature on public ECDH identity key');
    }
    const signed_verify = await scrypto.verify(kEcdsaSigParams, id_dsa, signed_export.sig, signed_export.pub);
    if (!signed_verify) {
      throw new Error('Unable to verify signature on public ECDH signed key');
    }

    // Now that we've verified, import all other keys (except optional one-time key).
    // NOTE: An ECDH public key does NOT need usage permissions for key derivation.
    const id_dh = await scrypto.importKey(kExportFormat, id_export.dh_pub, kEcdhAlgo,
                                          /*extractable=*/false, []);
    const signed_pk = await scrypto.importKey(kExportFormat, signed_export.pub, kEcdhAlgo,
                                              /*extractable=*/false, []);
    const eph_pk = await scrypto.importKey(kExportFormat, ephemeral_export, kEcdhAlgo,
                                           /*extractable=*/false, []);
    // If one_time_export is not specified, use null.
    const ot_pk = one_time_export 
      ? await scrypto.importKey(kExportFormat, one_time_export, kEcdhAlgo, /*extractable=*/false, [])
      : null;

    return {
      id_pks: { dsa: id_dsa, dh: id_dh },
      signed_pk,
      eph_pk,
      ot_pk,
    };
  }

  // PRIVATE IMPL METHODS.
  
  // This function takes a key pair, a wrapping key, and a wrapping auth
  // and returns an object with two wrapped key objects:
  //   pub: {
  //     data: wrapped public key,
  //     ... and other props.
  //   }
  //   priv: {
  //     data: wrapped public key,
  //     ... and other props.
  //   }
  static async WrapKeyPair(key_pair, wrap_key, wrap_auth) {
    const pub = await this.WrapKey(key_pair.publicKey, wrap_key, wrap_auth);
    const priv = await this.WrapKey(key_pair.privateKey, wrap_key, wrap_auth);

    return { pub, priv };
  }
  // This function takes output from the above, the wrapping key, and wrapping auth,
  // and returns the key pair that was wrapped:
  //   publicKey: the unwrapped public key.
  //   privateKey: the unwrapped private key.
  static async UnwrapKeyPair(wrapped, wrap_key, wrap_auth) {
    const publicKey = await this.UnwrapKey(wrapped.pub, wrap_key, wrap_auth);
    const privateKey = await this.UnwrapKey(wrapped.priv, wrap_key, wrap_auth);

    return { publicKey, privateKey };
  }

  // This function takes a single key, a wrapping key, and a wrapping auth,
  // and returns a wrapped key object:
  //   data: wrapped key,
  //   iv: iv used to wrap key
  //   import_params: parameters needed to unwrap this key
  //   extractable: if this key was extractable
  //   usages: list of usages for this key.
  static async WrapKey(key, wrap_key, wrap_auth) {
    const iv = GenerateIv(wrap_key.algorithm.name);  // Gen IV based on wrapping key.
    // Some properties to be assigned based on the key we are wrapping.
    const import_params = ImportParams(key.algorithm.name);
    const extractable = key.extractable;
    const usages = key.usages;
    // Now wrap key using wrap_key and appropriate wrapping parameters.
    const data = await scrypto.wrapKey(kWrapFormat, key, wrap_key, WrapParams(wrap_key, iv, wrap_auth));

    return { data, iv, import_params, extractable, usages };
  }
  // This function takes output from the above, the wrapping key, and wrapping auth,
  // and returns the key that was wrapped.
  static async UnwrapKey(wrapped, wrap_key, wrap_auth) {
    return await scrypto.unwrapKey(kWrapFormat, wrapped.data, wrap_key, WrapParams(wrap_key, wrapped.iv, wrap_auth),
                                   wrapped.import_params, wrapped.extractable, wrapped.usages);
  }

};
