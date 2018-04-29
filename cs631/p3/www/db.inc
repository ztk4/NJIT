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
  // TODO: SQL

  return true;
}

function login($email) {
  $conn = util\make_db_connection();
  // TODO: SQL
  
  $_SESSION['cid'] = $cid;  // TODO: fill in cid.

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
    // TODO: SQL
  } catch (Exception $e) {
    return false;
  }

  return array();
}

function get_all_products() {
  $conn = util\make_db_connection();
  // TODO: SQL
  
  return array();
}

function upsert_product($pid, $qty) {
  $conn = util\make_db_connection();
  try {
    $cid = util\expect_session_entry('cid');
    // TODO: SQL
  } catch (Exception $e) {
    return false;
  }

  return true;
}

function view_cart() {
  $conn = util\make_db_connection();
  try {
    $cid = util\expect_session_entry('cid');
    // TODO: SQL
  } catch (Exception $e) {
    return false;
  }

  return array();
}

function view_addresses() {
  $conn = util\make_db_connection();
  try {
    $cid = util\expect_session_entry('cid');
    // TODO: SQL
  } catch (Exception $e) {
    return false;
  }

  return array();
}

function view_ccnums() {
  $conn = util\make_db_connection();
  try {
    $cid = util\expect_session_entry('cid');
    // TODO: SQL
  } catch (Exception $e) {
    return false;
  }

  return array();
}

function place_order($address_name, $ccnum) {
  $conn = util\make_db_connection();
  try {
    $cid = util\expect_session_entry('cid');
    // TODO: SQL
  } catch (Exception $e) {
    return false;
  }

  return true;
}

function view_transactions($tstatus, $tdate) {
  $conn = util\make_db_connection();
  try {
    $cid = util\expect_session_entry('cid');
    // TODO: SQL
  } catch (Exception $e) {
    return false;
  }

  return array();
}

function get_stats($sdate, $edate) {
  $conn = util\make_db_connection();
  // TODO: SQL
  
  return array();
}

?>
