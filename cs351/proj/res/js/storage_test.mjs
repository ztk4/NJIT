import Storage from '/web/res/js/storage.mjs'

async function stuff(user) {
  const storage = await Storage.Open(user);

  try {
    console.log('Exists?: ' + await storage.UserExists());

    console.log('Set Salt');
    console.log(await storage.SetWrappingSalt(crypto.getRandomValues(new Uint8Array(16))));
    
    console.log('Get Salt');
    console.log(await storage.GetWrappingSalt());

    console.log('Get Id');
    console.log(await storage.GetIdentityPk());

    console.log('Set Id');
    console.log(await storage.SetIdentityPk('id_pk'));

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

    console.log('Add a recipient');
    console.log(await storage.AddRecipient({uname: 'john'}));

    console.log('Add root keys');
    console.log(await storage.AddRootKey('john', { key: 'ROOT', id: 1}));
    console.log(await storage.AddRootKey('john', { key: 'ROOT', id: 2}));

    console.log('Add chain keys');
    console.log(await storage.AddMessageChainKeys('john', [1, 2, 4, 5].map( i => ({root_id: 2, id: i, key: 'CHAIN'}) )));

    console.log('Add one chain key');
    console.log(await storage.AddMessageChainKey('john', {root_id: 1, id: 3, key: 'CHAIN'}));

    console.log('Get chain key john.2.2.');
    console.log(await storage.GetMessageChainKey('john', 2, 2));
    console.log('Delete chain key john.2.2');
    console.log(await storage.DeleteMessageChainKey('john', 2, 2));
    console.log('Get chain key john.2.2 (should be missing)');
    console.log(await storage.GetMessageChainKey('john', 2, 2));

    console.log('Get root key john.1');
    console.log(await storage.GetRootKey('john', 1));
    console.log('Delete root key john.1');
    console.log(await storage.DeleteRootKey('john', 1));
    console.log('Get root jkey john.1 (should be missing)');
    console.log(await storage.GetRootKey('john', 1));

    console.log('Add messages');
    console.log(await storage.AddMessage('john', {txt: 'Hello,', datetime: new Date('2015-03-25T13:00:01.123142-04:00')}));
    console.log(await storage.AddMessage('john', {txt: 'How are you doing?', datetime: new Date('2015-03-25T13:00:36.142342-04:00')}));
    console.log(await storage.AddMessage('john', {txt: 'Wanna grab coffee later?', datetime: new Date('2015-03-25T13:12:42.152325-04:00')}));

    console.log('Get 2 messages');
    console.log(await storage.GetMessages('john', 2));
    console.log('Get 3 messages from BEFORE 1:10 (should only return 2)');
    console.log(await storage.GetMessages('john', 3, new Date('2015-03-25T13:10:00-04:00')));
    console.log('Get 1 message from BEFORE 1:00:36.142');
    console.log(await storage.GetMessages('john', 1, new Date('2015-03-25T13:00:36.142-04:00')));
    console.log('Get 1 message from BEFORE 1:00:36.143');
    console.log(await storage.GetMessages('john', 1, new Date('2015-03-25T13:00:36.143-04:00')));
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
