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

var db_manager = google.gears.factory.create('beta.databasemanager');

var startTime = new Date().getTime();
var counter = 0;

function getNewDatabaseName() {
  return "unit_test_db_" + startTime + "_" + ++counter;
}

function testDatabaseManagerCreation() {
  assertNotNull(db_manager, 'Database manager should be a value');
}

function testDatabaseManagerApiSig() {
  var method = 'DatabaseManager.openDatabase()';
  assert(typeof db_manager.openDatabase != 'undefined',
         method + ' should be present');
  assertError(function() {
    db_manager.openDatabase();
  }, null, method + ' has required parameters');
  assertError(function() {
    db_manager.openDatabase('unit_test_db');
  }, null, method + ' requires a version parameter');
}

function testDatabaseOpening() {
  var db = db_manager.openDatabase('unit_test_db', '');
  assert(db, 'Database should be a value');
}

function testDatabaseOpenVersioned() {
  var name = getNewDatabaseName();
  var db1 = db_manager.openDatabase(name, '1.0');
  assertEqual('1.0', db1.version);
  // TODO(aa): Insert some data to test below.
  
  var db2 = db_manager.openDatabase(name, '1.0');
  // TODO(aa): Test the data that was created above.

  var db3 = db_manager.openDatabase(name, '');
  assertEqual('1.0', db3.version);
  // TODO(aa): Test the data that was created above.
  assert(db3, 'Database should be a value');
}

function testWeirdDatabaseNames() {
  var longName = '';
  for (var i = 0; i < 99; i++) {
    longName += 'x';
  }
  
  // This is meant to verify both that we can create databases with weird names,
  // and that they stay unique.
  var weirdNames = [
    longName,
    longName + 'x',
    'foo?',
    'foo',
    ' ',
    '',
    '/\:*?"<>|;'  // see base/common/string_utils.h - IsCharValidInPathComponent
  ]

  var baseName = getNewDatabaseName();
    
  for (var i = 0; i < weirdNames.length; i++) {
    var db = db_manager.openDatabase(baseName + weirdNames[i], '');
    // TODO(aa): Store the database's name in a text column inside the database.
  }

  for (var i = 0; i < weirdNames.length; i++) {
    var db = db_manager.openDatabase(baseName + weirdNames[i], '');
    // TODO(aa): Verify the name stored is correct.
  }
}

function testDatabaseVersionNull() {
  assertError(function() {
    var db = db_manager.openDatabase('unit_test_db', null);
  }, "Required argument 2 is missing.");
}

function testDatabaseApiSig() {
  withDb(function(db) {
    var method = 'Database2.transaction()';
    assert(typeof db.transaction != 'undefined',
           method + ' should be present');
    assertError(function() {
      db.transaction();
    }, null, method + ' requires a callback');
    
    var method = 'Database2.synchronousTransaction()';
    assert(typeof db.synchronousTransaction != 'undefined',
           method + ' should be present');
    
    var method = 'Database2.changeVersion()';
    assert(typeof db.changeVersion != 'undefined',
           method + ' should be present');
  });
}

// TODO(dimitri.glazkov): Uncomment when functionality implemented
//function testDatabaseTransaction() {
//  withDb(function(db) {
//    var method = 'Database2.transaction()';
//    var inFlight;
//    var outOfOrder = 0;
//    // this tests the fact that the transaction callback is called 
//    // asynchronously and whether the callback is invoked in a proper sequence
//    // if startAsync times out, the callback is not invoked at all
//    db.transaction(function(tx) {
//      inFlight = true;
//      outOfOrder--;
//      if (outOfOrder) {
//        assert(false, method + ' invokes callback out of sequence');
//      }
//      completeAsync();
//    });
//    outOfOrder++;
//    if (inFlight) {
//      assert(false, method + ' is not called asynchronously');
//    }
//    startAsync();

//    // TODO(aa): Remove when transaction() works.
//    completeAsync();
//  });
//}

// TODO(dimitri.glazkov): Uncomment when functionality implemented
// function testDatabaseSynchronousTransaction() {
//   withDb(function(db) {
//     var method = 'Database2.synchronousTransaction() ';
//     var inFlight;
//     var outOfOrder = 0;
//     // this tests the fact that the transaction callback is called 
//     // synchronously and whether the callback is invoked in a proper sequence
//     db.synchronousTransaction(function(tx) {
//       inFlight = true;
//       if (outOfOrder) {
//         assert(false, method + ' invokes callback out of sequence');
//       }
//     });
//     outOfOrder++;
//     if (!inFlight) {
//       assert(false, method + ' is not called synchronously');
//     }
//     
//     // test statement execution
//     db.synchronousTransaction(function(tx) {
//       var method = 'Database2Transaction.executeSql()';
//       var rs = tx.executeSql('SELECT * FROM Pages;');
//       assert(rs, method + ' should return a result set');
//     });
//   });
// }

function testStatementArguments() {
  withDb(function(db) {
    db.synchronousTransaction(function(tx) {
      // valid arguments
      tx.executeSql('SELECT * FROM Pages WHERE pageId = ? and version = ?',
        [ 1972, '1.0.0.0' ]);
      // arguments with null
      tx.executeSql('SELECT * FROM Pages WHERE pageId = ? and version = ?',
        [ 1972, null ]);
      // null arguments
      tx.executeSql('SELECT * FROM Pages', null);
      // invalid arguments: function
      assertError(function() {
        tx.executeSql('SELECT * FROM Pages WHERE pageId = ? and version = ?',
          [ function() {}, '1.0.0.0' ]);
      }, null, 'Function arguments are not allowed');
      // invalid arguments: object
      assertError(function() {
        tx.executeSql('SELECT * FROM Pages WHERE pageId = ? and version = ?',
          [ { you: 'is wrong' }, '1.0.0.0' ]);
      }, null, 'Function arguments are not allowed');
    });
  });
}

// TODO(dimitri.glazkov): Add tests for type fidelity, large integers, and
// unsupported types.
// TODO(dimitri.glazkov): Rework the tests to create/drop test tables with setup
// and tear-down methods.
function testSyncExecution() {
  withDb(function(db) {
    db.synchronousTransaction(function(tx) {
      var rs;
      tx.executeSql('CREATE TABLE IF NOT EXISTS Pages(' +
                    ' pageId INTEGER PRIMARY KEY,' +
                    ' version TEXT)');
      var pageId;
      rs = tx.executeSql('SELECT IFNULL(MAX(pageId) + 1, 1) AS max FROM Pages');
      assert(rs, 'Synchronous execute should return a result set');
      assert(rs.rows, 'result set must contain rows');
      assert(rs.rows.length, 'result set must contain one row');
      assert(rs.rows[0].max,
        'result set must return one non-zero int value with the key of "max"');
      pageId = rs.rows[0].max;
      tx.executeSql('INSERT INTO Pages(pageId, version) ' +
                    'VALUES(?,?)', [ pageId, 'test' ]);
      var rs = tx.executeSql('SELECT * FROM Pages WHERE pageId = ?',
        [ pageId ]);
      assert(rs.rows[0].pageId == pageId,
        'result set must return the last returned value');
    });
  });
}

// TODO(dimitri.glazkov): Uncomment when functionality implemented
//function testSQLTransactionApiSig() {
//  withDb(function(db) {
//    var method = 'SQLTransaction.executeSql';
//    db.transaction(function(tx) {
//      assert(typeof tx.executeSQL != 'undefined',
//             method + ' should be present');
//    });
//  });
//}

function withDb(fn, version) {
  db_manager && fn && fn.call(
    this, db_manager.openDatabase('unit_test_db', version || ''));
}

// NPAPI-TEMP - Disable tests that don't currently work in npapi build.
testStatementArguments._disable_in_npapi = true;
