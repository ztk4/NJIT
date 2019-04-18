// Exports a class wrapping the IndexDB storage layer for keys and messages.

// IndexDB promise API.
import { openDB, deleteDB } from 'https://unpkg.com/idb?module'

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

  // TODO: Storage for messages and session keys.
};

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
  async UserExists() {
    return (await this.db.count('keyval', 'id-key')) == 1;
  }

  // Public key getters/setters.
  // Setters return the records that they set.

  async GetIdentityPk() {
    // Will fail if no such key exists.
    return await this.db.get('keyval', 'id-key');
  }
  async SetIdentityPk(id_key) {
    // Will fail if key already exists.
    await this.db.add('keyval', id_key, 'id-key');
    return await this.GetIdentityPk();
  }

  async GetSignedPk() {
    return await this.db.get('keyval', 'signed-key');
  }
  async SetSignedPk(signed_key) {
    await this.db.put('keyval', signed_key, 'signed-key');
    return await this.GetSignedPk();
  }

  // Retrieves a single one-time-key from storage, and removes it.
  async PopOneTimeKey(key_idx) {
    const tx = this.db.transaction('one-time-keys', 'readwrite');
    let key = await tx.store.get(key_idx);
    tx.store.delete(key_idx);

    // Make sure transaction completes successfully.
    await tx.done;

    return key;
  }
  // Adds a collection of one-time-keys to storage, and returns the stored records.
  // NOTE: Stored records will have an added .id property.
  async ReplenishOneTimeKeys(keys) {
    const tx = this.db.transaction('one-time-keys', 'readwrite');
    const max_old_idx = Math.max(... await tx.store.getAllKeys());
    // Insert each key into db (order doesn't matter, so in parallel).
    await Promise.all(keys.map( key => tx.store.add(key) ));

    let key_records = await tx.store.getAll(IDBKeyRange.lowerBound(max_old_idx, true));
    await tx.done;

    return key_records;
  }

  // Deletes all of a users's stored data.
  async DeleteAll() {
    // Close connection to allow deletion of database.
    await this.db.close();

    await deleteDB(this.db_name);
  }
};
