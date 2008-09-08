// Copyright 2008, Google Inc.
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

var dummy = google.gears.factory.create('beta.dummymodule');

function testDummyCreation() {
  assertNotNull(dummy, 'Dummy module should be a value');
}

function testDummyProperty() {
  assert(!dummy.property, 'Initial value should be "false"');
  // assign a value and test to see if it holds
  dummy.property = true;
  assert(dummy.property, 'Dummy property value should be 5');
}

function testDummyMethod() {
  // should throw an exception if no arguments are given
  assertError(function() {
    dummy.method();
  }, null, 'Dummy method first argument should be required');
  // should return -1 if the second argument is not supplied
  assert(dummy.method('test') == -1, 'Dummy method should return -1');
  // should return second argument value if supplied
  assert(dummy.method('text', 1972) == 1972, 'Dummy method should return 1972');
}
