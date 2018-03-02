<?php

// This file sets up the environment for all endpoints.

// Pull in utility functions and constants.
require 'util.php'

// Set json as content type.
header('Content-Type: application/json');

// Validate incoming connection.
try {
  if (util\cat_file('.incoming.tok') !== util\expect_post_entry('token'))
    throw new RuntimeException('invalid connection');
} catch (Exception $e) {
  echo json_encode(array('err' => $e->message));
  exit;
}

?>
