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

require("_database.php");
require("_functions.php");
require("_layout.php");

validateUserCookie(true /* redirect if invalid */);
?>
<html>
<title>Gearpad</title>
<head>
<style type="text/css">
<? include("_style_base.php"); ?>

body {
  margin:0;
  overflow:hidden;
}

textarea {
  position:absolute; 
  overflow:auto; 
  left:2px; 
  top:1em; 
  right:2px; 
  bottom:1em; 
  margin-top:8px; 
  margin-bottom:2px; 
  border:1px solid #ccc;
}

iframe {
  height:200px;
}

#setup-offline a {
  color:red!important;
  font-weight:bold!important;
}

#status {
  position:absolute;
  bottom:0px;
  left:2px;
}

#contact {
  position:absolute;
  bottom:0px;
  right:2px;
}
</style>

<!--[if IE]>
<style type="text/css">
/*
  IE specific styles: override the definition of the elements to make them
  stretch right on IE's broken layout engine
*/
textarea {
  position:static;
  margin-top:1px;
  margin-left:2px;
  width:expression(document.body.offsetWidth - 6);
  height:expression(document.body.offsetHeight - 
          (document.getElementById("status") ? 
                       document.getElementById("status").offsetHeight : 0) -
          this.offsetTop - 5);
}

iframe {
  height:expression(document.body.offsetHeight - this.offsetTop);
}
</style>
<![endif]-->

</head>
<body onload="init()">

<? masthead(); ?>

<textarea></textarea>

<div id="status" class="green">Ready.</div>
<div id="contact">
  <a href="mailto:gearpad@example.com?subject=Gearpad">Contact</a>
</div>

<script type="text/javascript" src="base.js"></script>
<script type="text/javascript" src="cookies.js"></script>
<script type="text/javascript" src="xhr.js"></script>
<script type="text/javascript" src="utils.js"></script>
<script type="text/javascript" src="datastore.js"></script>
<script type="text/javascript" src="note.js"></script>

</body>
</html>
