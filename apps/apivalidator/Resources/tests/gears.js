testSuite("Gears binding tests", "dummy.html", {

	run: function ()
	{
		test("top level Gears modules", function() {
			var ti = parent.ti;
			assert(ti != null);
			assert(Titanium.Desktop != null);
			assert(Titanium.Database != null);
			assert(Titanium.Filesystem != null);
		});

		test("Desktop API", function() {
			var ti = parent.ti;
			assert(Titanium.Desktop.openFiles != null);
			assert(Titanium.Desktop.createShortcut != null);
		});

		test("Database API", function() {
			var ti = parent.ti;
			var db = new Titanium.Database();
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
