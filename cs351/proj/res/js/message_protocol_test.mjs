import MessageProtocol from '/web/res/js/message_protocol.mjs'

async function stuff() {
  try {
    // Create User Test.
    console.log('Creating a new User');
    console.log(await MessageProtocol.CreateNewUser('test', 'password'));
  } catch (err) {
    console.error(err, err.message);
  }

  // Cleanup.
  await MessageProtocol.DeleteUser('test');
}

stuff().then(_ => console.log('Tests Complete'), err => console.error(err, err.message));
