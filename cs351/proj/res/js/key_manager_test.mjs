import KeyManager from '/web/res/js/key_manager.mjs'

async function stuff() {
  // Public Key Generation Tests.

  console.log('Generate ID key pairs');
  let id_keypairs = await KeyManager.MakeIdentityKeyPairs();
  console.log(id_keypairs);

  console.log('Generate Signed key pair');
  let signed_keypair = await KeyManager.MakeSignedKeyPair();
  console.log(signed_keypair);

  console.log('Generate Ephemeral key pair');
  let ephemeral_keypair = await KeyManager.MakeEphemeralKeyPair();
  console.log(ephemeral_keypair);

  console.log('Generate 10 One-Time key pairs');
  let one_time_keypairs = await KeyManager.MakeOneTimeKeyPairs(10);
  console.log(one_time_keypairs);

  // Public Key Export Tests.

  console.log('Export ID key pairs');
  let id_export = await KeyManager.ExportIdentityKeyPairs(id_keypairs);
  console.log(id_export);

  console.log('Export Signed key pair');
  let signed_export = await KeyManager.ExportSignedKeyPair(signed_keypair, id_keypairs);
  console.log(signed_export);

  console.log('Export Ephemeral key pair');
  let eph_export = await KeyManager.ExportEphemeralKeyPair(ephemeral_keypair);
  console.log(eph_export);

  console.log('Export a One-Time key pair');
  let ot_export = await KeyManager.ExportOneTimeKey(one_time_keypairs[2]);
  console.log(ot_export);

  // Public Key Import Tests.

  console.log('Partial key import');
  let imported_partial = await KeyManager.ImportPublicKeys(id_export, signed_export, eph_export);
  console.log(imported_partial);

  console.log('Full key import');
  let imported = await KeyManager.ImportPublicKeys(id_export, signed_export, eph_export, ot_export);
  console.log(imported);

  // Salt and Wrapping Key Gen Tests.
  
  console.log('Salt generation');
  let salt = KeyManager.GenerateSalt(16);
  console.log(salt);

  console.log('Wrapping key and auth generation');
  let wrap = await KeyManager.MakeWrappingKeyAndAuth('mypassword', salt);
  console.log(wrap);

  // Public Key Wrapping Tests.
  
  console.log('Wrap ID key pairs');
  let id_wrapped = await KeyManager.WrapIdentityKeyPairs(id_keypairs, wrap.key, wrap.auth);
  console.log(id_wrapped);

  console.log('Wrap Signed key pairs');
  let signed_wrapped = await KeyManager.WrapSignedKeyPair(signed_keypair, wrap.key, wrap.auth);
  console.log(signed_wrapped);

  console.log('Wrap Ephemeral key pair');
  let eph_wrapped = await KeyManager.WrapEphemeralKeyPair(ephemeral_keypair, wrap.key, wrap.auth);
  console.log(eph_wrapped);

  console.log('Wrap a One-Time key pair');
  let ot_wrapped = await KeyManager.WrapOneTimeKeyPair(one_time_keypairs[2], wrap.key, wrap.auth);
  console.log(ot_wrapped);

  // Public Key Unwrap Tests.

  console.log('Unwrap ID key pairs');
  console.log(await KeyManager.UnwrapIdentityKeyPairs(id_wrapped, wrap.key, wrap.auth));

  console.log('Unwrap Signed key pairs');
  console.log(await KeyManager.UnwrapSignedKeyPair(signed_wrapped, wrap.key, wrap.auth));

  console.log('Unwrap Ephemeral key pairs');
  console.log(await KeyManager.UnwrapEphemeralKeyPair(eph_wrapped, wrap.key, wrap.auth));

  console.log('Unwrap One-Time key pairs');
  console.log(await KeyManager.UnwrapOneTimeKeyPair(ot_wrapped, wrap.key, wrap.auth));

}

stuff().then(_ => console.log('Tests Complete'), err => console.error(err));
