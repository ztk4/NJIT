<?php 

// This file takes post data and responds with a JSON document.
//
// The request should contain the following POST entries:
// questions: array(
//   array('qid' => id of question, 'points' => question worth) ...
// )
// token: secret in plaintext
//
// This JSON response will take the following format:
// {
//   "description": "indicates if exam addition worked",
//   "properties": {
//     "err": { "type": "string" },
//     "success": { "type": "boolean" },
//   }
// }

require 'prelude.inc';

// Response object.
$data = array();

try {
  // Get info.
  $questions = util\expect_post_entry('questions');

  // Publish exam to db.
  $post_data = array(
    'questions' => $questions
  );
  $resp = util\make_db_request('make_exam', $post_data);

  $data['success'] = ($resp->status === 'success');
} catch(Exception $e) {
  // On error, set code to 400 and set err property.
  http_response_code(400);
  $data['err'] = $e->getMessage();
}

echo json_encode($data);

?>
