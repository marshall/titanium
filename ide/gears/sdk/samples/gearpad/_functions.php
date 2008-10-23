<?
// Copyright 2007, Google Inc.
//
// Redistribution and use in source and binary forms, with or without 
// modification, are permitted provided that the following conditions are met:
//
//  1. Redistributions of source code must retain the above copyright notice, 
//     this list of conditions and the following disclaimer.
//  2. Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//  3. Neither the name of Google Inc. nor the names of its contributors may be
//     used to endorse or promote products derived from this software without
//     specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
// EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
// OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// Utility functions used by rest of Gearpad php pages.


//==============================================================================
// Custom error handling.
//==============================================================================
// Since we're using AJAX, the status code matter to us.
// Usually PHP just returns 200 for everything since it isn't buffering, it 
// too late to change the status by the time it figures out there's a problem.
//
// So, we need to buffer the output and errors we encounter and set the right
// status code if we get an error.
//==============================================================================
function handleError($code, $desc, $file, $line) {
  if ($code == E_NOTICE) {
    return;
  }

  ob_clean();
  header("Content-type: text/plain");
  header("HTTP/1.1 500 Internal Server Error");
  print("ERROR: $code $desc. $file:$line");
  exit();
}

ob_start();
set_error_handler("handleError");



//==============================================================================
// Cookie handling.
//==============================================================================
// This cookie policy is not secure and just for demo. In a real application
// you'd want to encrypt the create time with the cookie and invalidate old
// cookies on the server.
//==============================================================================

$SITE_SECRET = "gears-demo";

function setUserCookie($userid, $email) {
  global $SITE_SECRET;

  $hash = md5($userid.'-'.$SITE_SECRET);
  $val = $userid.'-'.$hash.'-'.$email;

  // store the cookie for 30 days
  setcookie('c', $val, time() + 60*60*24*30, '/');
}

function validateUserCookie($redirectIfInvalid = false) {
  global $SITE_SECRET;

  $raw = $_COOKIE['c'];

  if (!$raw) {
    handleInvalidUserCookie($redirectIfInvalid);
  }

  $vals = explode('-', $raw);

  if (count($vals) != 3) {
    return false;
  }

  $userid = $vals[0];
  $hash = $vals[1];

  if ($hash != md5($userid.'-'.$SITE_SECRET)) {
    handleInvalidUserCookie($redirectIfInvalid);
  }

  return $userid;
}

function handleInvalidUserCookie($redirectIfInvalid) {
  if ($redirectIfInvalid) {
    header('Location: login.php');
  } else {
    header("HTTP/1.1 403 Forbidden");
  }
  exit();
}


//==============================================================================
// Account management
//==============================================================================

function updateNote($id, $client, $version, $content) {
  $id = db_escape($id);
  $client = db_escape($client);
  $version = db_escape($version);
  $content = db_escape($content);

  // We allow the update if the version the client is sending is the current
  // version OR the client is the last client we spoke with
  $rslt = db_query_set(
      "update user set
         content='$content', version=version+1, last_client_id='$client' 
       where id = '$id' and 
         (last_client_id = '$client' or version = '$version') and
         @last_version := version"); // Assign the version before the update
                   // to a mysql variable. We then read this
                   // variable with the next SELECT statement.
                   // This ensures atomicy since mysql
                   // variables are per-connection and our
                   // connections are per-page-view.
  $num_rows = mysql_affected_rows();
  
  if ($num_rows == 1) {
    $rslt = firstRow(db_query_get("select @last_version + 1 as version"));
    $rslt['conflict'] = false;
  } else {
    $rslt = firstRow(db_query_get(
        "select version, content from user where id = '$id'"));
    $rslt['conflict'] = true;
  }

  return $rslt;
}

function createAccount($email, $password) {
  $email = db_escape($email);
  $password = crypt(db_escape($password));
  $userid = db_query_set(
      "insert into user (email, password, version, content) 
       values ('$email', '$password', 1, 'Hello, world!')");

  // $email was already in use
  if (!$userid) {
    return false;
  }

  setUserCookie($userid, $email);
  return $userid;
}

function login($email, $password) {
  $email = db_escape($email);

  $rslt = firstRow(db_query_get("select id, password from user
                                 where email = '$email'"));

  if (!$rslt) {
    return 'email';
  }

  if ($rslt['password'] != crypt($password, $rslt['password'])) {
    return 'password';
  }

  setUserCookie($rslt['id'], $email);
  return $rslt['id'];
}

function mailPassword($email) {
  global $SITE_SECRET;

  $token = crypt($email . $SITE_SECRET);
  $token = str_replace(array('.','/'), array('-','_'), $token);

  $path = explode('/', $_SERVER['SCRIPT_NAME']);
  array_pop($path);
  array_push($path, 'resetpass.php');

  mail($email, 
       'Account reset for Gearpad',
       "Click below to reset your account:\n\n"
       . 'http://' . $_SERVER['SERVER_NAME'] . implode('/', $path) .
       '?email=' . urlencode($email) . '&token=' . $token . "\n",
       'From: Gearpad <gearpad@example.com>\r\n');

  return true;
}

function resetPassword($email, $token, $newpass) {
  global $SITE_SECRET;

  $token = str_replace(array('-','_'), array('.','/'), $token);
  $check = crypt($email . $SITE_SECRET, $token);

  if ($check != $token) {
    return false;
  }

  $newpass = db_escape($newpass);
  $newpass = crypt($newpass);

  db_query_set("update user set password = '$newpass' where email = '$email'");
  $rslt = firstRow(db_query_get("select id from user where email = '$email'"));
  setUserCookie($rslt['id'], $email);

  return true;
}
?>
