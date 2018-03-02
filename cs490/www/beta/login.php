<?php 

// This file takes post data and responds with a JSON document.
//
// The request should contain the following POST entries:
// user: username in plaintext
// pass: password in plaintext
// token: secret in plaintext
//
// This JSON response will take the following format:
// {
//   "description": "login response",
//   "properties": {
//     "err": { "type": "string" },
//     "success": { "type": "boolean" },
//     "instructor": { "type": "boolean", "description": "is an instructor" }
//   }
// }

require 'prelude.inc'

// Response object.
$data = array();

try {
  // Get login info.
  $user = util\expect_post_entry('user');
  $pass = util\expect_post_entry('pass');

  // Attempt login.
  $post_data = array(
    'user' => $user,
    'pass' => $pass
  );
  $resp = util\make_db_request('login_handler', $post_data);

  if ($data['success'] = ($resp->status === 'success')) {
    $data['instructor'] = $resp->instructor === 'instructor';
  }
} catch(Exception $e) {
  // On error, set code to 400 and set err property.
  http_response_code(400);
  $data['err'] = $e->message;
}

echo json_encode($data);

?>
