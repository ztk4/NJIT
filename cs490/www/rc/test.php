<?php

set_include_path(require 'get_include_path.php');
require 'util.inc';

try {
  $output = util\make_eval_request(array(
    'fname' => 'test',
    'testcases' => array('(1, 2, 3)==6', '(-42)==-42', '()==0'),
    'code' => "def tet(*args):\n\ta = 1\n\tfor i in range(1000000):\n\t\ta += a",
    'points' => 100
  ));

  echo '<code>';
  echo json_encode($output);
  echo '</code>';
} catch (Exception $e) {
  echo $e->getMessage();
}

?>
