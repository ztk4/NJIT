<?php

// Common constants and utilities.

// Make namespace to avoid name collisions.
namespace util;

// CONSTANTS

// Directory where non-webhosted files live.
const kBaseDir = '/afs/cad/u/z/t/ztk4/njit/cs490/rc/';
// Uri where DB enpoints are located.
const kDbUri = 'https://web.njit.edu/~poa5/beta/';
// Uri where evaluate endpoint is located.
const kEvalUri = 'http://afsaccess1.njit.edu/~ztk4/cs490/rc/evaluate.php';
// Comand for running evaluate (without arguments to evaluate).
// NOTE: flags passed to sandbox before -- control restrictions.
const kEvalCmd = kBaseDir . 'sandbox -- ' .
                 '/afs/cad/linux/anaconda3/anaconda/bin/python3.5 -S ' .
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
    throw new \RuntimeException('failed to open file');

  $contents = fread($fp, filesize($path));
  fclose($fp);

  if (!$contents)
    throw new \RuntimeException('failed to read file');

  return $contents;
}

// Returns the value of the $key entry in post.
// Throws OutOfBoundsException when $key is not in post.
function expect_post_entry($key) {
  if (!isset($_POST[$key]))
    throw new \OutOfBoundsException($key . ' not in POST data');

  return $_POST[$key];
}

// Same as above, but returns null when not present.
function maybe_post_entry($key) {
  if (!isset($_POST[$key]))
    return null;

  return $_POST[$key];
}

// Generic curl request, returns result as a php object.
// Throws a RuntimeException on failure.
function make_curl_request($uri, $data) {
  // Prepare post data.
  $post_data = http_build_query($data);

  // Prepare request.
  $cp = curl_init($uri);
  curl_setopt($cp, CURLOPT_POST, true);
  curl_setopt($cp, CURLOPT_POSTFIELDS, $post_data);
  curl_setopt($cp, CURLOPT_RETURNTRANSFER, true);

  // Execute request.
  $resp = json_decode(curl_exec($cp));
  curl_close($cp);

  if (is_null($resp))
    throw new \RuntimeException('unable to make curl request');

  return $resp;
}

// Makes a request to a DB endpoint, returns result as a php object.
// Throws a RuntimeException on failure.
function make_db_request($endpoint, $data = array()) {
  $data['token'] = trim(cat_file('.outgoing.tok'));

  // Make request.
  return make_curl_request(kDbUri . $endpoint . '.php', $data);
}

// Makes an internal curl request to the debug server for execution purposes.
// More details can be found in evaluate.php
// Throws a RuntimeException on failure.
function make_eval_request($data) {
  // Validate as own incoming connection.
  $data['token'] = trim(cat_file('.incoming.tok'));

  // Make request.
  return make_curl_request(kEvalUri, $data);
}

?>
