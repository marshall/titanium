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


// Test the ability of Gears to coerce parameters to other types

// Coerce to boolean
function testCoerceToBool() {
  if (isDebug) {
    var internalTests = google.gears.factory.create('beta.test');

    // null and undefined should never be coerced
    assertError(function () {
        internalTests.testCoerceBool(undefined, false); }, null,
        'undefined should cause an error.');
    assertError(function () {
        internalTests.testCoerceBool(null, false); }, null,
        'null should cause an error.');
    // false
    assert(internalTests.testCoerceBool(false, false),
        'false should coerce to false.');
    assert(internalTests.testCoerceBool(0, false),
        '0 should coerce to false.');
    assert(internalTests.testCoerceBool(0.0, false),
        '0.0 should coerce to false.');
    assert(internalTests.testCoerceBool("", false),
        '"" should coerce to false.');
    assert(internalTests.testCoerceBool(NaN, false),
        'NaN should coerce to false.');
    assert(internalTests.testCoerceBool(Boolean(false), false),
        'Boolean(false) should coerce to false.');
    // true
    assert(internalTests.testCoerceBool(true, true),
        'true should coerce to true.');
    assert(internalTests.testCoerceBool(1, true),
        '1 should coerce to true.');
    assert(internalTests.testCoerceBool(-1, true),
        '-1 should coerce to true.');
    assert(internalTests.testCoerceBool(88.35, true),
        '88.35 should coerce to true.');
    assert(internalTests.testCoerceBool(123e-2, true),
        '88.35 should coerce to true.');
    assert(internalTests.testCoerceBool({}, true),
        '{} should coerce to true.');
    assert(internalTests.testCoerceBool(function () { return; }, true),
        'function () { return; } should coerce to true.');
    assert(internalTests.testCoerceBool("test", true),
        '"test" should coerce to true.');
    assert(internalTests.testCoerceBool("false", true),
        '"false" should coerce to true.');
    assert(internalTests.testCoerceBool("False", true),
        '"False" should coerce to true.');
    assert(internalTests.testCoerceBool("true", true),
        '"true" should coerce to true.');
    assert(internalTests.testCoerceBool("True", true),
        '"True" should coerce to true.');
    // Boxed types
    assert(internalTests.testCoerceBool(Boolean(true), true),
        'Boolean(true) should coerce to true.');
    assert(internalTests.testCoerceBool(Number(3.4), true),
        'Number(3.4) should coerce to true.');
    assert(internalTests.testCoerceBool(String("test"), true),
        'String("test") should coerce to true.');
  }
}

// Coerce to integer
function testCoerceToInt() {
  if (isDebug) {
    var internalTests = google.gears.factory.create('beta.test');

    // null and undefined should never be coerced
    assertError(function () {
        internalTests.testCoerceInt(undefined, 0); }, null,
        'undefined should cause an error.');
    assertError(function () {
        internalTests.testCoerceInt(null, 0); }, null,
        'null should cause an error.');
    // everything else
    assert(internalTests.testCoerceInt(1, 1),
        '1 should coerce to 1.');
    assert(internalTests.testCoerceInt(3.141, 3),
        '3.141 should coerce to 3.');
    assert(internalTests.testCoerceInt(-99.49, -99),
        '-99.49 should coerce to -99.');
    assert(internalTests.testCoerceInt(99.9999, 99),
        '99.9999 should coerce to 99.');
    assert(internalTests.testCoerceInt(123e-2, 1),
        '123e-2 should coerce to 1.');
    assert(internalTests.testCoerceInt("1", 1),
        '"1" should coerce to 1.');
    assert(internalTests.testCoerceInt("-99.9999", -99),
        '"-99.9999" should coerce to -99.');
    assert(internalTests.testCoerceInt(true, 1),
        'true should coerce to 1.');
    assert(internalTests.testCoerceInt(false, 0),
        'false should coerce to 0.');
    assert(internalTests.testCoerceInt("", 0),
        '"" should coerce to 0.');
    assertError(function () { internalTests.testCoerceInt("test", 0); },
        null, '"test" should cause an error.');
    assertError(function() { internalTests.testCoerceInt(NaN, 0); },
        null, 'NaN should cause an error.');
    assertError(function() { internalTests.testCoerceInt({}, 0); },
        null, '{} should cause an error.');
    assertError(function() {
        internalTests.testCoerceInt(function () { return; }, 0); }, null,
        'function () { return; } should cause an error.');
    // Boxed types
    assert(internalTests.testCoerceInt(Boolean(true), 1),
        'Boolean(true) should coerce to 1.');
    assert(internalTests.testCoerceInt(Number(3.4), 3),
        'Number(3.4) should coerce to 3.');
    assertError(function () {
          internalTests.testCoerceInt(String("test"), 0);
        }, null, 'String("test") should cause an error.');
  }
}
    
// Coerce to double
function testCoerceToDouble() {
  if (isDebug) {
    var internalTests = google.gears.factory.create('beta.test');

    // null and undefined should never be coerced
    assertError(function () {
        internalTests.testCoerceDouble(undefined, 0); }, null,
        'undefined should cause an error.');
    assertError(function () {
        internalTests.testCoerceDouble(null, 0); }, null,
        'null should cause an error.');
    // everything else
    assert(internalTests.testCoerceDouble(1, 1),
        '1 should coerce to 1.');
    assert(internalTests.testCoerceDouble(3.14159, 3.14159),
        '3.14159 should coerce to 3.14159.');
    assert(internalTests.testCoerceDouble(123e-2, 1.23),
        '123e-2 should coerce to 1.23.');
    assert(internalTests.testCoerceDouble("3.14159", 3.14159),
        '"3.14159" should coerce to 3.14159.');
    assert(internalTests.testCoerceDouble("123e-2", 1.23),
        '"123e-2" should coerce to 1.23.');
    assert(internalTests.testCoerceDouble(true, 1),
        'true should coerce to 1.');
    assert(internalTests.testCoerceDouble(false, 0),
        'false should coerce to 0.');
    assert(internalTests.testCoerceDouble("", 0),
        '"" should coerce to 0.');
    assertError(function () { internalTests.testCoerceDouble("test", 0); },
        null, '"test" should cause an error.');
    assertError(function() { internalTests.testCoerceDouble(NaN, 0); },
        null, 'NaN should cause an error.');
    assertError(function() { internalTests.testCoerceDouble({}, 0); },
        null, '{} should cause an error.');
    assertError(function() {
          internalTests.testCoerceDouble(function () { return; }, 0);
        }, null, 'function () { return; } should cause an error.');
    // Boxed types
    assert(internalTests.testCoerceDouble(Boolean(true), 1),
        'Boolean(true) should coerce to 1.');
    assert(internalTests.testCoerceDouble(Number(3.4), 3.4),
        'Number(3.4) should coerce to 3.4.');
    assertError(function () {
          internalTests.testCoerceDouble(String("test"), 0);
        }, null, 'String("test") should cause an error.');
  }
}

// Coerce to string
function testCoerceToString() {
  if (isDebug) {
    var internalTests = google.gears.factory.create('beta.test');

    // null and undefined should never be coerced
    assertError(function () {
        internalTests.testCoerceString(undefined, ""); }, null,
        'undefined should cause an error.');
    assertError(function () {
        internalTests.testCoerceString(null, ""); }, null,
        'null should cause an error.');
    // everything else
    assert(internalTests.testCoerceString("test", "test"),
        '"test" should coerce to "test".');
    assert(internalTests.testCoerceString(-12, "-12"),
        '-12 should coerce to "-12".');
    assert(internalTests.testCoerceString(3.14159, "3.14159"),
        '3.14159 should coerce to "3.14159".');
    assert(internalTests.testCoerceString(true, "true"),
        'true should coerce to "true".');
    assert(internalTests.testCoerceString(false, "false"),
        'false should coerce to "false".');
    assert(internalTests.testCoerceString(NaN, "NaN"),
        'NaN should coerce to "NaN".');
    assert(internalTests.testCoerceString({}, "[object Object]"),
        '{} should coerce to "[object Object]".');
    assert(internalTests.testCoerceString(
          { toString: function () { return "test"; } }, "test"),
        '{ toString: function () { return "test"; } } ' +
        'should coerce to "test".');
    // Boxed types
    assert(internalTests.testCoerceString(Boolean(true), "true"),
        'Boolean(true) should coerce to "true".');
    assert(internalTests.testCoerceString(Number(3.4), "3.4"),
        'Number(3.4) should coerce to "3.4".');
    assert(internalTests.testCoerceString(String("test"), "test"),
        'String("test") should coerce to "test".');
    // TODO(aa): This test passes in both IE and FF, but the actual
    // output is browser dependent (with respect to whitespace) so it's
    // difficult to write a test case for it. As it is written, the test
    // passes in FF. One can verify that it also passes in IE by replacing
    // all instances of '\n' with a space character (' '). I've left it in
    // for now to (1) prove that it works and (2) as reminder to others when
    // changing coercion code to check that it still works.
    /*
    assert(internalTests.testCoerceString(function () { },
          "function () {\n}"),
        'function () { } should coerce to "function () {\n}".');
    */
  }
}

// Test GetType method
function testGetType() {
  if (isDebug) {
    var internalTests = google.gears.factory.create('beta.test');

    assert(internalTests.testGetType("bool", true),
        'true should be a JSPARAM_BOOL.');
    assert(internalTests.testGetType("int", 1),
        '1 should be a JSPARAM_INT.');
    assert(internalTests.testGetType("double", 3.4),
        '3.4 should be a JSPARAM_DOUBLE.');
    assert(internalTests.testGetType("string", "test"),
        '"test" should be a JSPARAM_STRING16.');
    assert(internalTests.testGetType("null", null),
        'null should be JSPARAM_NULL.');
    assert(internalTests.testGetType("undefined", undefined),
        'undefined should be JSPARAM_UNDEFINED.');
    assert(internalTests.testGetType("array", [1, 2, 3]),
        '[1, 2, 3] should be a JSPARAM_ARRAY.');
    assert(internalTests.testGetType("function", function () { return; }),
        'function() { return; } should be a JSPARAM_FUNCTION.');
    assert(internalTests.testGetType("object", { test: "test" }),
        '{ test: "test" } should be a JSPARAM_OBJECT.');
  }
}
