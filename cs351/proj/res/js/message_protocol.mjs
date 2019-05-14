// This file defines the message protocol used by this application.
// The protocol is loosely based on Signal.

// Storage API for persistent storage.
import Storage from '/web/res/js/storage.mjs'
// Key management API, handles all crypto work.
import KeyManager from '/web/res/js/key_manager.mjs'
// Utilities for sending/receiving web requests.
import { GetJson, PostJson, AbToHex, HexToAb } from '/web/res/js/util.mjs'

// Some helpers for encoding/decoding exported keys.
function EncodeKeyExport(key_export) {
  return AbToHex(key_export);
}
function EncodeKeyExportObject(object) {
  return Object.fromEntries(Object.entries(object).map(
    ([key, val]) => [key, EncodeKeyExport(val)]
  ));
}
function EncodeKeyExportArray(array) {
  return array.map( val => EncodeKeyExport(val) );
}

function DecodeKeyExport(encoded) {
  return HexToAb(encoded);
}
function DecodeKeyExportObject(object) {
  return Object.fromEntries(Object.entries(object).map(
    ([key, val]) => [key, DecodeKeyExport(val)]
  ));
}
function DecodeKeyExportArray(array) {
  return array.map( val => DecodeKeyExport(val) );
}

// Helper method that creates a record for a wrapped key.
// Just places the key in an object where additional top-level fields can be added.
function MakeKeyRecord(wrapped_key) {
  return { key: wrapped_key };
}
// Gets a key from records like the above.
function GetWrappedKeyFromRecord(record) {
  return record.key;
}

export default class MessageProtocol {
  // Returns an initialized protocol instance from this users's username and their password.
  static async MakeInitialized(uname, password) {
    // First open a stoage connection for uname.
    const storage = await Storage.Open(uname);
    // Assert this user exists.
    if (!(await storage.UserExists())) {
      storage.Close();
      throw new Error('Tried to initialize a non-existent user');
    }

    // Then get the salt for creating the wrapping key from storage.
    const wrap_salt = await storage.GetWrappingSalt();
    // Now create the wrapping key and auth.
    const wrap = await KeyManager.MakeWrappingKeyAndAuth(password, wrap_salt);
    
    // Finally, read the ID keys from storage and export them.
    const id_keypairs = await storage.GetIdentityPk();
    const id_export = await KeyManager.ExportIdentityKeys(id_keypairs);

    return MessageProtocol(uname, { storage, wrap, id_export });
  }

  // Helper for creating a new user, returns true on success.
  // NOTE: If this method returns true, then the same uname and password can be
  //       passed to MakeInitialized above to create a MessageProtocol instance
  //       for this new user.
  static async CreateNewUser(uname, password) {
    // First, let's open a storage connection for this user.
    const storage = await Storage.Open(uname);
    // Assert this user doesn't already exist.
    if (await storage.UserExists()) {
      storage.Close();
      throw new Error('Cannot create a new user by a taken user name');
    }

    // Try to create user below.
    try {
      // Now let's generate their initial set of public keys.
      const [id_keypairs, signed_keypair, ot_keypairs] = await Promise.all([
        KeyManager.MakeIdentityKeyPairs(),
        KeyManager.MakeSignedKeyPair(),
        KeyManager.MakeOneTimeKeyPairs(10),
      ]);
      // And their wrapping salt.
      const wrap_salt = await KeyManager.GenerateSalt(16);
      
      // We also want to store the above salt, and derive wrapping constants from it and password.
      const [ salt_record, wrap ] = await Promise.all([
        storage.SetWrappingSalt(wrap_salt),
        KeyManager.MakeWrappingKeyAndAuth(password, wrap_salt),
      ]);

      // Let's wrap these keys and store them.
      // This will also generate IDs for the OT keys.
      const [ id_wrap, signed_wrap, ot_wraps ] = await Promise.all([
        KeyManager.WrapIdentityKeyPairs(id_keypairs, wrap.key, wrap.auth),
        KeyManager.WrapSignedKeyPair(signed_keypair, wrap.key, wrap.auth),
        Promise.all(ot_keypairs.map( ot_keypair => KeyManager.WrapOneTimeKeyPair(ot_keypair, wrap.key, wrap.auth) )),
      ]);
      const [ id_record, signed_record, ot_records ] = await Promise.all([
        storage.SetIdentityPk(MakeKeyRecord(id_keypairs)),
        storage.SetSignedPk(MakeKeyRecord(signed_keypair)),
        storage.ReplenishOneTimeKeys(ot_wraps.map( ot_wrap => MakeKeyRecord(ot_wrap) )),
      ]);

      // Let's export these keypairs and try to advertise these to the server as a new user.
      const [ id_export, signed_export, ot_exports ] = await Promise.all([
        KeyManager.ExportIdentityKeys(id_keypairs),
        KeyManager.ExportSignedKey(signed_keypair, id_keypairs),
        Promise.all(ot_keypairs.map( ot_keypair => KeyManager.ExportOneTimeKey(ot_keypair) )),
      ]);
      // Need indeces of the ot_keypairs.
      const ot_idx = ot_records.map( ot_record => ot_record.id );
      // Now advertise to server.
      await this.Advertise(uname, { id_export, signed_export, ot_exports }, { ot_idx });

      // No more storage stuff to do.
      await storage.Close();
    } catch (err) {
      console.error(err, err.message);

      // On error, destory anything we stored for this user.
      // NOTE: Our storage handle must be closed for this.
      await storage.Close();
      await this.DeleteUser(uname);

      return false;
    }

    return true;
  }

  // Creates a message protocol instance from a user name, and an init object.
  // The init object must contain:
  //   storage: a Storage instance for uname.
  //   wrap: { key: wrapping key, auth: wrapping auth } (used when interacting with storage),
  //   id_export: the exported identity PKs (used for sending / receiving ALL messages).
  constructor(uname, { storage, wrap, id_export }) {
    // Store the passed parameters.
    this.uname = uname;
    this.storage = storage;
    this.wrap = wrap;
    this.id_export = id_export;
  }

  // This delete all local storage associated with a user (including all keys).
  // Not really a supported action from an API perspective, but useful for tests and cleanup.
  static async DeleteUser(uname) {
    const storage = await Storage.Open(uname);
    return await storage.DeleteAll();
  }

  // PRIVATE IMPL FOLLOWS.
  
  // Advertises a user name associated with a collection of exported PKs to the server.
  // Currently, it is ecpted that:
  //   exports: {
  //     id_export: exported id pks,
  //     signed_export: exported signed pk,
  //     ot_exports: exported ot pks,
  //   },
  //   meta: {
  //     ot_idx: indeces of the ot pks in ot_exports.
  //   }
  static async Advertise(uname, { id_export, signed_export, ot_exports }, meta) {
    // Before sending, we need to encode any exported keys.
    const id_encoded = EncodeKeyExportObject(id_export);
    const signed_encoded = EncodeKeyExportObject(signed_export);
    const ot_encoded = EncodeKeyExportArray(ot_exports);

    return await PostJson('/api/user/new', {
      uname: uname,
      exports: { id_encoded, signed_encoded, ot_encoded },
      meta: meta
    });
  }
};
