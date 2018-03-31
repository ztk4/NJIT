<?php

// This file takes past data and responds with a JSON document.
//
// The request should contain the following POST entries:
// changes: array(
//  array('user' => user name, 'questions' => array(
//    array('qid' => question id, 'deductions' => new deductions object), ...
//  )
// )
// token: secret in plain text
//
// This JSON response will take the following format:
// {
//   "description": "Alters pending exams",
//   "properties": {
//     "err": { "type": "string" },
//     "success": { "type": "boolean" }
//   }
// }

set_include_path(require 'get_include_path.php');
require 'prelude.inc';

// Response object.
$data = array();

try {
  // Get changelist from POST data.
  $changes = util\expect_post_entry('changes');

  // Ask the DB for relevant exam info.
  $exam_resp = util\make_db_request('get_active_exam');
  if ($exam_resp->success === 'success') {
    // Get points for each question.
    $qscores = array();
    foreach ($exam_resp->exam as $question) {
      $qscores[$question->QId] = $question->Points;
    }

    // Pre-Calculate new scores.
    foreach ($changes as $cidx => $change) {
      foreach ($change['questions'] as $qidx => $question) {
        $score = $qscores[$question['qid']];
        foreach (json_decode($question['deductions']) as $deduction) {
          $score -= $deduction->points;
        }
        if ($score < 0) $score = 0;
        $changes[$cidx]['questions'][$qidx]['score'] = $score;
      }
    }

    // Ask the DB to carry out the alterations.
    $post_data = array(
      'changes' => $changes,
    );
    $resp = util\make_db_request('alter_pending_exams', $post_data);

    $data['success'] = ($resp->status === 'success');
  }
} catch (Exception $e) {
  // On error, set code to 400 and set err property.
  http_response_code(400);
  $data['err'] = $e->getMessage();
}

echo json_encode($data);

?>
