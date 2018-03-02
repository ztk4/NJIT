<?php 

// This file takes post data and responds with a JSON document.
//
// The request should contain the following POST entries:
// user: username in plaintext
// token: secret in plaintext
//
// This JSON response will take the following format:
// {
//   "description": "exam history for given user",
//   "properties": {
//     "err": { "type": "string" },
//     "success": { "type": "boolean" },
//     "exams": {
//
//       "type": "array",
//       "items": {
//         "properties": {
//           "eid": { "type": "integer", "description": "exam id" },
//           "questions": {
//
//             "type": "array",
//             "items": { 
//               "properties": {
//                 "qid": { "type": "integer", "description": "question id" },
//                 "points": { "type": "integer" },
//                 "deductions": {
//
//                   "type": array",
//                   "items": {
//                     "properties": {
//                       "points": { "type": "integer" },
//                       "reason": { "type": "string" },
//                     }
//                   }
//
//                 }
//               }
//             }
//
//           }
//         }
//       }
//
//     }
//   }
// }

require 'prelude.inc';

// Response object.
$data = array();

try {
  // Get info.
  $user = util\expect_post_entry('user');

  // Ask db for past exams.
  $post_data = array(
    'user' => $user,
  );
  $resp = util\make_db_request('past_scores', $post_data);

  if ($data['success'] = ($resp->status === 'success')) {
    $data['exams'] = $resp->exams;  // TODO: Check if this is okay.
  }
} catch(Exception $e) {
  // On error, set code to 400 and set err property.
  http_response_code(400);
  $data['err'] = $e->getMessage();
}

echo json_encode($data);

?>
