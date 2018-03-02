<?php 

// This file takes post data and responds with a JSON document.
//
// The request should contain the following POST entries:
// user: username in plaintext
// token: secret in plaintext
//
// This JSON response will take the following format:
// {
//   "description": "active exam for user",
//   "properties": {
//     "err": { "type": "string" },
//     "success": { "type": "boolean" },
//     "exam": {
//       "properties": {
//         "eid": { "type": "integer", "description": "exam id" },
//         "questions": {
//
//           "type": "array",
//           "items": { 
//             "properties": {
//               "qid": { "type": "integer", "description": "question id" },
//               "prompt": { "type": "string" },
//               "fname": { "type": "string" },
//               "points": { "type": "integer" }
//             }
//           }
//
//         }
//       }
//     }
//   }
// }

require 'prelude.inc'

// Response object.
$data = array();

try {
  // Get info.
  $user = util\expect_post_entry('user');

  // Ask db for active exam.
  $post_data = array(
    'user' => $user,
  );
  $resp = util\make_db_request('get_active_exam', $post_data);

  if ($data['success'] = ($resp->status === 'success')) {
    $data['exam'] = $resp->exam;  // TODO: Check if this is okay.
  }
} catch(Exception $e) {
  // On error, set code to 400 and set err property.
  http_response_code(400);
  $data['err'] = $e->message;
}

echo json_encode($data);

?>
