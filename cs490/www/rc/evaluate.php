<?php

// This is a wrapper for running the evaluate python script within the sandbox.
// Unfortunately, the public webost host runs a pre 3.5 kernel, which is needed
// for seccomp filter mode, so this endpoint is access on the debug server
// afsacces1 from the public webhost.
//
// This file takes post data and responds with a JSON document.
//
// The request should contain the following POST entries:
// fname: function name.
// testcases: array( testcase strings ).
// code: (optional) student code.
// points (optional) max points.
//
// This JSON response will take the following format:
// {
//   "description": "evaluation response",
//   "properties": {
//     "err": { "type": "string" },
//     "success": { "type": "boolean" },
//     "score": { "type": "integer" },
//     "deductions": {
//       "type": "array",
//       "items": {
//         "points": { "type": "integer" },
//         "reason": { "type": "string" }
//       }
//     }
//   }
// }

set_include_path(require 'get_include_path.php');
require 'prelude.inc';

// Response.
// $data = array();

try {
  // Get required info.
  $fname = util\expect_post_entry('fname');
  $testcases = util\expect_post_entry('testcases');
  // Optional info.
  $code = util\maybe_post_entry('code');
  $points = util\maybe_post_entry('points');
  $constraint = util\maybe_post_entry('constraint');

  // Attempt to build a command and execute it.
  $cmd = util\kEvalCmd . ' --name=' . escapeshellarg($fname);
  foreach ($testcases as $testcase) {
    $cmd .= ' --test_case=' . escapeshellarg($testcase);
  }
  if (!is_null($code)) {
    $cmd .= ' --code=' . escapeshellarg($code) .
            ' --points=' . escapeshellarg($points);
  }
  if ($constraint === 'for-loop' || $constraint === 'if-statement' ||
      $constraint === 'recursion') {
    $cmd .= ' --restriction=' . escapeshellarg($constraint);
  }

  // Execute Command.
  $rc = -1;
  $output = array();
  exec($cmd . ' 2>&1', $output, $rc);

  $output = implode("\n", $output);

  if (is_null($code)) {
    // Just checking validity of test cases.
    $data['success'] = ($rc === 0);
  } else {
    // Grading an answer.
    if ($rc === 0) {
      $tmp = json_decode($output);
      if (is_null($tmp))
        throw new \RuntimeException('unable to parse evaluation output');

      $data['score'] = $tmp->score;
      $data['deductions'] = $tmp->deductions;
    } else {
      // TODO: Check if nonzero return code indicates failure of the sandbox or
      //       evaluator vs. failure of student code / malicious behavior.
      //       For now, assuming failure of student code, 0 marks.
      $data['score'] = 0;
      $data['deductions'] = array(
        array('points' => $points, 'reason' => $output)
      );
    }

    $data['success'] = true;
  }
} catch (Exception $e) {
  // On error, set code to 400 and set err property.
  http_response_code(400);
  $data['err'] = $e->getMessage();
}

echo json_encode($data);

?>
