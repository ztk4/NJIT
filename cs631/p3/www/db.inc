<?php
// This file defines every atomic transaction on our database.
// Each function preforms a specific transaction, and returns relevant info
// to the calling client (usually a php script rendering a client response).

require_once 'util.inc';

// We need php session for much of the logic below.
session_start();

function make_user($fname, $lname, $email, $address, $phone, $addr_name,
  $recpt_name, $street, $snumber, $city, $zip, $state, $country) {
  $conn = util\make_db_connection();
  
  // Create User.
  $stmt = util\prepare_statement($conn,
    "INSERT INTO Customer (LName, FName, Email, Address, Phone, Status)" .
    "  VALUES (?, ?, ?, ?, ?, ?);");
  $stmt->bind_param('ssssss', $fname, $lname, $email, $address, $phone,
    'Regular');
  if (!$stmt->execute()) return false;
  $stmt->close();

  // Fetch CID.
  $stmt = util\prepare_statement($conn,
    "SELECT CID FROM Customer WHERE Email = ?;");
  $stmt->bind_param('s', $email);
  $stmt->bind_result($cid);
  if (!$stmt->execute() || !$stmt->fetch()) return false;
  $stmt->close();

  // Insert Shipping Address.
  $stmt = util\prepare_statement($conn,
    "INSERT INTO Shipping_Address" .
    "  VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?);");
  $stmt->bind_param('issssssss', $cid, $addr_name, $recpt_name, $street,
    $snumber, $city, $zip, $state, $country);
  if (!$stmt->execute()) return false;
  $stmt->close();

  // Insert Empty Cart.
  $stmt = util\prepare_statement($conn,
    "INSERT INTO Cart (c, SAName, TStatus) VALUES (?, ?, ?);");
  $stmt->bind_param('iss', $cid, $addr_name, 'Active');
  if (!$stmt->execute()) return false;
  $stmt->close();

  $conn->close();
  return true;
}

function login($email) {
  $conn = util\make_db_connection();

  // Attempt to get CID.
  $stmt = util\prepare_statement($conn,
    "SELECT CID FROM Customer WHERE Email = ?;");
  $stmt->bind_param('s', $email);
  $stmt->bind_result($cid);
  if (!$stmt->execute() || !$stmt->fetch()) return false;
  $stmt->close();
  
  $_SESSION['cid'] = $cid;

  $conn->close();
  return true;
}

function logout() {
  // Simply remove user's data from session.
  unset($_SESSION['cid']);

  return true;
}

function get_user_info() {
  $conn = util\make_db_connection();
  try {
    $cid = util\expect_session_entry('cid');
  } catch (Exception $e) {
    return false;
  }

  $stmt = util\prepare_statement($conn,
    "SELECT FName, LName, Email, Address, Phone, Status" . 
    "  FROM Customer WHERE CID = ?;");
  $stmt->bind_param("i", $cid);
  $stmt->bind_result($fname, $lname, $email, $address, $phone, $status);
  if (!$stmt->execute() || !$stmt->fetch()) return false;
  $stmt->close();

  $conn->close();
  return array(
    'fname' => $fname,
    'lname' => $lname,
    'email' => $email,
    'address' => $address,
    'phone' => $phone,
    'status' => $status,
  );
}

function get_all_products() {
  $conn = util\make_db_connection();
  try {
    $cid = util\expect_session_entry('cid');
  } catch (Exception $e) {
    return false;
  }

  $stmt = util\prepare_statement($conn, "CALL getProdInfo(?);");
  $stmt->bind_param('i', $cid);
  // TODO: Bind results...
  if (!$stmt->execute() || !$stmt->fetch()) return false;
  $stmt->close();
  
  $conn->close();
  return array();  // TODO: populate
}

function upsert_product($pid, $qty) {
  $conn = util\make_db_connection();
  try {
    $cid = util\expect_session_entry('cid');
  } catch (Exception $e) {
    return false;
  }

  // TODO: need the SQL for this one still.

  $conn->close();
  return true;
}

function view_cart() {
  $conn = util\make_db_connection();
  try {
    $cid = util\expect_session_entry('cid');
  } catch (Exception $e) {
    return false;
  }

  $stmt = util\prepare_statement($conn,
    "SELECT PID, PType, PName, PriceSold, Qty, PQty" .
    "  FROM Cart NATURAL JOIN Appears_In NATURAL JOIN Product" . 
    "  WHERE CID = ? AND TStatus = 'Active';");
  $stmt->bind_param('i', $cid);
  $stmt->bind_result($pid, $ptype, $pname, $pprice, $qty, $pqty);
  if (!$stmt->execute()) return false;

  $result = array();
  while ($stmt->fetch()) {
    array_push($result, array(
      'pid' => $pid,
      'ptype' => $ptype,
      'pname' => $pname,
      'pprice' => $pprice,
      'qty' => $qty,
      'pqty' => $pqty,
    ));
  }
  $stmt->close();

  $conn->close();
  return $result;
}

function view_addresses() {
  $conn = util\make_db_connection();
  try {
    $cid = util\expect_session_entry('cid');
  } catch (Exception $e) {
    return false;
  }

  $stmt = util\prepare_statement($conn,
    "SELECT SAName FROM Shipping_Address WHERE CID = ?;");
  $stmt->bind_param('i', $cid);
  $stmt->bind_result($addr_name);
  if (!$stmt->execute()) return false;

  $result = array();
  while ($stmt->fetch()) {
    array_push($result, $addr_name);
  }
  $stmt->close();

  $conn->close();
  return $result;
}

function view_ccnums() {
  $conn = util\make_db_connection();
  try {
    $cid = util\expect_session_entry('cid');
  } catch (Exception $e) {
    return false;
  }

  $stmt = util\prepare_statement($conn,
    "SELECT CCNumber FROM Stored_Card WHERE CID = ?;");
  $stmt->bind_param('i', $cid);
  $stmt->bind_result($ccnum);
  if (!$stmt->execute()) return false;

  $result = array();
  while ($stmt->fetch()) {
    array_push($result, $ccnum);
  }
  $stmt->close();

  $conn->close();
  return $result;
}

function place_order($address_name, $ccnum) {
  $conn = util\make_db_connection();
  try {
    $cid = util\expect_session_entry('cid');
  } catch (Exception $e) {
    return false;
  }

  // TODO: SQL

  return true;
}

function view_transactions($tstatus, $tdate) {
  $conn = util\make_db_connection();
  try {
    $cid = util\expect_session_entry('cid');
  } catch (Exception $e) {
    return false;
  }

  // TODO: SQL

  return array();
}

function get_stats($sdate, $edate) {
  $conn = util\make_db_connection();
  
  // TODO: SQL

  return array();
}

?>
