<?php
// This file attempts to insert a product into a user's cart, or update the qty
// of an existing product if it's already present. This file acts as an http
// endpoint for requesting this action without rendering a graphical response.
// Fit for asynchronous dispatch from javascript for example.

require 'util.inc'
require 'db.inc'

try {
  // Expect to recieve a product id and a quantity.
  $pid = util\expect_post_entry('pid');
  $qty = util\expect_post_entry('qty');

  // Call the endpoint from db.
  if (!upsert_product($pid, $qty))
    throw new RuntimeException('Failed to upsert ' . $qty . ' of product #' .
                               $pid);

  http_response_code(204);  // Upsert was okay, but no content.
} catch (Exception $e) {
  http_response_code(400);  // Generic error code.
  echo $e->getMessage();    // Helpful for debugging.
}

?>
