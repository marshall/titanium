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

require("_functions.php");
require("_database.php");
require("_layout.php");

$mode = $_POST['mode'];
$email = trim($_POST['email']);
$password = trim($_POST['password']);
$password2 = trim($_POST['password2']);
$message = '';

function validate() {
  global $email, $password, $password2, $mode;

  if ($email == 'email') {
    $email = '';
  }

  if ($password == 'password') {
    $password = '';
  }

  if ($password2 == 'confirm password') {
    $password2 = '';
  }

  if (!$email ||
      $mode != 'forgot' && !$password ||
      $mode == 'create' && !$password2) {
    return '<span class="error">All fields are required.</span>';
  }

  if (strpos($email, "@") === false) {
    return '<span class="error">That\'s not a valid email address.</span>';
  }

  if ($mode == 'create' && $password != $password2) {
    return '<span class="error">Those passwords don\'t match.</span>';
  }
}

if ($mode) {
  $message = validate();

  if (!$message) {
    if ($mode == 'create') {
      if (!($userid = createAccount($email, $password))) {
        $message = '<span class="error">That email is already taken.</span>';
      }
    } else if ($mode == 'login') {
      $rslt = login($email, $password);

      if ($rslt == 'email') {
        $message = '<span class="error">No user with that email address.</span>';
      } else if ($rslt == 'password') {
        $message = '<span class="error">Wrong password.</span>';
      } else {
        $userid = $rslt;
      }
    } else if ($mode == 'forgot') {
      if (!mailPassword($email)) {
        $message = '<span class="error">No user with that email address.</span>';
      } else {
        $message = '<span class="ok">OK! Go check your email now.</span>';
      }
    }
  }
}

if (($mode == 'create' || $mode == 'login') && !$message) {
  header('Location: .');
  exit();
}
?>

<script type="text/javascript" src="base.js"></script>
<script type="text/javascript" src="cookies.js"></script>
<script type="text/javascript" src="utils.js"></script>

<html>
<head>
<style type="text/css">
<? include("_style_base.php"); ?>
<? include("_style_form.php"); ?>

#iesux {
  text-align:center;
}

#online-login, #offline-login {
  text-align:left;
  width:200px;
  margin:2em auto;
}

#offline-banner {
  border:1px solid #999;
  background:#eee;
  font-weight:bold;
  padding:0.5em 1em;
  margin-bottom:0.5em;
}

a {
  padding-right:0.5em;
}
</style>
</head>
<body style="margin:0;">

<? masthead('Login'); ?>

<div id="iesux">
<div id="online-login" style="display:none">
  <p><b class='green'>Gearpad</b> is a simple notepad you can access from anywhere, even offline.
  <p>When you return to the network, your notes will be synchronized with the server automatically. Hooray!

<div id="head"></div>
<?= $message ?>
<div id="clip" style="position:relative; overflow:hidden;" 
  onkeypress="if (event.keyCode == 13 && presubmit()) document.forms[0].submit()">
<input id="email" type="text" label="email" value="<?= $email ?>"><br>
<input id="password" type="password" label="password"><br>
<input id="password2" type="password" label="confirm password">
</div>
<form method="post" style="margin:0" action="login.php" onsubmit="return presubmit()">
<input type="submit" value="OK!" style="width:auto;"><br><br>
<a href="#" onclick="createMode(); return false;">Create account</a
><a href="#" onclick="forgotMode(); return false;">Forgot password</a
><a href="#" onclick="loginMode(); return false;">Log in</a>
<input name="email" type="hidden">
<input name="password" type="hidden">
<input name="password2" type="hidden">
<input name="mode" type="hidden">
</form>
</div>

<div id="offline-login" style="display:none">
  <div id="offline-banner" class="red">You are logging in offline.</div>
  <p>Your notes will be synchronized with the server the next time you 
      connect.</p>
  <br>
  <select id="users" size="1" style="width:100%" onchange="offlineUserChosen()">
    <option value="-1">Select a user...</option>
  </select>
  <br><br>
  <a href="#" onclick="location.reload()">Try online login again...</a>
</div>
</div>

<script type="text/javascript" src="accelimation.js"></script>
<script type="text/javascript" src="labels.js"></script>
<script type="text/javascript">
  var clip;
  var fields;
  var head;
  var navLinks;
  var form;
  var mode;
  var gears = new Gears();

  if (!gears.canGoLocal) {
    onlineLogin();
  } else {
    var img = new Image();
    img.onload = onlineLogin;
    img.onerror = offlineLogin;
    img.src = "x.gif?r=" + new Date().getTime();
    var timerId = window.setTimeout(offlineLogin, 5000);
  }

  function offlineLogin() {
    if (timerId) {
      window.clearTimeout(timerId);
    }

    DOM.getElementById("offline-login").style.display = "block";

    var users = gears.executeToObjects('select cookie from user');
    var select = DOM.getElementById("users");

    for (var i = 0, user; user = users[i]; i++) {
      var opt = document.createElement('option');
      var email = user.cookie.split('-')[2];
      opt.appendChild(document.createTextNode(email));
      opt.value = user.cookie;
      select.appendChild(opt);
    }
  }

  function offlineUserChosen() {
    var select = DOM.getElementById("users");
    var option = select.options[select.selectedIndex];

    createCookie('c', option.value, 30);
    location.href = ".";
  }

  function onlineLogin() {
    if (timerId) {
      window.clearTimeout(timerId);
    }

    DOM.getElementById("online-login").style.display = "block";
    clip = DOM.getElementById("clip");
    fields = DOM.getElementsByTagName(clip, "input");
    head = DOM.getElementById("head");
    form = DOM.getElementsByTagName(document, 'form')[0];
    navLinks = DOM.getElementsByTagName(form, 'a');
    mode = "<?= $mode ?>" || "login";

    setupLabels(fields);

    if (mode == "create") {
      createMode(true);
    } else if (mode == "login") {
      loginMode(true);
    } else if (mode == "forgot") {
      forgotMode(true);
    }
  }

  function presubmit() {
    try {
      form.email.value = DOM.getElementById("email").value;

      if (form.mode.value == "login") {
        form.password.value = DOM.getElementById("password").value;
        form.password2.value = "";
      }

      if (form.mode.value == "create") {
        form.password.value = DOM.getElementById("password").value;
        form.password2.value = DOM.getElementById("password2").value;
      }

      return true;
    } catch (e) {
      alert(e.lineNumber + ": " + e.message);
      return false;
    }
  }

  function loginMode(fast) {
    head.innerHTML = "<b>Log in</b>:";
    form.mode.value = "login";

    navLinks[0].style.display = "";
    navLinks[1].style.display = "";
    navLinks[2].style.display = "none";

    changeMode(1, fast);
  }

  function createMode(fast) {
    head.innerHTML = "<b>Create an account</b>:";
    form.mode.value = "create";

    navLinks[0].style.display = "none";
    navLinks[1].style.display = "none";
    navLinks[2].style.display = "";

    changeMode(2, fast);
  }

  function forgotMode(fast) {
    head.innerHTML = "<b>Forgot your password?</b>:";
    form.mode.value = "forgot";

    navLinks[0].style.display = "none";
    navLinks[1].style.display = "none";
    navLinks[2].style.display = "";

    changeMode(0, fast);
  }

  function changeMode(elmIdx, fast) {
    for (var i = 0; i < fields.length; i++) {
      if (i <= elmIdx) {
        fields[i].style.display = "";
      } else {
        fields[i].style.display = "none";
      }
    }

    var elm = fields[elmIdx];
    var h = elm.offsetTop + elm.offsetHeight;

    if (fast || DOM.is_pocket_ie) {
      clip.style.height = h + "px";
    } else {
      var a = new Accelimation(clip.style, "height", h, 150, 2, "px");
      a.start();
    }
  }
</script>
</body>
</html>
