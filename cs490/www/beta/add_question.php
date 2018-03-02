<?php 

// This file takes post data and responds with a JSON document.
//
// The request should contain the following POST entries:
// difficulty: string indiciating difficulty
// topic: string indicating topic
// prompt: string listing the prompt
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
//     "success": { "type": "boolean" },
//   }
// }

set_include_path(require 'get_include_path.php');
require 'prelude.inc';

// Response object.
$data = array();

try {
  // Get info.
  $difficulty = util\expect_post_entry('difficulty');
  $topic = util\expect_post_entry('topic');
  $prompt = util\expect_post_entry('prompt');
  $fname = util\expect_post_entry('fname');
  $testcases = util\expect_post_entry('testcases');

  // Validate info.
  if (!util\validate_question($fname, $testcases))
    throw new RuntimeException('invalid fname or testcases');

  // Publish question to db.
  $post_data = array(
    'difficulty' => $difficulty,
    'topic' => $topic,
    'prompt' => $prompt,
    'fname' => $fname,
    'testcases' => $testcases
  );
  $resp = util\make_db_request('add_question', $post_data);

  $data['success'] = ($resp->status === 'success');
} catch(Exception $e) {
  // On error, set code to 400 and set err property.
  http_response_code(400);
  $data['err'] = $e->getMessage();
}

echo json_encode($data);

?>
