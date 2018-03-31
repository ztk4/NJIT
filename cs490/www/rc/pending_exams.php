<?php 

// This file takes post data and responds with a JSON document.
//
// The request should contain the following POST entries:
// token: secret in plaintext
//
// This JSON response will take the following format:
// {
//   "description": "All pending (submitted but unreleased) exams",
//   "properties": {
//     "err": { "type": "string" },
//     "success": { "type": "boolean" },
//     "eid": { "type": "integer" },
//     "exams": {
//
//       "properties": {
//         "USER_NAME": {
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
//         /* Other Users */
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
  // Ask db for pending exams.
  $resp = util\make_db_request('get_pending_exams');

  if ($data['success'] = 
        ($resp->status === 'success' && count($resp->exams) > 0)) {
    $data['eid'] = $resp->exams[0]->EId;
    $data['exams'] = array();
    foreach ($resp->exams as $examrow) {
      if (!isset($data['exams'][$examrow->User])) {
        $data['exams'][$examrow->User]['questions'] = array();
      }
      if (!isset($data['exams'][$examrow->User]['questions'][$examrow->QId])) {
        $data['exams'][$examrow->User]['questions'][$examrow->QId] = array();
      }
      $data['exams'][$examrow->User]['questions'][$examrow->QId] = array(
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
