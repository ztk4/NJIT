<?php
// This file defines every atomic transaction on our database.
// Each function preforms a specific transaction, and returns relevant info
// to the calling client (usually a php script rendering a client response).

require_once 'util.inc';

// We need php session for much of the logic below.
session_start();

function make_customer($fname, $lname, $email, $address, $phone, $addr_name,
  $recpt_name, $street, $snumber, $city, $zip, $state, $country) {
  $conn = util\make_db_connection();
  
  // Call register procedude.
  $stmt = util\prepare_statement($conn,
    "CALL register(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);");
  $stmt->bind_param('sssssssssssss', $fname, $lname, $email, $address, $phone,
    $addr_name, $recpt_name, $street, $snumber, $city, $zip, $state, $country);
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
  if (!$stmt->execute()) return false;
  $stmt->bind_result($cid);
  if (!$stmt->fetch()) return false;
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

function is_logged_in() {
  return isset($_SESSION['cid']);
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
  if (!$stmt->execute()) return false;
  $stmt->bind_result($fname, $lname, $email, $address, $phone, $status);
  if (!$stmt->fetch()) return false;
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

function edit_user_info($fname, $lname, $email, $address, $phone) {
  $conn = util\make_db_connection();
  try {
    $cid = util\expect_session_entry('cid');
  } catch (Exception $e) {
    return false;
  }

  $stmt = util\prepare_statement($conn,
    "UPDATE Customer" .
    "  SET FName = ?, LName = ?, Email = ?, Address = ?, Phone = ?" .
    "  WHERE CID = ?;");
  $stmt->bind_param('sssssi', $fname, $lname, $email, $address, $phone, $cid);
  if (!$stmt->execute()) return false;
  $stmt->close();

  $conn->close();
  return true;
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
  if (!$stmt->execute()) return false;
  
  $stmt->bind_result($pid, $ptype, $pname, $pprice, $descr, $pqty, $cputype,
    $btype, $weight, $printertype, $res);

  $result = array();
  while ($stmt->fetch()) {
    array_push($result, array(
      'pid' => $pid,
      'ptype' => $ptype,
      'pname' => $pname,
      'pprice' => $pprice,
      'descr' => $descr,
      'pqty' => $pqty,
      'cputype' => $cputype,
      'btype' => $btype,
      'weight' => $weight,
      'printertype' => $printertype,
      'res' => $res,
    ));
  }
  $stmt->close();

  $conn->close();
  return $result;
}

function upsert_product($pid, $qty) {
  $conn = util\make_db_connection();
  try {
    $cid = util\expect_session_entry('cid');
  } catch (Exception $e) {
    return false;
  }

  $stmt = util\prepare_statement($conn,
    "CALL upsertProd(?, ?, ?);");
  $stmt->bind_param('iii', $cid, $pid, $qty);
  if (!$stmt->execute()) return false;
  $stmt->close();

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
    "  FROM Cart NATURAL JOIN Appears_in NATURAL JOIN Product" . 
    "  WHERE CID = ? AND TStatus = 'Active';");
  $stmt->bind_param('i', $cid);
  if (!$stmt->execute()) return false;

  $stmt->bind_result($pid, $ptype, $pname, $pprice, $qty, $pqty);
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
  if (!$stmt->execute()) return false;

  $stmt->bind_result($addr_name);
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
  if (!$stmt->execute()) return false;

  $stmt->bind_result($ccnum);
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

  $stmt = util\prepare_statement($conn,
    "CALL placeOrder(?, ?, ?);");
  $stmt->bind_param('iss', $cid, $address_name, $ccnum);
  if (!$stmt->execute()) return false;
  $stmt->bind_result($result);
  if (!$stmt->fetch()) return false;
  $stmt->close();
  
  $conn->close();
  return $result === "WORKED";
}

function view_transactions($tstatus, $saname) {
  $conn = util\make_db_connection();
  try {
    $cid = util\expect_session_entry('cid');
  } catch (Exception $e) {
    return false;
  }

  $tstatus_pat = "%{$tstatus}%";
  $saname_pat = "%{$saname}%";

  $stmt = util\prepare_statement($conn,
    "SELECT CartID, SAName, CCNumber, TStatus, TDate" .
    "  FROM Cart" .
    "  WHERE CID = ? AND TStatus != 'Active'" .
    "    AND SAName LIKE ? AND TStatus LIKE ?" .
    "  ORDER BY TDate DESC;");
  $stmt->bind_param('iss', $cid, $saname_pat, $tstatus_pat); 
  if (!$stmt->execute()) return false;

  $stmt->bind_result($cartid, $saname, $ccnum, $tstatus, $tdate);
  $result = array();
  while ($stmt->fetch()) {
    array_push($result, array(
      'cartid' => $cartid,
      'saname' => $saname,
      'ccnum' => $ccnum,
      'tstatus' => $tstatus,
      'tdate' => $tdate,
    ));
  }
  $stmt->close();

  $conn->close();
  return $result;
}

function get_freq_prods($sdate, $edate) {
  $conn = util\make_db_connection();

  // TODO: Check that this is right.
  //       I think the subquery may not need the Product JOIN.
  $stmt = util\prepare_statement($conn,
    "SELECT PID, PName, SUM(Qty)" .
    "  FROM Cart NATURAL JOIN Appears_in NATURAL JOIN Product" .
    "  WHERE TStatus != 'Active' AND TDate BETWEEN ? AND ?" .
    "  GROUP BY PID" .
    "  HAVING SUM(Qty) >= ALL(SELECT Sum(Qty)" .
    "                           FROM Cart NATURAL JOIN Appears_in" .
    "                             NATURAL JOIN Product" .
    "                           WHERE TStatus != 'Active'" .
    "                             AND TDate BETWEEN ? AND ?" .
    "                           GROUP BY PID);");
  $stmt->bind_param('ss', $sdate, $edate, $sdate, $edate);
  if (!$stmt->execute()) return false;

  $stmt->bind_result($pid, $pname, $freq);
  $result = array();
  while ($stmt->fetch()) {
    array_push($result, array(
      'pid' => $pid,
      'pname' => $pname,
      'freq' => $freq,
    ));
  }
  $stmt->close();

  $conn->close();
  return $result;
}

function get_pop_prods($sdate, $edate) {
  return array();
}

function get_best_customers($sdate, $edate) {
  return array();
}

function get_best_zipcodes($sdate, $edate) {
  return array();
}

function get_avg_pprice($sdate, $edate) {
  return array();
}

?>
