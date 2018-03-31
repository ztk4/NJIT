<?php 

// This file takes post data and responds with a JSON document.
//
// The request should contain the following POST entries:
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

set_include_path(require 'get_include_path.php');
require 'prelude.inc';

// Response object.
$data = array();

try {
  // Ask db for active exam.
  $resp = util\make_db_request('get_active_exam');

  if ($data['success'] = ($resp->success === 'success')) {
    $data['exam'] = array(
      'eid' => $resp->exam[0]->EId,
      'questions' => array()
    );
    foreach ($resp->exam as $question) {
      array_push($data['exam']['questions'], array(
        'qid' => $question->QId,
        'prompt' => $question->Question,
        'fname' => $question->FName,
        'points' => $question->Points,
      ));
    }
  }
} catch(Exception $e) {
  // On error, set code to 400 and set err property.
  http_response_code(400);
  $data['err'] = $e->getMessage();
}

echo json_encode($data);

?>
