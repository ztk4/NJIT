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
import { StrToAb, AbToStr, AbConcat } from '/web/res/js/util.mjs'

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
// This is the HMAC algorithm decription.
const kHmacAlgo = {
  name: 'HMAC',
  hash: 'SHA-256',
  length: 32 * 8,  // Key length in bits.
};
// These are the signing params for ECDSA signing.
const kEcdsaSigParams = {
  name: 'ECDSA',
  hash: 'SHA-512',
};

// The max number of ECDH bits that can be derived from P-521.
// NOTE: Took this from an error message, but I assume it's just 521 bits padded?
const kMaxEcdhBits = 528;

// The export format to use when exporting keys.
const kExportFormat = 'raw';
// The export format to use when wrapping keys.
const kWrapFormat = 'jwk';

// Number of iterations to use for Pbkdf2.
const kPbkdf2Iterations = 100000;  // Takes roughly 100ms on my laptop.
// Constant info strings to use for HKDF.
const kHkdfInitInfo = StrToAb('e2ee-init');
const kHkdfRatchetInfo = StrToAb('e2ee-ratchet');
const kHkdfMessageInfo = StrToAb('e2ee-message');
// Inputs for HMAC used for the symmetric key ratchet.
const kMessageKeyInput = StrToAb('\x01');
const kChainKeyInput = StrToAb('\x02');

// Gets the derivation parameters for a Pbkdf2 key,
// using a salt (of at least 16 bytes).
function Pbkdf2DeriveParams(salt) {
  return {
    name: 'PBKDF2',
    salt: salt,
    iterations: kPbkdf2Iterations,
    hash: { name: 'SHA-512' },
  };
}

// Gets the derivation parameters for an ECDH key exchange,
// using the other parties public key.
function EcdhDeriveParams(other_pk) {
  return {
    name: 'ECDH',
    public: other_pk,
  };
}

// Gets the derivation parameters for a HKDF key.
function HkdfDeriveParams(salt, info) {
  return {
    name: 'HKDF',
    hash: 'SHA-512',
    salt: salt,
    info: info,
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

// Gets encryption params for an aes key.
function AesParams(aes_key, iv, auth) {
  let params = {
    name: aes_key.algorithm.name,
  };

  switch (params.name) {
   case 'AES-GCM':
    // Only set GCM auth data if it is supplied.
    if (auth) {
      params.additionalData = auth;
    }
    // fallthrough
   case 'AES-CBC':
    params.iv = iv;
    break;

   default:
    // There are other valid aes key types,
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
// As a general rule, the Make* methods make keys/keypairs,
// the Export* methods export key pairs in an unencrypted format (sometimes with signatures),
// the Import* methods import key pairs from an unencrypted format (verifying any signatures),
// the Wrap* methods export and encrypt keys using a password derived wrapping key (for local storage),
// the Unwrap* methods unwrap keys using the same password derived wrapping key.
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
    const pass_key = await scrypto.importKey('raw', StrToAb(password), 'PBKDF2',
                                             /*extractable=*/false, ['deriveBits']);
    // Now, we'll derive 512 bits:
    //   256 for a 256-bit AES key,
    //   256 for auth data.
    const derived = await scrypto.deriveBits(Pbkdf2DeriveParams(salt), pass_key, 512);

    // We then use some of the above bits to create an AES-GCM key.
    const key = await scrypto.importKey('raw', new DataView(derived, 0, 32), 'AES-GCM',
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
  static async ExportIdentityKeys(id_keypairs) {
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
  static async ExportSignedKey(signed_keypair, id_keypairs) {
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
  static async ExportEphemeralKey(ephemeral_keypair) {
    return await scrypto.exportKey(kExportFormat, ephemeral_keypair.publicKey);
  }
  // Wraps the ephemeral public key, returning the wrapped key.
  // NOTE: Only public key of ephemeral pair ever needs to be stored.
  static async WrapEphemeralPublicKey(ephemeral_keypair, wrap_key, wrap_auth) {
    return await this.WrapKey(ephemeral_keypair.publicKey, wrap_key, wrap_auth);
  }
  // Unwraps public key wrapped above, returning the key.
  static async UnwrapEphemeralPublicKey(wrapped, wrap_key, wrap_auth) {
    return await this.UnwrapKey(wrapped, wrap_key, wrap_auth);
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
  //   signed_pk: public signed key, or null if an exported signed key was not passed.
  //   ot_pk: one-time public key, or null if an exported one-time key was not passed.
  static async ImportPublicKeys(id_export, signed_export, one_time_export) {
    // First get the dsa key for verifying signatures.
    const id_dsa = await scrypto.importKey(kExportFormat, id_export.dsa_pub, kEcdsaAlgo,
                                           /*extractable=*/false, ['verify']);
    
    // Now verify signatures of dh and signed key before importing others.
    const dh_verify = await scrypto.verify(kEcdsaSigParams, id_dsa, id_export.dh_sig, id_export.dh_pub);
    if (!dh_verify) {
      throw new Error('Unable to verify signature on public ECDH identity key');
    }
    // Only verified signed key if supplied.
    if (signed_export) {
      const signed_verify =
        await scrypto.verify(kEcdsaSigParams, id_dsa, signed_export.sig, signed_export.pub);
      if (!signed_verify) {
        throw new Error('Unable to verify signature on public ECDH signed key');
      }
    }

    // Now that we've verified, import all other keys.
    // NOTE: An ECDH public key does NOT need usage permissions for key derivation.
    const id_dh = await scrypto.importKey(kExportFormat, id_export.dh_pub, kEcdhAlgo,
                                          /*extractable=*/false, []);
    const signed_pk = signed_export
      ? await scrypto.importKey(kExportFormat, signed_export.pub, kEcdhAlgo, /*extractable=*/false, [])
      : null;
    const ot_pk = one_time_export 
      ? await scrypto.importKey(kExportFormat, one_time_export, kEcdhAlgo, /*extractable=*/false, [])
      : null;

    return {
      id_pks: { dsa: id_dsa, dh: id_dh },
      signed_pk,
      ot_pk,
    };
  }

  // This method imports a public ephemeral key from the export method above.
  // Typically, this is used to import another person's ephemeral key in the initiation
  // or ratchet steps of the protocol.
  // NOTE: Imported key will NOT be extractable (shouldn't need to re-export them).
  // Returns just the imported ephemeral public key.
  static async ImportEphemeralKey(eph_export) {
    return await scrypto.importKey(kExportFormat, eph_export, kEcdhAlgo, /*extractable=*/false, []);
  }

  // Derives the initial root and chain keys for the initiator of a new session.
  // Takes this users's id keys and ephemeral keys,
  // and the recipients id pks, signed pks, and one-time pks.
  // NOTE: The one-time key argument is optional.
  // Returns:
  //   root_key: a root symmetric key (an array buffer of bytes),
  //   chain_key: a chain symmetric key (an array buffer of butes).
  static async DeriveInitiatorSessionKeys(id_keypairs, eph_keypair,
                                          recip_id_pks, recip_signed_pk, recip_ot_pk) {
    // Derive the parts of the master secret as needed.
    const init_id_recip_signed_secret = await scrypto.deriveBits(EcdhDeriveParams(recip_signed_pk),
                                                                 id_keypairs.dh.privateKey,
                                                                 kMaxEcdhBits);
    const init_eph_recip_id_secret = await scrypto.deriveBits(EcdhDeriveParams(recip_id_pks.dh),
                                                              eph_keypair.privateKey,
                                                              kMaxEcdhBits);
    const init_eph_recip_signed_secret = await scrypto.deriveBits(EcdhDeriveParams(recip_signed_pk),
                                                                  eph_keypair.privateKey,
                                                                  kMaxEcdhBits);
    // Create a master secret via concatentation,
    // optionally including a one-time key secret if possible.
    let master_secret;
    if (recip_ot_pk) {
      const init_eph_recip_ot_secret = await scrypto.deriveBits(EcdhDeriveParams(recip_ot_pk),
                                                                eph_keypair.privateKey,
                                                                kMaxEcdhBits);
      master_secret = AbConcat(init_id_recip_signed_secret, init_eph_recip_id_secret,
                               init_eph_recip_signed_secret, init_eph_recip_ot_secret);
    } else {
      master_secret = AbConcat(init_id_recip_signed_secret, init_eph_recip_id_secret,
                               init_eph_recip_signed_secret);
    }

    // Make an HKDF key from the master secret.
    const master_key = await scrypto.importKey('raw', master_secret, 'HKDF',
                                               /*extractable=*/false, ['deriveBits']);
    // And finally derive 64 bytes from the key, half for the root key, half for the chain key.
    // NOTE: Salt is all 0's and same length as output.
    const derived = await scrypto.deriveBits(HkdfDeriveParams(/*salt=*/new ArrayBuffer(64), kHkdfInitInfo),
                                             master_key, 64 * 8);

    // Now split into root and chain keys, and return.
    const root_key = derived.slice(0, 32);
    const chain_key = derived.slice(32);

    return { root_key, chain_key };
  }
  // Derives the initial root and chain keys for the recipient of a new session.
  // Takes this user's id keys, signed keys, and one-time keys,
  // and the initiator's id pks and ephemeral pk.
  // NOTE: The one-time key argument is optional.
  // Returns:
  //   root_key: a root symmetric key (an array buffer of bytes),
  //   chain_key: a chain symmetric key (an array buffer of bytes).
  static async DeriveRecipientSessionKeys(init_id_pks, init_eph_pk,
                                          id_keypairs, signed_keypair, ot_keypair) {
    // Derive the parts of the master secret as needed.
    const init_id_recip_signed_secret = await scrypto.deriveBits(EcdhDeriveParams(init_id_pks.dh),
                                                                 signed_keypair.privateKey,
                                                                 kMaxEcdhBits);
    const init_eph_recip_id_secret = await scrypto.deriveBits(EcdhDeriveParams(init_eph_pk),
                                                              id_keypairs.dh.privateKey,
                                                              kMaxEcdhBits);
    const init_eph_recip_signed_secret = await scrypto.deriveBits(EcdhDeriveParams(init_eph_pk),
                                                                  signed_keypair.privateKey,
                                                                  kMaxEcdhBits);

    // Create a master secret via concatentation,
    // optionally including a one-time key secret if possible.
    let master_secret;
    if (ot_keypair) {
      const init_eph_recip_ot_secret = await scrypto.deriveBits(EcdhDeriveParams(init_eph_pk),
                                                                ot_keypair.privateKey,
                                                                kMaxEcdhBits);
      master_secret = AbConcat(init_id_recip_signed_secret, init_eph_recip_id_secret,
                               init_eph_recip_signed_secret, init_eph_recip_ot_secret);
    } else {
      master_secret = AbConcat(init_id_recip_signed_secret, init_eph_recip_id_secret,
                               init_eph_recip_signed_secret);
    }

    // Make an HKDF key from the master secret.
    const master_key = await scrypto.importKey('raw', master_secret, 'HKDF',
                                               /*extractable=*/false, ['deriveBits']);
    // And finally derive 64 bytes from the key, half for the root key, half for the chain key.
    // NOTE: Salt is all 0's and same length as output.
    const derived = await scrypto.deriveBits(HkdfDeriveParams(/*salt=*/new ArrayBuffer(64), kHkdfInitInfo),
                                             master_key, 64 * 8);

    // Now split up and return.
    const root_key = derived.slice(0, 32);
    const chain_key = derived.slice(32);

    return { root_key, chain_key };
  }

  // Takes the previous root key, and ephemeral keys of this user, and the other party,
  // and ratchets the root key and chain key forward.
  // Returns:
  //   root_key: a root symmetric key (an array buffer of bytes),
  //   chain_key: a chain symmetric key (an array buffer of bytes).
  static async DhRatchet(prev_root_key, other_eph_pk, eph_keypair) {
    // Derive the shared DH secret.
    const shared_secret =
      await scrypto.deriveBits(EcdhDeriveParams(other_eph_pk), eph_keypair.privateKey, kMaxEcdhBits);
    // Use the above secret as IKM for an HKDF key.
    const shared_key = await scrypto.importKey('raw', shared_secret, 'HKDF',
                                               /*extractable=*/false, ['deriveBits']);
    // And finally, derive 64 bytes from the key, half for new root key, half for new chain key.
    // NOTE: This time, salt is the previous root key.
    const derived = await scrypto.deriveBits(HkdfDeriveParams(prev_root_key, kHkdfRatchetInfo),
                                             shared_key, 64 * 8);

    // Split up and return.
    const root_key = derived.slice(0, 32);
    const chain_key = derived.slice(32);

    return { root_key, chain_key };
  }

  // Takes the current chain key, derives a message key, and ratchets to the next chain key.
  // Returns:
  //   message_key: a message symmetrict key (an array buffer of bytes),
  //   chain_key: a chain symmetric key (an array buffer of bytes).
  static async SymmetricRatchet(prev_chain_key) {
    // Create an HMAC key from prev_chain_key.
    const hmac_key = await scrypto.importKey('raw', prev_chain_key, kHmacAlgo,
                                             /*extractable=*/false, ['sign']);
    // Although we are using HMAC for key generation,
    // webcrypto thinks of it only as a signing algorithm.
    // We will essentially just use the resulting MACs as keys.
    const message_prekey = await scrypto.sign('HMAC', hmac_key, kMessageKeyInput);
    const chain_key = await scrypto.sign('HMAC', hmac_key, kChainKeyInput);

    // The above message prekey is only 32-bytes,
    // where we'd like 80-bytes for encryptiong and authentication altogether.
    // Hence the use of HKDF below to extract 80-bytes from the message prekey.
    const hkdf_key = await scrypto.importKey('raw', message_prekey, 'HKDF',
                                             /*extractable=*/false, ['deriveBits']);
    // Using 0-filled salt of equal size to output.
    const message_key = await scrypto.deriveBits(HkdfDeriveParams(/*salt=*/new ArrayBuffer(80), kHkdfMessageInfo),
                                                 hkdf_key, 80 * 8);

    return { message_key, chain_key };
  }

  // Takes a message text, message key, and the exported ID pks of both parties,
  // and encrypts + authenticates the message.
  // Returns:
  //   cipher_text: encrypted message value
  //   mac: a MAC created from the pks concatenated with cipher_text.
  static async EncryptAndAuthorizeMessage(text, key, id_export, other_id_export) {
    // First, we split the message key into it's parts.
    const aes_key = key.slice(0, 32);
    const auth_key = key.slice(32, 64);
    const aes_iv = key.slice(64);

    // Now we import aes_key as a key object.
    const enc_key = await scrypto.importKey('raw', aes_key, 'AES-CBC',
                                            /*extractable=*/false, ['encrypt']);
    // And then encrypt the message text.
    const cipher_text = await scrypto.encrypt(AesParams(enc_key, aes_iv), enc_key, StrToAb(text));

    // Now, we want to use HMAC to sign the above cipher_text,
    // but we will first prepend PKs for authentication.
    const signing_data = AbConcat(id_export.dsa_pub, id_export.dh_pub,
                                  other_id_export.dsa_pub, other_id_export.dh_pub,
                                  cipher_text);
    const signing_key = await scrypto.importKey('raw', auth_key, kHmacAlgo,
                                                /*extractable=*/false, ['sign']);
    const mac = await scrypto.sign('HMAC', signing_key, signing_data);

    // NOTE: The keys prepended when generated the signature are NOT sent along with the message.
    //       Instead, the other party must be able to reproduce these values independently.
    return { cipher_text, mac };
  }

  // Takes a cipher text, MAC, message key, and the exported ID pks of both parties,
  // and verifies and decrypts the message.
  // Returns the plaintext message.
  static async VerifyAndDecryptMessage(cipher_text, mac, key, id_export, other_id_export) {
    // First, we split the message key into it's parts.
    const aes_key = key.slice(0, 32);
    const auth_key = key.slice(32, 64);
    const aes_iv = key.slice(64);

    // Now we want to verify the MAC before trying to decrypt the message.
    // NOTE: The key order puts our keys SECOND (since the sender put their keys first).
    const signed_data = AbConcat(other_id_export.dsa_pub, other_id_export.dh_pub,
                                 id_export.dsa_pub, id_export.dh_pub,
                                 cipher_text);
    const verify_key = await scrypto.importKey('raw', auth_key, kHmacAlgo,
                                               /*extractable=*/false, ['verify']);
    const verified = await scrypto.verify('HMAC', verify_key, mac, signed_data);

    if (!verified) {
      throw new Error('Unable to verify message signature');
    }

    // Now that we've verified the signature, let's decrypt.
    const dec_key = await scrypto.importKey('raw', aes_key, 'AES-CBC',
                                            /*extractable=*/false, ['decrypt']);
    
    return AbToStr(await scrypto.decrypt(AesParams(dec_key, aes_iv), dec_key, cipher_text));
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
    const data = await scrypto.wrapKey(kWrapFormat, key, wrap_key, AesParams(wrap_key, iv, wrap_auth));

    return { data, iv, import_params, extractable, usages };
  }
  // This function takes output from the above, the wrapping key, and wrapping auth,
  // and returns the key that was wrapped.
  static async UnwrapKey(wrapped, wrap_key, wrap_auth) {
    return await scrypto.unwrapKey(kWrapFormat, wrapped.data, wrap_key, AesParams(wrap_key, wrapped.iv, wrap_auth),
                                   wrapped.import_params, wrapped.extractable, wrapped.usages);
  }

};
