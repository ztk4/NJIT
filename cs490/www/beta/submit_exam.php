<?php 

// This file takes post data and responds with a JSON document.
//
// The request should contain the following POST entries:
// user: username in plaintext
// answers: array(
//   array('qid' => question id, 'code' => student code) ...
// )
// token: secret in plaintext
//
// This JSON response will take the following format:
// {
//   "description": "indicates if exam submission worked",
//   "properties": {
//     "err": { "type": "string" },
//     "success": { "type": "boolean" },
//   }
// }

set_include_path(require 'get_include_path.php');
require 'prelude.inc';

// Response object.
$data = array();

try {
  // Get info.
  $user = util\expect_post_entry('user');
  $answers = util\expect_post_entry('answers');

  // Try grading each question.
  $graded_answers = array();
  foreach ($answers as $answer) {
    // Fetch question from db.
    $post_data = array(
      'qid' => $answer['qid'],
    );
    $question = util\make_db_request('get_active_question', $post_data);

    // Attempt to grade the question.
    $grade = util\make_eval_request(array(
      'points' => $question->Points,
      'fname' => $question->FName,
      'code' => $answer['code'],
      'testcases' => explode("\n", $question->testcases)
    ));

    // TODO: Check format.
    array_push($graded_answers, array(
      'qid' => $answer['qid'],
      'code' => $answer['code'],
      'score' => $grade->score,
      'deductions' => json_encode($grade->deductions)
    ));
  }

  // Attempt to submit exam to db.
  $post_data = array(
    'user' => $user,
    'graded_answers' => $graded_answers,
  );
  $resp = util\make_db_request('submit_exam', $post_data);

  $data['success'] = ($resp->status === 'success');
} catch(Exception $e) {
  // On error, set code to 400 and set err property.
  http_response_code(400);
  $data['err'] = $e->getMessage();
}

echo json_encode($data);

?>
