var db = google.gears.factory.create('beta.database');
db.open('database_noworker_tests');

// On IE, the specialEmpty value below will be a NULL BSTR.  This is
// supposed to be treated the same as the empty string, but some code
// dereferenced it.
function testSpecialEmptyString() {
  // This one cannot run in a worker
  if (typeof document == 'undefined') {
    return;
  }

  var elm = document.createElement('textarea');
  document.body.appendChild(elm);
  var specialEmpty = elm.value;

  assertError(function() {
    // NULL-dereference crash.
    db.execute(specialEmpty).close();
  });

  // Sanity check for same bug in bind parameters (there wasn't
  // one).
  db.execute('select ?', [specialEmpty]).close();

  handleResult(db.execute('select ? as field', ['value']), function(rs) {
    assertError(function() {
      // NULL-dereference crash.
      rs.fieldByName(specialEmpty);
    });
  });
}

function testLongDBNames() {
  
  var testDB = google.gears.factory.create('beta.database');
  
  // 65 chars
  var extraLongDBName = 'aVeryLongDBNameThatShouldFailaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa';
  try {
    testDB.open(extraLongDBName);
    assert(false,'DB should have failed on long name');
  } catch (error) {
  }
  
  testDB.close();
  
  //64 chars
  var maxLengthDBName = 'thisDBHasTheLongestValidNameThatCanBeCreatedByGearsaaaaaaaaaaaaa';
  try {
    testDB.open(maxLengthDBName);
  } catch (error) {
    assert(false,'DB should have succeeded in creating 64 char db name');
  }
  
  testDB.close();
}
