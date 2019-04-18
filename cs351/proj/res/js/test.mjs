import Storage from '/web/res/js/storage.mjs'

async function stuff(user) {
  const storage = await Storage.Open(user);

  try {
    console.log('Exists?: ' + await storage.UserExists());

    console.log('Get Id');
    console.log(await storage.GetIdentityPk());

    console.log('Set Id');
    console.log(await storage.SetIdentityPk());

    console.log('Get Signed Key');
    console.log(await storage.GetSignedPk());

    console.log('Set Signed Key');
    console.log(await storage.SetSignedPk('signed_key1'));

    console.log('Get signed key');
    console.log(await storage.GetSignedPk());

    console.log('Reset Signed Key');
    console.log(await storage.SetSignedPk('signed_key2'));

    console.log('Set Several Keys');
    console.log(await storage.ReplenishOneTimeKeys([{k:'one'}, {k:'two'}, {k:'three'}]));

    console.log('Remove key 2');
    console.log(await storage.PopOneTimeKey(2));

    console.log('Add 4 more keys');
    console.log(await storage.ReplenishOneTimeKeys(['four', 'five', 'six', 'seven'].map( x => ({k: x}) )));

    console.log('Remove key 2 again (should return nothing)');
    console.log(await storage.PopOneTimeKey(2));
  }
  catch (err) {
    console.error('Error:');
    console.error(err);
  }

  console.log('Cleaning up');

  // Cleanup!
  await storage.DeleteAll();
}

stuff('test1');
