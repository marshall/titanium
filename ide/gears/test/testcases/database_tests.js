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

var db = google.gears.factory.create('beta.database');
db.open('unittestdb');
db.execute('drop table if exists simple');
db.execute('create table simple ' +
           '(myvarchar varchar(255), ' +
           'myint int, ' +
           'myfloat float, ' +
           'mydouble double)');

function testInsertThenSelect() {
  db.execute('delete from simple');
  handleResult(db.execute('select count(*) from simple'), function(rs) {
    assertEqual(0, rs.field(0));
  });

  db.execute('insert into simple values (?, ?, ?, ?)',
             ['potato', 42, 88.8, 3.14]);

  handleResult(db.execute('select count(*) as count from simple'),
               function(rs) {
    assertEqual(1, rs.field(0));
  });

  handleResult(db.execute('select * from simple'), function(rs) {
    assertEqual(4, rs.fieldCount());
    assertEqual('potato', rs.field(0));
    assertEqual(42, rs.field(1));
    assertEqual(88.8, rs.field(2));
    assertEqual(3.14, rs.field(3));
    assert(!rs.next(), 'Expected no next row');
    assert(!rs.isValidRow());
  });
}

function testInsertTypePreservation() {
  db.execute('delete from simple');

  db.execute('insert into simple values (?, ?, ?, ?)',
             [42, 'foo', 'bar', 'baz']);

  handleResult(db.execute('select * from simple'), function(rs) {
    // 42 should have gotten coerced to a string because of the text type on the
    // column.
    assertEqual('42', rs.field(0)); 

    // The remaining values should remain numbers since they can be represented
    // as numbers.
    assertEqual('foo', rs.field(1)); 
    assertEqual('bar', rs.field(2));
    assertEqual('baz', rs.field(3));
  });
}

function testInsertNull() {
  db.execute('delete from simple');
  db.execute('insert into simple values (?, ?, ?, ?)',
             [null, null, null, null]);

  handleResult(db.execute('select * from simple'), function(rs) {
    for (var i = 0; i < 3; i++) {
      assertEqual(null, rs.field(i),
                  'Expected field number %s to be null'.subs(i));
    }
  });
}

function testInsertUndefined() {
  db.execute('delete from simple');
  db.execute('insert into simple values (?, ?, ?, ?)',
             [undefined, undefined, undefined, undefined]);

  handleResult(db.execute('select * from simple'), function(rs) {
    for (var i = 0; i < 3; i++) {
      // TODO(aa): I am not sure what this should do, but it definitely should
      // not do this. Maybe undefined is an illegal value to insert? Maybe it
      // gets converted to null?
      assertEqual('undefined', rs.field(i),
                  'Expected field number %s to be undefined'.subs(i));
    }
  });
}

function testFieldAccessors() {
  function fieldIsCorrect(rs, index, colName, value) {
    assert(rs.isValidRow(), 'Expected valid row');

    // check value by index, value by name, and type
    assertEqual(value, rs.field(index),
                'Incorrect value for field index %s'.subs(index));
    assertEqual(value, rs.fieldByName(colName),
                'Incorrect value for field name %s'.subs(colName));
    assertEqual(colName, rs.fieldName(index),
                'Incorrect name for column %s'.subs(index));
  }

  db.execute('delete from simple');
  db.execute('insert into simple values (?, ?, ?, ?)',
             ['foobar', 42, 33.3, 1123.58]);

  handleResult(db.execute('select * from simple'), function(rs) {
    assertEqual(4, rs.fieldCount(), 'Wrong number of fields');

    fieldIsCorrect(rs, 0, 'myvarchar', 'foobar');
    fieldIsCorrect(rs, 1, 'myint', 42);
    fieldIsCorrect(rs, 2, 'myfloat', 33.3);
    fieldIsCorrect(rs, 3, 'mydouble', 1123.58);
  });
}

function testCertainResultSetMethodsShouldAlwaysBeSafe() {
  function invokeResultSetMethods(rs) {
    // these methods should not throw for _any_ ResultSet
    rs.fieldCount();
    assert(!rs.isValidRow(), 'ResultSet should not be valid');
  }

  // Even if statement doesn't return results, should return a valid ResultSet.
  var TEMP_TABLE = 'Temporary1';
  var statements = [
    'drop table if exists ' + TEMP_TABLE,
    'create table ' + TEMP_TABLE + ' (foo int)',
    'select * from ' + TEMP_TABLE,
    'insert into ' + TEMP_TABLE + ' values (1)',
    'delete from ' + TEMP_TABLE
  ];

  for (var i = 0; i < statements.length; ++i) {
    handleResult(db.execute(statements[i]), function(rs) {
      invokeResultSetMethods(rs);
    });
  }
}

// TODO(aa): Following two functions should should throw, as if not enough
// params were passed.
function testUndefinedQueryParamThatFails() {
  db.execute('delete from simple');

  handleResult(db.execute('select * from simple where myint=?', [undefined]),
               function(rs) {
    assert(!rs.isValidRow(), 'Expected no results');
  });
}

function testUndefinedQueryParamThatWorks() {
  db.execute('delete from simple');
  db.execute('insert into simple values (?, ?, ?, ?)',
             [undefined, 4, 4, 4]);

  handleResult(
    db.execute('select * from simple where myvarchar=?', [undefined]),
    function(rs) {
      assertEqual('undefined', rs.field(0));
  });
}

function testArgsRequired() {
  assertError(function() {
    db.execute('select * from simple where id = ?');
  });
}

function testArgsForbidden() {
  assertError(function() {
    db.execute('select * from simple', [42]);
  });
}

function testArgsMustBeArray() {
  assertError(function() {
    // note missing array brackets []
    db.execute('select * from simple where id = ?', 42);
  });
}

function testEmptyArgsOK() {
  // this is OK - we're not actually passing any args
  var rs = db.execute('select * from simple', []);
  rs.close();
}

function testUndefinedArgumentsArray() {
  // If we pass <undefined> for the arguments array, that should be the same
  // as not passing it at all.
  var rs = db.execute('select * from simple', undefined);
  rs.close();
}

function testNullArgumentsArray() {
  // If we pass <null> for the arguments array, that should be the same
  // as not passing it at all.
  var rs = db.execute('select * from simple', null);
  rs.close();
}

function testWrongNumberOfArgs() {
  assertError(function() {
    var rs = db.execute('select * from simple where id=? and myvarchar=?',
                        [42]);
    rs.close();
  });
}

function testMultipleQueryParams() {
  db.execute('insert into simple values (?, ?, ?, ?)',
             ['bobo', 42, 88.8, 3.14]);

  handleResult(
    db.execute('select * from simple where myvarchar=? and myint=?',
               ['bobo', 42]),
    function(rs) {
      assert(rs.isValidRow(), 'Expected valid row');
      assertEqual('bobo', rs.field(0));
      assertEqual(42, rs.field(1));
      assertEqual(88.8, rs.field(2));
      assertEqual(3.14, rs.field(3));
  });
}

// test fix for bug #180262
function testIntegerRanges() {
  function testIntegerValue(value) {
    db.execute('delete from RangeTestTable');
    db.execute('insert into RangeTestTable values (' + value + ')');

    handleResult(db.execute('select * from RangeTestTable'), function(rs) {
      assertEqual(value, rs.field(0));
    });
  }

  db.execute('drop table if exists RangeTestTable');
  db.execute('create table RangeTestTable (i int)');

  testIntegerValue(-4294967296);  // -2^32
  testIntegerValue(-2147483649);  // -2^31 - 1
  testIntegerValue(-2147483648);  // -2^31       aka int32.min
  testIntegerValue(-2147483647);  // -2^31 + 1
  testIntegerValue(0);
  testIntegerValue(+2147483646);  //  2^31 - 2
  testIntegerValue(+2147483647);  //  2^31 - 1   aka int32.max
  testIntegerValue(+2147483648);  //  2^31
  testIntegerValue(0xfffffffe);   //  2^32 - 2
  testIntegerValue(0xffffffff);   //  2^32 - 1   aka uint32.max
  testIntegerValue(+4294967296);  //  2^32
  testIntegerValue(9007199254740989);   // large positive int64
  testIntegerValue(-9007199254740989);  // large negative int64
  // We dont explicitly test min/max possible int64 values because javascript
  // seems to represent those values as floats rather than ints and enough
  // precision is lost to mess up our test.
}

function testNullArguments() {
  var v1inserted = 0;
  var v2inserted = 'null';
  var v3inserted = null;
  db.execute('drop table if exists NullArgsTable');
  db.execute('create table NullArgsTable (v1 int, v2 text, v3)');
  db.execute('insert into NullArgsTable values (?, ?, ?)',
             [v1inserted, v2inserted, v3inserted]);

  handleResult(db.execute('select * from NullArgsTable'), function(rs) {
    assertEqual(v1inserted, rs.field(0));
    assertEqual(v2inserted, rs.field(1));
    assertEqual(v3inserted, rs.field(2));
  });
}

// This caught a crashing bug where JsContext wasn't set in a worker
// thread, when object1 called object2 which threw a JavaScript exception.
// In this case, object1 is a Database and object2 is a ResultSet, and
// the exception is thrown due to an error in the first call to
// sqlite3_step(), as opposed to sqlite3_prepare_v2().
function testValidStatementError() {
  assertError(function() {
    db.execute('rollback').close();
  });
}

function testPragmaSetDisabled() {
  assertError(function() {
    db.execute('PRAGMA cache_size = 1999').close();
  });
}

function testPragmaGetDisabled() {
  assertError(function() {
    db.execute('PRAGMA cache_size').close();
  });
}

// Causes a segmentation fault.
// TODO(shess) Fix things so that we can run this, again.  See bug 199.
/*
function testStackOverflow() {
  var q = 'select * from (select 1';
  for (var i = 0; i < 50000; ++i) {
    q += ' union select 1';
  }
  q += ')';
  assertError(function() {
    db.execute(q).close();
  });
}
*/

// Using VACUUM would cause corruption of fts2 tables.  We have
// auto_vacuum turned on, so VACUUM isn't really necessary.
function testVacuumDisabled() {
  assertError(function() {
    db.execute('VACUUM').close();
  });
}

function testCloseDatabase() {
  var db = google.gears.factory.create('beta.database');
  db.open('testclosedb');
  db.close();
}

function testCloseDatabaseTwice() {
  var db = google.gears.factory.create('beta.database');
  db.open('testclosedb');
  db.close();
  db.close();
}

function testExecuteMsecOnlyInDebug() {
  // property should exist iff this is a debug build
  assertEqual(Boolean(db.executeMsec), isDebug);
}

// Test that the optional database name works correctly for all variations.
// <undefined>, <null>, and not passing any arguments should all mean the
// same thing, on all APIs throughout Appcelerator Titanium.
function testOptionalDatabaseName() {
  var db1 = google.gears.factory.create('beta.database');

  // pass '' for the database name
  db1.open('');
  // insert a value
  db1.execute('drop table if exists TempTable');
  db1.execute('create table TempTable (int i, t text)');
  db1.execute('insert into TempTable' +
              ' values (1, "OptionalDatabaseName")');
  db1.close();
  
  function testDb(testName, opt_dbName) {
    db1.open(opt_dbName);

    // expect to find value inserted above
    handleResult(db1.execute('select * from TempTable'), function(rs) {
      assert(rs.isValidRow(),
             'Did not find test value in db "%s"'.subs(testName));
    });

    db1.close();
  }

  testDb('null', null);
  testDb('undefined', undefined);
  testDb('argument not passed');
}

function testInvalidDatabaseNameType() {
  var db1 = google.gears.factory.create('beta.database');

  function tryOpen(notAString) {
    assertError(function() {
      db1.open(notAString);
      db1.close();
    });
  }  
  
  tryOpen(42);
  tryOpen(true);
  tryOpen(new Date());
  tryOpen([]);
  
  return true;
}

function testSemicolonSeparatedStatementsShouldNotWork() {
  // this test simulates one large class of SQL injection attacks
  db.execute('drop table if exists TempTable');
  db.execute('create table TempTable' +
             ' (Username text, Email text, Password text)');
  db.execute('insert into TempTable values ("joe", "joe@a.com", "abcdef")');

  // simulate multi-part query attack
  var goodStmt = 'select Email from TempTable where Username="joe"';
  var evilStmt = 'update TempTable set Password="" where Username="joe"';
  var rs = db.execute(goodStmt + ' ; ' + evilStmt);
  rs.close();

  // see if the attack succeeded
  handleResult(db.execute('select * from TempTable where Username="joe"'),
               function(rs) {
    assertEqual('abcdef', rs.fieldByName('Password'), 'Database p0wn3d');
  });
}

function testAttachShouldNotWork() {
  assertError(function() {
    // TODO(shess) Find portable solution which won't put
    // gears_test.db in your root if this succeeds.
    db.execute('ATTACH DATABASE ? AS attached_db', ['/gears_test.db']);
  });

  assertError(function() {
    db.execute('DETACH DATABASE attached_db');
  });
}

function testAttachFnShouldNotWork() {
  assertError(function() {
    // TODO(shess) Find portable solution which won't put
    // gears_test.db in your root if this succeeds.
    db.execute('SELECT sqlite_attach(?, ?, NULL)', 
               ['/gears_test.db', 'attached_db']).close();
  });

  assertError(function() {
    db.execute('SELECT sqlite_detach(?)', ['attached_db']).close();
  });
}

function testOpenDatabaseWithIds() {
  function setupDB(id) {
    var db1 = google.gears.factory.create('beta.database');
    db1.open(String(id));
    db1.execute('drop table if exists xxx');
    db1.execute('create table xxx (id int auto_increment primary key, ' +
                'mydata int)');
    db1.execute('insert into xxx values (?, ?)', [id, id]);
    db1.close();
  }

  function testDB(id) {
    var db1 = google.gears.factory.create('beta.database');
    db1.open(String(id));

    handleResult(db1.execute('select * from xxx where id=?', [id]),
                 function(rs) {
      assertEqual(parseInt(id), rs.field(0),
                  'DB with id %s has wrong value for mydata'.subs(id));
    });

    db1.close();
  }

  setupDB(1);
  setupDB(2);
  setupDB(3);

  testDB(1);
  testDB(2);
  testDB(3);
}

function testOpenDatabaseWithIllegalIds() {
  function doIllegalIdTest(id) {
    var db1 = google.gears.factory.create('beta.database');
    assertError(function() {
      db1.open(id);
      db1.close();
    });
  }

  doIllegalIdTest('/');
  doIllegalIdTest('\\');
  doIllegalIdTest(':');
  doIllegalIdTest('*');
  doIllegalIdTest('?');
  doIllegalIdTest('"');
  doIllegalIdTest('<');
  doIllegalIdTest('>');
  doIllegalIdTest('|');
  doIllegalIdTest(';');
  doIllegalIdTest(',');
  doIllegalIdTest('.a_perfectly_valid_id_except_for_leading_dot');
  doIllegalIdTest('a_perfectly_valid_id_except_for_trailing_dot.');
  doIllegalIdTest('an id with spaces');
  doIllegalIdTest('an_id_with_an_eight_bit_char�cter');
}

function testSimultaneousDatabaseOpens() {
  function useDB(db) {
    db.execute('drop table if exists TestTable');
    db.execute('create table TestTable (i int)');
  }

  var db1 = google.gears.factory.create('beta.database');
  db1.open('foo');
  useDB(db1);

  var db2 = google.gears.factory.create('beta.database');
  db2.open('bar');
  useDB(db2);

  db2.close();
  db1.close();
}

function testLastInsertRowId() {
  db.execute('drop table if exists lastInsertRowId');
  db.execute('create table lastInsertRowId (' +
             'id int auto_increment primary key, ' +
             'myint int)');

  for (var i = 1; i < 10; ++i) {
    db.execute('insert into lastInsertRowId (id, myint) values ' +
               '(' + i + ', ' + i * 100 + ')');

    assertEqual(i, db.lastInsertRowId, 
                'Didn\'t find expected last insert rowid');
  }
}

function testRowsAffected() {
  db.execute('delete from simple');
  db.execute('insert into simple values (?, ?, ?, ?)',
             ['potato', 10, 3.14, 2.72]);
  db.execute('insert into simple values (?, ?, ?, ?)',
             ['tomato', 10, 1.0, 1.1]);
  assertEqual(1, db.rowsAffected);
  db.execute('update simple set myint = 20 where myint = 10');
  assertEqual(2, db.rowsAffected);
  db.execute('begin');
  db.execute('delete from simple');
  assertEqual(0, db.rowsAffected);
  db.execute('rollback');
  db.execute('delete from simple where 1');
  assertEqual(2, db.rowsAffected);
}
