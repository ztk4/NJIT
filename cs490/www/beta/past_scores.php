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
//       "properties": {
//         "EXAM_ID": {
//
//           "properties": {
//             "questions": {
//
//               "QUESTION_ID": { 
//                 "properties": {
//                   "points": { "type": "integer" },
//                   "score": { "type": "integer" },
//                   "deductions": {
//
//                     "type": array",
//                     "items": {
//                       "properties": {
//                         "points": { "type": "integer" },
//                         "reason": { "type": "string" },
//                       }
//                     }
//
//                   }
//                 }
//               }
//
//             }
//           }
//         }
//         /* Other exam ids */
//       }
//
//     }
//   }
// }

set_include_path(require 'get_include_path.php');
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
  $resp = util\make_db_request('view_past_scores', $post_data);

  if ($data['success'] = ($resp->status === 'success')) {
    $data['exams'] = array();
    foreach ($resp->exams as $examrow) {
      if (!isset($data['exams'][$examrow->EId])) {
        $data['exams'][$examrow->EId]['questions'] = array();
      }
      if (!isset($data['exams'][$examrow->EId]['questions'][$examrow->QId])) {
        $data['exams'][$examrow->EId]['questions'][$examrow->QId] = array();
      }
      $data['exams'][$examrow->EId]['questions'][$examrow->QId] = array(
        'points' => $examrow->Points,
        'score' => $examrow->Score,
        'deductions' => json_decode($examrow->Deductions)
      );
    }
  }
} catch(Exception $e) {
  // On error, set code to 400 and set err property.
  http_response_code(400);
  $data['err'] = $e->getMessage();
}

echo json_encode($data);

?>
