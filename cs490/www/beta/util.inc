<?php

// Common constants and utilities.

// Make namespace to avoid name collisions.
namespace util;

// CONSTANTS

// Directory where non-webhosted files live.
const kBaseDir = '/afs/cad/u/z/t/ztk4/njit/cs490/beta/';
// Uri where DB enpoints are located.
const kDbUri = 'https://web.njit.edu/~poa5/';  // TODO: Check path.
// Comand for running evaluate (without arguments to evaluate).
// NOTE: flags passed to sandbox before -- control restrictions.
const kEvalCmd = kBaseDir . 'sandbox -- `which python3` -S ' .
                 kBaseDir . 'evaluate.py';

// FUNCTIONS

// Read file from base dir.
// Throws a RuntimeException on failure.
// WARNING: Does not parse $file to validate path, do NOT pass user given
//          strings to this function.
function cat_file($file) {
  $path = kBaseDir . $file;

  $fp = fopen($path, 'r');
  if (!$fp)
    throw new RuntimeException('failed to open file');

  $contents = fread($fp, filesize($path));
  fclose($fp);

  if (!$contents)
    throw new RuntimeException('failed to read file');

  return $contents;
}

// Returns the value of the $key entry in post.
// Throws OutOfBoundsException when $key is not in post.
function expect_post_entry($key) {
  if (!isset($_POST[$key]))
    throw new OutOfBoundsException($key . ' not in POST data');

  return $_POST[$key]
}

// Makes a request to a DB endpoint, returns result as a php object.
// Throws a RuntimeException on failure.
function make_db_request($endpoint, $data = array()) {
  // TODO: secret token.

  // Prepare post data.
  $post_data = http_build_query(array('data' => $data));

  // Prepare curl request.
  $cp = curl_init(kDbUri . $endpoint . '.php');
  curl_setopt($cp, CURLOPT_POST, 1);
  curl_setopt($cp, CURLOPT_POSTFIELDS, $post_data);
  curl_setopt($cp, CURLOPT_RETURNTRANSFER, true);

  // Execute the curl request.
  $resp = json_decode($curl_exec($cp));
  curl_close($cp);

  if (is_null($resp))
    throw new RuntimeException('unable to make curl request');

  return $resp;
}

// Validates the given test cases and function name for a given question
// Returns true/false indicating the validity of question.
// TODO: Consider surfacing error messages indicating why it might be invalid.
function validate_question($fname, $testcases) {
  // Prepare cmd string.
  $cmd = kEvalCmd . ' --name=' . escapeshellarg($fname);
  foreach ($testcases as $testcase) {
    $cmd .= ' --test_case=' . escapeshellarg($testcase);
  }

  // Execute the command.
  $rc = -1;
  $output = array();
  exec($cmd, $output, $rc);

  // rc of 0 indicates valid question parameters.
  return ($rc === 0);
}

// Grades a question given a max number of points and the student's code.
// Returns a php object with score and deductions.
// Throws RuntimeException on failure.
function grade_answer($points, $fname, $student_code, $testcases) {
  $cmd = kEvalCmd . ' --name=' . escapeshellarg($fname) .
                    ' --code=' . escapeshellarg($code) .
                    ' --points=' . escapeshellarg($points);
  foreach ($testcases as $testcase) {
    $cmd .= ' --test_case=' . escapeshellarg($testcase);
  }

  // Execute the command.
  $rc = -1;
  $output = array();
  exec($cmd . ' 2>&1', $output, $rc);

  // Collapse output.
  $output = implode("\n", $output);

  if ($rc === 0) {
    $tmp = json_decode($output);
    if (is_null($tmp))
      throw new RuntimeException('unable to parse evaluation output');
    return $tmp;
  }

  // TODO: Check if nonzero return code indicates failure of the sandbox or
  //       evaluator vs. failure of student code / malicious behavior.
  //       For now, assuming failure of student code, 0 marks.
  return array(
    'score' => 0,
    'deductions' => array(
      array('points' => $points, 'reason' => $output)
    )
  );
}

?>
