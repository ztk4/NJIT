// Exports a class wrapping the IndexDB storage layer for keys and messages.

// IndexDB promise API.
import { openDB, deleteDB } from 'https://unpkg.com/idb/with-async-ittr?module'

// Some utilities.
import { kMinDate, kMaxDate } from '/web/res/js/util.mjs'

// Prefix for database names.
const db_prefix = 'e2ee-';

// Used to initialize storage in our DB.
function InitDb(db) {
  // Creates a generic object store for (key, value) pairs.
  // Great for stoage of single objects between sessions.
  db.createObjectStore('keyval');

  // Creates storage for one-time-keys.
  db.createObjectStore('one-time-keys', {
    // Use the ID property of stored objects as their key.
    keyPath: 'id',
    // If not already set, automatically set via autoincrement.
    autoIncrement: true,
  });

  // Storage for recipients this user has talked to.
  const recipient_store = db.createObjectStore('recipients', {
    // Use the ID property of stored recipients as their key.
    keyPath: 'id',
    // Autoincrement ids if one was not supplied.
    autoIncrement: true,
  });
  // We want to be able to find a recipient by their username.
  recipient_store.createIndex(/*name=*/'uname', /*keyPath=*/'uname', {
    // Usernames should be unique.
    unique: true,
  });
  // We want to be able to find most recently messaged users.
  // NOTE: This datetime is NOT necessarily unique.
  recipient_store.createIndex(/*name=*/'last-message', /*keyPath=*/'last_message_datetime');

  // Storage for chat root keys.
  db.createObjectStore('root-keys', {
    // We have a chain of root keys per recipient.
    // Therefore our key is the tuple (recip_id, id).
    keyPath: ['recip_id', 'id'],
    // NOTE: Autoincrement does not support multipart keys, therefore
    //       we have to increment id ourselves.
  });

  // Storage for chat chain and message keys.
  db.createObjectStore('message-chain-keys', {
    // We have a chain of chain keys per root key.
    // Therefore our key should identify a root key, and then have another id.
    keyPath: ['recip_id', 'root_id', 'id'],
    // NOTE: Again, no autoincrement.
  });

  // Storage for messages.
  const message_store = db.createObjectStore('messages', {
    // We have many messages per recipient. We don't actually care about
    // maintaing which messages had which key or keeping a consistent id scheme
    // on messages, so we let their primary key be autoIncrementing.
    keyPath: 'id',
    autoIncrement: true,
  });
  // We want to be able to find messages by recipient and recency.
  // NOTE: The datetime is NOT necessarily unique.
  message_store.createIndex(/*name=*/'recip-and-date', /*keyPath=*/['recip_id', 'datetime']);
};

// Checks if a transaction supports a mode.
function TxSupportsMode(tx, mode) {
  let supported = false;
  switch (mode) {
   case 'readonly':  // readonly or readwrite works.
    supported |= (tx.mode === 'readonly');
    // fallthrough
   case 'readwrite':  // only readwrite works.
    supported |= (tx.mode === 'readwrite');
    break;

   case 'versionchange':  // only versionchange works.
    supported |= (tx.mode === 'versionchange');
  }

  return supported;
}

// Checks if a transaction can query a list of object stores.
function TxContainsStores(tx, stores) {
  // Only true if tx contains each store in stores.
  // NOTE: tx.objectStoreNames is NOT an Array, so has no includes method.
  //       It is a DOMStringList, which has a similar contains method.
  return stores.every( store => tx.objectStoreNames.contains(store) );
}

// In the class below, all methods that query the underlying storage take an optional
// parameter tx. If specified, this should be a transaction to the underlying this.db
// and should have the proper mode and storages.
export default class Storage {
  // Opens a connection to the specified user's storage.
  static async Open(uname) {
    const db_name = db_prefix + uname;
    return new Storage(db_name, await openDB(db_name, 1, {
      // Upgrade for us means no pre-existing storage, so initialize.
      upgrade(db) { InitDb(db); },
      blocked() { console.log('blocked'); },
      blocking() { console.log('blocking'); },
    }));
  }

  // Useful to have the database name and handle around.
  constructor(db_name, db) {
    // Store name.
    this.db_name = db_name;
    // IndexDB database object.
    this.db = db;
  }

  // Checks if the user exists or not by checking for existence of 
  // an identity key pair in storage.
  async UserExists(tx) {
    tx = this.ValidateOrDefault(tx, 'keyval', 'readonly');
    return (await tx.objectStore('keyval').count('id-key')) == 1;
  }

  // Public key getters/setters.
  // Setters return the records that they set.

  async GetIdentityPk(tx) {
    tx = this.ValidateOrDefault(tx, 'keyval', 'readonly');
    // Will fail if no such key exists.
    return await tx.objectStore('keyval').get('id-key');
  }
  async SetIdentityPk(id_key, tx) {
    tx = this.ValidateOrDefault(tx, 'keyval', 'readwrite');
    // Will fail if key already exists.
    await tx.objectStore('keyval').add(id_key, 'id-key');
    return await this.GetIdentityPk(tx);
  }

  async GetSignedPk(tx) {
    tx = this.ValidateOrDefault(tx, 'keyval', 'readonly');
    return await tx.objectStore('keyval').get('signed-key');
  }
  async SetSignedPk(signed_key, tx) {
    tx = this.ValidateOrDefault(tx, 'keyval', 'readwrite');
    await tx.objectStore('keyval').put(signed_key, 'signed-key');
    return await this.GetSignedPk(tx);
  }

  // Retrieves a single one-time-key from storage, and removes it.
  async PopOneTimeKey(key_idx, tx) {
    tx = this.ValidateOrDefault(tx, 'one-time-keys', 'readwrite');
    let otk_store = tx.objectStore('one-time-keys');
    let key = await otk_store.get(key_idx);
    await otk_store.delete(key_idx);

    return key;
  }
  // Adds a collection of one-time-keys to storage, and returns the stored records.
  // NOTE: Stored records will have an added .id property.
  async ReplenishOneTimeKeys(keys, tx) {
    tx = this.ValidateOrDefault(tx, 'one-time-keys', 'readwrite');
    let otk_store = tx.objectStore('one-time-keys');
    // Insert each key into db (order doesn't matter, so in parallel).
    const store_keys = await Promise.all(keys.map( key => otk_store.add(key) ));

    // Get all inserted records.
    return await Promise.all(store_keys.map( id => otk_store.get(id) ));
  }

  // Get a particular root key from store.
  async GetRootKey(recip_uname, root_id, tx) {
    tx = this.ValidateOrDefault(tx, ['recipients', 'root-keys'], 'readonly');
    // Look up the recipient by name.
    const recip_id = (await this.GetRecipient(recip_uname, tx)).id;
    // Now get the key.
    return await tx.objectStore('root-keys').get([recip_id, root_id]);
  }
  // Add new root key to store.
  // NB: root_key.id MUST be set to a unique value for this recipient.
  async AddRootKey(recip_uname, root_key, tx) {
    tx = this.ValidateOrDefault(tx, ['recipients', 'root-keys'], 'readwrite');
    // Write the recip_id to root_key's keypath.
    root_key.recip_id = (await this.GetRecipient(recip_uname, tx)).id;
    const store_key = await tx.objectStore('root-keys').add(root_key);

    return await this.GetRootKey(recip_uname, store_key[1], tx);
  }
  // Removes a root key from storage.
  async DeleteRootKey(recip_uname, root_id, tx) {
    tx = this.ValidateOrDefault(tx, ['recipients', 'root-keys'], 'readwrite');
    // Look up recipient by name.
    const recip_id = (await this.GetRecipient(recip_uname, tx)).id;
    return await tx.objectStore('root-keys').delete([recip_id, root_id]);
  }

  // Gets a message chain key by recipient, root_id, and chain_id.
  async GetMessageChainKey(recip_uname, root_id, chain_id, tx) {
    tx = this.ValidateOrDefault(tx, ['recipients', 'message-chain-keys'], 'readonly');
    // Lookup recipients by name.
    const recip_id = (await this.GetRecipient(recip_uname, tx)).id;
    return await tx.objectStore('message-chain-keys').get([recip_id, root_id, chain_id]);
  }
  // Add new message chain keys, all for the same root key.
  // NB: For each key, [key.root_id, key.id] MUST be unique for this recipient.
  async AddMessageChainKeys(recip_uname, keys, tx) {
    tx = this.ValidateOrDefault(tx, ['recipients', 'message-chain-keys'], 'readwrite');
    // Look up recipient by name.
    const recip_id = (await this.GetRecipient(recip_uname, tx)).id;
    // Add recip_id to each of the keys.
    for (let key of keys) {
      key.recip_id = recip_id;
    }

    // Now add the keys.
    const store_keys = await Promise.all(keys.map(
      key => tx.objectStore('message-chain-keys').add(key)
    ));
    
    // Finally, return all the new key records.
    return await Promise.all(store_keys.map(
      store_key => this.GetMessageChainKey(recip_uname, store_key[1], store_key[2], tx)
    ));
  }
  // Convenience method for the above when you only have one key to add.
  // NB: [key.root_id, key.id] MUST be unique for this recipient.
  async AddMessageChainKey(recip_uname, key, tx) {
    tx = this.ValidateOrDefault(tx, ['recipients', 'message-chain-keys'], 'readwrite');
    // Just wrap key in an array, and then unwrap the returned array.
    return (await this.AddMessageChainKeys(recip_uname, [key], tx))[0];
  }
  async DeleteMessageChainKey(recip_uname, root_id, chain_id, tx) {
    tx = this.ValidateOrDefault(tx, ['recipients', 'message-chain-keys'], 'readwrite');
    // Look up recipient id.
    const recip_id = (await this.GetRecipient(recip_uname, tx)).id;
    return await tx.objectStore('message-chain-keys').delete([recip_id, root_id, chain_id]);
  }

  // Retrieve a recipient from storage.
  async GetRecipient(recip_uname, tx) {
    tx = this.ValidateOrDefault(tx, 'recipients', 'readonly');
    return await tx.objectStore('recipients').index('uname').get(recip_uname);
  }
  // Add a new recipient to storage.
  async AddRecipient(recipient, tx) {
    tx = this.ValidateOrDefault(tx, 'recipients', 'readwrite');
    await tx.objectStore('recipients').add(recipient);

    return this.GetRecipient(recipient.uname, tx);
  }

  // Retrieves messages by recipient.
  // Messages are returned in reverse chronological order.
  // Before can be specified to only return messages before a given datetime,
  // but defaults to the maximum datetime.
  async GetMessages(recip_uname, count, before, tx) {
    tx = this.ValidateOrDefault(tx, ['recipients', 'messages'], 'readonly');
    // Get recipient id.
    const recip_id = (await this.GetRecipient(recip_uname, tx)).id;
    // Build a key range based on recip_id and before.
    const range = IDBKeyRange.bound([recip_id, kMinDate], [recip_id, before || kMaxDate],
                                    /*lowerOpen=*/false, /*upperOpen=*/true);

    // Return the first count entries from the above range.
    // NOTE: We are iterating backwards (prev) in order to achieve reverse chronological order.
    let messages = [];
    for await (const cursor of tx.objectStore('messages').index('recip-and-date').iterate(range, 'prev')) {
      messages.push(cursor.value);

      // Break early if enough messages have been read.
      if (messages.length >= count)
        break;
    }

    return messages;
  }
  // Adds a message to storage.
  async AddMessage(recip_uname, msg, tx) {
    tx = this.ValidateOrDefault(tx, ['recipients', 'messages'], 'readwrite');
    // Get recipient id.
    msg.recip_id = (await this.GetRecipient(recip_uname, tx)).id;
    const store_key = await tx.objectStore('messages').add(msg);

    // Get stored message.
    return await tx.objectStore('messages').get(store_key);
  }

  // Deletes all of a users's stored data.
  async DeleteAll() {
    // Close connection to allow deletion of database.
    await this.db.close();

    await deleteDB(this.db_name);
  }

  // PRIVATE IMPL METHODS:
  
  // Validates the passed transaction for the required objects stores and mode.
  // If valid, passed transaction is returned.
  // Otherwise, a default transaction satisfying the requirements is returned.
  ValidateOrDefault(tx, stores, mode) {
    // Handle the case when a single string is passed as stores.
    if (!Array.isArray(stores))
      stores = [stores];

    if (tx && tx.db == this.db && TxSupportsMode(tx, mode) &&
        TxContainsStores(tx, stores)) {
      return tx;
    }

    // If the tx was not null and was not valid, log.
    if (tx) {
      console.debug(`WARNING: Transaction (mode='${tx.mode}', stores=${tx.stores}) does not ` +
                    `support mode '${mode}' or stores ${stores}`);
    }

    return this.db.transaction(stores, mode);
  }
};
