testSuite("Gears binding tests", "dummy.html", {

	run: function ()
	{
		test("top level Gears modules", function() {
			assert(ti != null);
			assert(ti.Desktop != null);
			assert(ti.Database != null);
			assert(ti.Filesystem != null);
		});

		test("Desktop API", function() {
			assert(ti.Desktop.openFiles != null);
			assert(ti.Desktop.createShortcut != null);
		});

		test("Database API", function() {
			var db = new ti.Database();
			assert(db.open != null);
			assert(db.execute != null);
			assert(db.close != null);

			db.open('apivalidator-test');
			db.execute('create table if not exists apivalidator ' +
				'(testfield text)');

			db.execute('insert into apivalidator values (?)',['testrow']);
			var rs = db.execute('select * from apivalidator');

			assert(rs != null);
			assert(rs.isValidRow != null);
			assert(rs.field != null);
			assert(rs.close != null);
	
			assert(rs.isValidRow());
			assert(rs.fieldCount() == 1);
			assert(rs.field(0) == 'testrow');
			rs.close();
		});
	}
}); 
