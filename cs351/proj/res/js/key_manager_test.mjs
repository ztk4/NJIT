import KeyManager from '/web/res/js/key_manager.mjs'

function CompareAb(lhs, rhs) {
  if (lhs.byteLength !== rhs.byteLength) return 'Lengths Differ';

  const lview = new Uint8Array(lhs);
  const rview = new Uint8Array(rhs);
  for (const [idx, lv] of lview.entries()) {
    if (lv !== rview[idx]) return 'Mismatch at position ' + idx;
  }

  return 'Equal';
}

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
  let id_export = await KeyManager.ExportIdentityKeys(id_keypairs);
  console.log(id_export);

  console.log('Export Signed key pair');
  let signed_export = await KeyManager.ExportSignedKey(signed_keypair, id_keypairs);
  console.log(signed_export);

  console.log('Export Ephemeral key pair');
  let eph_export = await KeyManager.ExportEphemeralKey(ephemeral_keypair);
  console.log(eph_export);

  console.log('Export a One-Time key pair');
  let ot_export = await KeyManager.ExportOneTimeKey(one_time_keypairs[2]);
  console.log(ot_export);

  // Public Key Import Tests.

  console.log('Partial key import');
  let imported_partial = await KeyManager.ImportPublicKeys(id_export, signed_export);
  console.log(imported_partial);

  console.log('Full key import');
  let imported = await KeyManager.ImportPublicKeys(id_export, signed_export, ot_export);
  console.log(imported);

  console.log('Ephemeral key import');
  let eph_imported = await KeyManager.ImportEphemeralKey(eph_export);
  console.log(eph_export);

  // Salt and Wrapping Key Gen Tests.
  
  console.log('Salt generation');
  let salt = KeyManager.GenerateSalt(16);
  console.log(salt);

  let now = new Date();
  console.log('Wrapping key and auth generation');
  let wrap = await KeyManager.MakeWrappingKeyAndAuth('mypassword', salt);
  let elapsed = new Date() - now;
  console.log(wrap);
  console.log('Elapsed Time (ms): ', elapsed);

  // Public Key Wrapping Tests.
  
  console.log('Wrap ID key pairs');
  let id_wrapped = await KeyManager.WrapIdentityKeyPairs(id_keypairs, wrap.key, wrap.auth);
  console.log(id_wrapped);

  console.log('Wrap Signed key pairs');
  let signed_wrapped = await KeyManager.WrapSignedKeyPair(signed_keypair, wrap.key, wrap.auth);
  console.log(signed_wrapped);

  console.log('Wrap Ephemeral PK');
  let eph_wrapped = await KeyManager.WrapEphemeralPublicKey(ephemeral_keypair, wrap.key, wrap.auth);
  console.log(eph_wrapped);

  console.log('Wrap a One-Time key pair');
  let ot_wrapped = await KeyManager.WrapOneTimeKeyPair(one_time_keypairs[2], wrap.key, wrap.auth);
  console.log(ot_wrapped);

  // Public Key Unwrap Tests.

  console.log('Unwrap ID key pairs');
  console.log(await KeyManager.UnwrapIdentityKeyPairs(id_wrapped, wrap.key, wrap.auth));

  console.log('Unwrap Signed key pairs');
  console.log(await KeyManager.UnwrapSignedKeyPair(signed_wrapped, wrap.key, wrap.auth));

  console.log('Unwrap Ephemeral PK');
  console.log(await KeyManager.UnwrapEphemeralPublicKey(eph_wrapped, wrap.key, wrap.auth));

  console.log('Unwrap One-Time key pairs');
  console.log(await KeyManager.UnwrapOneTimeKeyPair(ot_wrapped, wrap.key, wrap.auth));

  // Initial Session Key Tests.

  // Key pairs recipient would have.
  let recip_id_keys = await KeyManager.MakeIdentityKeyPairs();
  let recip_signed_key = await KeyManager.MakeSignedKeyPair();
  let recip_ot_key = (await KeyManager.MakeOneTimeKeyPairs(1))[0];
  
  // PKs recipient would receive.
  let init_id_pks = imported.id_pks;
  let init_eph_pk = eph_imported;

  // PKs initiator would recieve.
  let recip_id_export = await KeyManager.ExportIdentityKeys(recip_id_keys);
  let recip_signed_export = await KeyManager.ExportSignedKey(recip_signed_key, recip_id_keys);
  let recip_ot_export = await KeyManager.ExportOneTimeKey(recip_ot_key);
  let recip_import = await KeyManager.ImportPublicKeys(recip_id_export, recip_signed_export, recip_ot_export);
  let recip_id_pks = recip_import.id_pks;
  let recip_signed_pk = recip_import.signed_pk;
  let recip_ot_pk = recip_import.ot_pk;

  console.log('Without OneTime Keys');
  let partial_init_sess = 
    await KeyManager.DeriveInitiatorSessionKeys(id_keypairs, ephemeral_keypair,
                                                recip_id_pks, recip_signed_pk);
  let partial_recip_sess =
    await KeyManager.DeriveRecipientSessionKeys(init_id_pks, init_eph_pk,
                                                recip_id_keys, recip_signed_key);
  console.log('Root Key Cmp: ',
    CompareAb(partial_init_sess.root_key, partial_recip_sess.root_key));
  console.log('Chain Key Cmp: ',
    CompareAb(partial_init_sess.chain_key, partial_recip_sess.chain_key));

  console.log('With OneTime Keys');
  let init_sess = 
    await KeyManager.DeriveInitiatorSessionKeys(id_keypairs, ephemeral_keypair,
                                                recip_id_pks, recip_signed_pk, recip_ot_pk);
  let recip_sess =
    await KeyManager.DeriveRecipientSessionKeys(init_id_pks, init_eph_pk,
                                                recip_id_keys, recip_signed_key, recip_ot_key);
  console.log('Root Key Cmp: ',
    CompareAb(init_sess.root_key, recip_sess.root_key));
  console.log('Chain Key Cmp: ',
    CompareAb(init_sess.chain_key, recip_sess.chain_key));

}

stuff().then(_ => console.log('Tests Complete'), err => console.error(err, err.message));
