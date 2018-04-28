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
//             },
//             "constraint": { "type": "string" }
//           }
//         }
//
//     }
//   }
// }

set_include_path(require 'get_include_path.php');
require 'prelude.inc';

// Response object.
$data = array();

try {
  $resp = util\make_db_request('get_all_questions');

  if ($data['success'] = ($resp->success === 'success')) {
    $data['questions'] = array();
    foreach ($resp->questions as $question) {
      array_push($data['questions'], array(
        'qid' => $question->QId,
        'prompt' => $question->Question,
        'difficulty' => $question->Difficulty,
        'topic' => $question->Topic,
        'fname' => $question->FName,
        'testcases' => explode("\n", $question->testcases),
        'constraint' => $question->Constraints,
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
