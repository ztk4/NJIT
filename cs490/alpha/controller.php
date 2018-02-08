<?php 

// This file takes post data and response with a JSON document.
//
// The request should contain the following POST entries:
// user: username in plaintext
// pass: password in plaintext
//
// This JSON response will take the following format:
// {
//   "description": "controller response",
//   "oneOf": [
//     "properties": {
//       "err": { "type": "string" }
//      },
//      "properties": {
//        "localSuccess": { "type": "boolean" },
//        "njitSuccess": { "type": "boolean" }
//      }
//   ]
// }

// Constants used within this script.
$njit_webauth = "https://cp4.njit.edu/cp/home/login";
$njit_key = 'njitSuccess';
$local_db = "https://web.njit.edu/~UCID/path/to/script.php";  // TODO: Fill in.
$local_key = 'localSuccess';

// Set the content type for good measure.
header('Content-Type: application/json');

// Response data.
$data = array();
// Little bit of error handling.
if (!isset($_POST['user'])) {
  $data['err'] = "Request post body must contain 'user' entry.";
} elseif (!isset($_POST['pass'])) {
  $data['err'] = "Request post body must contain 'pass' entry.";
} else {

  $user = htmlspecialchars($_POST['user']);
  $pass = htmlspecialchars($_POST['pass']);

  // First, lets hit the NJIT database:
  $webauth_post = array(
    'user' => $user,
    'pass' => $pass,
    'uuid' => '0xACA021'  // Weird constant baked into login script.
  );
  $cp = curl_init($njit_webauth);
  $webauth_post = http_build_query($webauth_post);
  curl_setopt($cp, CURLOPT_POST, 1);
  curl_setopt($cp, CURLOPT_POSTFIELDS, $webauth_post);
  curl_setopt($cp, CURLOPT_RETURNTRANSFER, true);

  curl_exec($cp);  // Don't need the response value.
  $code = curl_getinfo($cp, CURLINFO_HTTP_CODE);
  curl_close($cp);
  if ($code == 200) {
    // Code 200 means that the response was OK and we were returned an 'Invalid
    // Login' page.
    $data[$njit_key] = false;
  } else if ($code == 302) {
    // Code 302 means that the login was successful, and the client should
    // move on to the Location header to continue the login process.
    $data[$njit_key] = true;
  } else {
    $data['err'] = "Failed to connect to the NJIT login page.";
  }

  // Second, lets hit the backend storage script.
  $local_post = array(
    'user' => $user,  // TODO: Ensure this format is okay.
    'pass' => $pass
  );
  $cp = curl_init($local_db);
  $local_post = http_build_query($local_post);
  curl_setopt($cp, CURLOPT_POST, 1);
  curl_setopt($cp, CURLOPT_POSTFIELDS, $local_post);
  curl_setopt($cp, CURLOPT_RETURNTRANSFER, true);

  $resp = json_decode(curl_exec($cp));
  if (is_null($resp)) {
    $data['err'] = "Failed to parse local db response as JSON";
  } else {
    $data[$local_key] = $resp->found;
  }
}

// Set code to Bad Request if an error occurred.
if (isset($data['err'])) {
  http_response_code(400);
}
echo json_encode($data);

?>
