<?php
// This file contains utility functions for accessing the database and file i/o.

namespace util;

// Constants

// All paths are relative to here.
const kBaseDir  = '/afs/cad/u/z/t/ztk4/njit/cs631/p3/';
// Hostname of db service.
const kDbUri    = 'sql3.njit.edu';
// mysql username
const kDbUser   = 'ejd29';
// Databse name on mysql instance.
const kDbName   = 'ejd29';

// Functions

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

// Returns a mysqli object handle connected to our db.
// The caller MUST call ->close() on the returned object.
// Throws a RuntimeException on connection failure.
function make_db_connection() {
  $conn = new mysqli(kDbUri, kDbUser, trim(cat_file('.db.pass')), kDbName);

  if ($conn->connect_error) {
    throw new \RuntimeException('Error(' . $conn->connect_errno . '): ' .
                                'Failed to connect to db ``' .
                                $conn->connect_error . "''");
  }

  return $conn;
}

// Returns the value of the session entry for $key.
// Throws OutOfBoundsException when $key is not present.
function expect_session_entry($key) {
  if (!isset($_SESSION[$key]))
    throw new \OutOfBoundsException($key . ' not in SESSION');

  return $_SESSION[$key];
}

?>
