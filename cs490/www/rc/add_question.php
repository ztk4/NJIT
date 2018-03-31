<?php 

// This file takes post data and responds with a JSON document.
//
// The request should contain the following POST entries:
// diff: string indiciating difficulty
// topic: string indicating topic
// question: string listing the prompt
// fname: string (the name of the function)
// testcases: array(
//   string representing a test case (see evaluate.py for format)
// )
// token: secret in plaintext
//
// This JSON response will take the following format:
// {
//   "description": "indicates if question addition worked",
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
  // Get info.
  $difficulty = util\expect_post_entry('diff');
  $topic = util\expect_post_entry('topic');
  $prompt = util\expect_post_entry('question');
  $fname = util\expect_post_entry('fname');
  $testcases = util\expect_post_entry('cases');

  // Validate info.
  $eval_data = array('fname' => $fname, 'testcases' => $testcases);
  if (!util\make_eval_request($eval_data)->success)
    throw new RuntimeException('invalid fname or testcases');

  // Publish question to db.
  $post_data = array(
    'difficulty' => $difficulty,
    'topic' => $topic,
    'question' => $prompt,
    'fname' => $fname,
    'testcases' => $testcases
  );
  $resp = util\make_db_request('add_questions', $post_data);

  $data['success'] = ($resp->status === 'success');
} catch(Exception $e) {
  // On error, set code to 400 and set err property.
  http_response_code(400);
  $data['err'] = $e->getMessage();
}

echo json_encode($data);

?>
