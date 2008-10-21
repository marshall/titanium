var db = google.gears.factory.create('beta.database');
db.open('fts_test');
db.execute('drop table if exists foo');
db.execute('create virtual table foo using fts1(content)');
db.execute('drop table if exists foo2');
db.execute('create virtual table foo2 using fts2(content)');

function testFulltextIndexingFTS1() {
  db.execute('delete from foo');
  db.execute('insert into foo (content) values ' +
             '("to sleep perchance to dream")');
  db.execute('insert into foo (content) values ("to be or not to be")');

  handleResult(db.execute('select * from foo where content match "dream"'),
               function(rs) {
    assert(rs.isValidRow(), 'Fulltext match returned no results');
    assertEqual('to sleep perchance to dream', rs.field(0),
                'Fulltext match statement failed');
  });
}

function testFulltextIndexingByRowIdFTS1() {
  db.execute('delete from foo');
  db.execute('insert into foo(rowid, content) values(12345, "hello")');

  handleResult(db.execute('select content from foo where rowid=12345'),
               function(rs) {
    assert(rs.isValidRow(), 'select by rowid returned no results');
    assertEqual('hello', rs.field(0), 'select by rowid failed');
  });

  handleResult(
    db.execute('select content from foo where content match "hello"'),
    function(rs) {
      assert(rs.isValidRow(), 'match rowid-inserted value returned no results');
      assertEqual('hello', rs.field(0), 'match rowid-inserted value failed');
  });

  // TODO(aa): Is there any reason we are repeating the same test twice?
  handleResult(db.execute('select content from foo where rowid=12345'),
               function(rs) {
    assert(rs.isValidRow(), 'select by rowid returned no results');
    assertEqual('hello', rs.field(0), 'select by rowid failed');
  });
}

function testFulltextIndexingRowIdChangesFTS1() {
  // This test is known not to work in the current version of Cinnamon.
  // Simply put, updates don't work.
  //
  // TODO(miket): remove following line when Cinnamon can handle updates.
  return;

  db.execute('delete from foo');
  db.execute('insert into foo(rowid, content) values(12345, "hello")');
  db.execute('update foo set rowid=23456 where rowid=12345');

  handleResult(db.execute('select content from foo where rowid=12345'),
               function(rs) {
    assert(rs.isValidRow(), 'rowid update didn\'t take');
  });

  handleResult(db.execute('select content from foo where rowid=23456'),
               function(rs) {
    assert(rs.isValidRow(), 'select by modified rowid returned no results');
    assertEqual('hello', rs.field(0), 'select by modified rowid failed');
  });

  handleResult(db.execute('select rowid from foo where content match "hello"'),
               function(rs) {
    assert(rs.isValidRow(),
           'select modified rowid by match returned no results');
    assertEqual(23456, rs.field(0), 'select modified rowid by match failed');
  });
}

function testFulltextIndexingFTS2() {
  db.execute('delete from foo2');
  db.execute('insert into foo2 (content) values ' +
             '("to sleep perchance to dream")');
  db.execute('insert into foo2 (content) values ("to be or not to be")');

  handleResult(db.execute('select * from foo2 where content match "dream"'),
               function(rs) {
    assert(rs.isValidRow(), 'Fulltext match statement returned no results');
    assertEqual('to sleep perchance to dream', rs.field(0));
  });
}

function testFulltextIndexingByRowIdFTS2() {
  db.execute('delete from foo2');
  db.execute('insert into foo2(rowid, content) values(12345, "hello")');

  handleResult(db.execute('select content from foo2 where rowid=12345'),
               function(rs) {
    assert(rs.isValidRow(), 'select by rowid returned no results');
    assertEqual('hello', rs.field(0), 'select by rowid failed');
  });

  handleResult(
    db.execute('select content from foo2 where content match "hello"'),
    function(rs) {
      assert(rs.isValidRow(), 'match rowid-inserted value returned no results');
      assertEqual('hello', rs.field(0), 'match rowid-inserted value failed');
  });

  handleResult(db.execute('select content from foo2 where rowid=12345'),
               function(rs) {
    assert(rs.isValidRow(), 'select by rowid returned no results');
    assertEqual('hello', rs.field(0), 'select by rowid failed');
  });
}

// fts2 was throwing assertions when inserting with rowid<=0.
function testFulltextIndexingRowidZeroFTS2() {
  db.execute('delete from foo2');
  db.execute('insert into foo2(rowid, content) values(0, "bother")');

  handleResult(db.execute('select content from foo2 where rowid=0'),
               function(rs) {
    assert(rs.isValidRow(), 'select by rowid returned no results');
    assertEqual('bother', rs.field(0), 'select by rowid failed');
  });

  db.execute('insert into foo2(rowid, content) values(-1, "bother")')
  handleResult(db.execute('select content from foo2 where rowid=-1'),
               function(rs) {
    assert(rs.isValidRow(), 'select by rowid returned no results');
    assertEqual('bother', rs.field(0), 'select by rowid failed');
  });

  handleResult(
    db.execute('select rowid from foo2 where content match "bother" ' +
               'order by rowid'),
    function(rs) {
      assert(rs.isValidRow(), 'match rowid-inserted value returned no results');
      assertEqual(-1, rs.field(0), 'match rowid-inserted first value wrong');

      rs.next();
      assert(rs.isValidRow(), 'expected two results');

      assertEqual(0, rs.field(0), 'match rowid-inserted second value wrong');
  });
}

function testFulltextIndexingRowIdChangesFTS2() {
  // This test is known not to work in the current version of Cinnamon.
  // Simply put, updates don't work.
  //
  // TODO(miket): remove following line when Cinnamon can handle updates.
  return;

  db.execute('delete from foo2');
  db.execute('update foo2 set rowid=23456 where rowid=12345');

  handleResult(db.execute('select content from foo2 where rowid=12345'),
               function(rs) {
    assert(rs.isValidRow(), 'rowid update didn\'t take');
  });

  handleResult(db.execute('select content from foo2 where rowid=23456'),
               function(rs) {
    assert(rs.isValidRow(), 'select by modified rowid returned no results');
    assertEqual('hello', rs.field(0), 'select by modified rowid failed');
  });

  handleResult(db.execute('select rowid from foo2 where content match "hello"'),
               function(rs) {
    assert(rs.isValidRow(), 
           'select modified rowid by match returned no results');
    assertEqual(23456, rs.field(0), 'select modified rowid by match failed');
  });
}

function testFulltextIndexingSnippet() {
  // Test that SQLite ticket #2429 is fixed.
  // http://www.sqlite.org/cvstrac/tktview?tn=2429
  db.execute('drop table if exists t1');
  db.execute('create virtual table t1 using fts2(a, b, c)');
  db.execute('insert into t1(a, b, c) values(?, ?, ?)',
             ['one three four', 'one four', 'one four two']);

  // Test fts2o-1.1
  handleResult(db.execute('select rowid, snippet(t1) from t1 where c match ?',
                          ['four']),
               function(rs) {
    assert(rs.isValidRow(), 'column c snippet returned no results');
    assertEqual('one <b>four</b> two', rs.field(1),
                'column c snippet returned wrong results');
    rs.next();
    assert(!rs.isValidRow(), 'column c match returned too many results');
  });

  // Test fts2o-1.2
  handleResult(db.execute('select rowid, snippet(t1) from t1 where b match ?',
                          ['four']),
               function(rs) {
    assert(rs.isValidRow(), 'column b snippet returned no results');
    assertEqual('one <b>four</b>', rs.field(1),
                'column b snippet returned wrong results');
    rs.next();
    assert(!rs.isValidRow(), 'column b match returned too many results');
  });

  // Test fts2o-1.3
  handleResult(db.execute('select rowid, snippet(t1) from t1 where a match ?',
                          ['four']),
               function(rs) {
    assert(rs.isValidRow(), 'column a snippet returned no results');
    assertEqual('one three <b>four</b>', rs.field(1),
                'column a snippet returned wrong results');
    rs.next();
    assert(!rs.isValidRow(), 'column a match returned too many results');
  });
}

function testFulltextIndexingSchemaChange() {
  db.execute('drop table if exists t1');
  db.execute('drop table if exists t3');
  db.execute('create virtual table t1 using fts2(a, b, c)');
  db.execute('insert into t1(a, b, c) values(?, ?, ?)',
             ['one three four', 'one four', 'one two']);

  // Test fts2o-3.1
  handleResult(db.execute('select a, b, c from t1 where c match ?',
                          ['two']),
               function(rs) {
    assert(rs.isValidRow(), 'Unexpected empty result set');
    assertEqual('one three four', rs.field(0), 'Unexpected column a');
    assertEqual('one four', rs.field(1), 'Unexpected column b');
    assertEqual('one two', rs.field(2), 'Unexpected column c');
    rs.next();
    assert(!rs.isValidRow(), 'Unexpected second result');
  });

  // This causes an SQLITE_SCHEMA w/in fts2.
  db.execute('create table t3(a, c, b)');

  // Test fts2o-3.2
  handleResult(db.execute('select a, b, c from t1 where c match ?',
                          ['two']),
               function(rs) {
    assert(rs.isValidRow(), 'Unexpected empty result set');
    assertEqual('one three four', rs.field(0), 'Unexpected column a');
    assertEqual('one four', rs.field(1), 'Unexpected column b');
    assertEqual('one two', rs.field(2), 'Unexpected column c');
    rs.next();
    assert(!rs.isValidRow(), 'Unexpected second result');
  });
}

// From http://code.google.com/p/gears/issues/detail?id=530
// "Deleting an inserted fts table row inside transaction after
//  creating new fts table causes db exception".
function testFulltextInsertCreateTableDelete() {
  db.execute("drop table if exists t1");
  db.execute("create virtual table t1 using fts2(n)");
  db.execute("insert into t1 (n,rowid) values (?,?)", ['G',1]);

  db.execute("drop table if exists t2");
  db.execute("create virtual table t2 using fts2(n)");
  db.execute("drop table t2");

  // transaction causes the bug to surface
  db.execute("begin");
  db.execute("delete from t1 where rowid = ?",[1]);
  db.execute("commit"); // fails here.
}
