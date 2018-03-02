<?php 

// This file takes post data and responds with a JSON document.
//
// The request should contain the following POST entries:
// token: secret in plaintext
//
// This JSON response will take the following format:
// {
//   "description": "all questions in question bank",
//   "properties": {
//     "err": { "type": "string" },
//     "success": { "type": "boolean" },
//       "questions": {
//
//         "type": "array",
//         "items": { 
//           "properties": {
//             "qid": { "type": "integer", "description": "question id" },
//             "difficulty": { "type": "string" },
//             "topic": { "type": "string" },
//             "prompt": { "type": "string" },
//             "fname": { "type": "string" },
//             "testcases": {
//               "type": "array",
//               "items": { "type": "string" }
//             }
//           }
//         }
//
//     }
//   }
// }

require 'prelude.inc';

// Response object.
$data = array();

try {
  // Ask db for all questions.
  $resp = util\make_db_request('get_all_questions');

  if ($data['success'] = ($resp->status === 'success')) {
    $data['questions'] = $resp->questions;  // TODO: Check if this is okay.
  }
} catch(Exception $e) {
  // On error, set code to 400 and set err property.
  http_response_code(400);
  $data['err'] = $e->getMessage();
}

echo json_encode($data);

?>
