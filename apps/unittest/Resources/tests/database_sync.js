
var datadir = Titanium.Filesystem.getApplicationDataDirectory();

//
// delete any existing files we may have created related to DB
// so we can ensure that the code below is running fresh on this app
//
var listing = Titanium.Filesystem.getFile(datadir).getDirectoryListing();
for (var c=0;c<listing.length;c++)
{
	var f = listing[c];
	if (f.isFile() && f.name()!='application.properties')
	{
		Titanium.API.debug("deleting file: "+f);
		f.deleteFile();
	}
	else if (f.isDirectory())
	{
		Titanium.API.debug("deleting directory: "+f);
		f.deleteDirectory(true);
	}
}

Titanium.AppTest.addResult('database_sync.database',typeof(Titanium.Filesystem)=='object');

Titanium.AppTest.addResult('database_sync.database',typeof(Titanium.Database)=='object');
Titanium.AppTest.addResult('database_sync.database.method_open',typeof(Titanium.Database.open)=='function');


var db = null;

try
{
	db = Titanium.Database.open("test_db");
	Titanium.AppTest.addResult('database_sync.open_db',db!=null);
	Titanium.AppTest.addResult('database_sync.database.method_execute',typeof(db.execute)=='function');
	Titanium.AppTest.addResult('database_sync.database.method_close',typeof(db.close)=='function');
	Titanium.AppTest.addResult('database_sync.database.method_remove',typeof(db.remove)=='function');
	Titanium.AppTest.addResult('database_sync.database.property_rowsAffected',typeof(db.rowsAffected)=='number');
}
catch(e)
{
	Titanium.AppTest.addResult('database_sync.open_db',false,"failed with open db exception: "+e);
}

try
{
	var rs = db.execute("CREATE TABLE TEST (name TEXT)");
	Titanium.AppTest.addResult('database_sync.create_table',true);
	Titanium.AppTest.addResult('database_sync.create_table_result_type',typeof(rs)=='object','type was: '+typeof(rs));
	Titanium.AppTest.addResult('database_sync.create_table_result_not_null',rs!=null);
	Titanium.AppTest.addResult('database_sync.create_table_result_not_valid_row',rs.isValidRow()===false);
	Titanium.AppTest.addResult('database_sync.create_table_result_test_fieldcount',rs.fieldCount()===0);
	Titanium.AppTest.addResult('database_sync.create_table_result_test_fieldname',rs.fieldName(0)===null);
	Titanium.AppTest.addResult('database_sync.create_table_result_test_field',rs.field(0)===null);
	rs.close();
}
catch(e)
{
	Titanium.AppTest.addResult('database_sync.create_table',false,"failed with exception: "+e);
}

try
{
	var rs = db.execute("CREATE TABLE IF NOT EXISTS TEST (name TEXT)");
	Titanium.AppTest.addResult('database_sync.create_table_if_exists',typeof(rs)=='object');
	Titanium.AppTest.addResult('database_sync.create_table_if_exists.rowsAffected',db.rowsAffected==0);
	Titanium.AppTest.addResult('database_sync.create_table_if_exists.isValidRow',rs.isValidRow()==false);
	rs.close();
}
catch(e)
{
	Titanium.AppTest.addResult('database_sync.create_table_if_exists',false,"failed with exception: "+e);
}

try
{
	var rs = db.execute("select count(*) from TEST");
	Titanium.AppTest.addResult('database_sync.select_count',typeof(rs)=='object');
	Titanium.AppTest.addResult('database_sync.select_count.next',rs.next()==undefined);
	Titanium.AppTest.addResult('database_sync.select_count.count',rs.field(0)==0);
	rs.close();
}
catch(e)
{
	Titanium.AppTest.addResult('database_sync.select_count',false,"failed with exception: "+e);
}

try
{
	db.execute("insert into TEST values('a')");
	Titanium.AppTest.addResult('database_sync.insert',true);
	var rs = db.execute("select * from TEST");
	Titanium.AppTest.addResult('database_sync.select_star_isvalidrow',rs.isValidRow());
	Titanium.AppTest.addResult('database_sync.select_star_next',rs.next()==undefined);
	Titanium.AppTest.addResult('database_sync.select_star_isvalidrow2',rs.isValidRow()==false);
	Titanium.AppTest.addResult('database_sync.select_star_field',rs.field(0)=='a');
	Titanium.AppTest.addResult('database_sync.select_star_fieldname',rs.fieldName(0)=='name');
	Titanium.AppTest.addResult('database_sync.select_star_fieldByName',rs.fieldByName('name')=='a');
	Titanium.AppTest.addResult('database_sync.select_star_fieldCount',rs.fieldCount()==1);
	rs.close();
}
catch(e)
{
	Titanium.AppTest.addResult('database_sync.insert',false,"failed with exception: "+e);
}

try
{
	db.execute("DROP TABLE TEST");
	Titanium.AppTest.addResult('database_sync.drop_table',true);
}
catch(e)
{
	Titanium.AppTest.addResult('database_sync.drop_table',false,"failed with exception: "+e);
}

try
{
	db.execute("select * from TEST");
	Titanium.AppTest.addResult('database_sync.select_after_drop',false,'returned result but should have thrown exception');
}
catch(e)
{
	Titanium.AppTest.addResult('database_sync.select_after_drop',true,"exception: "+e);
}

try
{
	var rs = db.execute("CREATE TABLE TEST (name TEXT, size INT)");
	Titanium.AppTest.addResult('database_sync.create_table3',true);
	rs.close();
}
catch(e)
{
	Titanium.AppTest.addResult('database_sync.create_table3',false,"failed with exception: "+e);
}

try
{
	db.execute("insert into TEST values('b',123)");
	var rs = db.execute("select name as n, size as s from TEST");
	Titanium.AppTest.addResult('database_sync.select_multiple',true);
	Titanium.AppTest.addResult('database_sync.select_multiple.fieldCount',rs.fieldCount()==2);
	Titanium.AppTest.addResult('database_sync.select_multiple.rowCount',rs.rowCount()==1);
	rs.next();
	Titanium.AppTest.addResult('database_sync.select_multiple.field0',rs.field(0)=='b');
	Titanium.AppTest.addResult('database_sync.select_multiple.field1',rs.field(1)==123);
	Titanium.AppTest.addResult('database_sync.select_multiple.field1_type',typeof(rs.field(1))=='number');
	Titanium.AppTest.addResult('database_sync.select_multiple.fieldbyname',rs.fieldByName('n')=='b');
	rs.close();
	db.execute("insert into TEST values('c',567)");
	rs = db.execute("select count(*), sum(size) from TEST");
	rs.next();
	Titanium.AppTest.addResult('database_sync.select_multiple2.rowCount',rs.rowCount()==1);
	Titanium.AppTest.addResult('database_sync.select_multiple2.field0',rs.field(0)==2);
	Titanium.AppTest.addResult('database_sync.select_multiple2.field1',rs.field(1)==690);
	rs.close();
	rs = db.execute("select * from TEST");
	var count=0;
	while(rs.isValidRow())
	{
		count++;
		rs.next();
	}
	Titanium.AppTest.addResult('database_sync.select_multiple3.count',count==2);
	Titanium.AppTest.addResult('database_sync.select_multiple3.rowsAffected',db.rowsAffected==2);

	rs.close();
}
catch(e)
{
	Titanium.AppTest.addResult('database_sync.select_multiple',false,"failed with exception: "+e);
}


try
{
	db.remove();
	Titanium.AppTest.addResult('database_sync.db_remove',true);
}
catch(e)
{
	Titanium.AppTest.addResult('database_sync.db_remove',false,"failed with exception: "+e);
}


try
{
	// already removed so this should fail silently and just return
	db.close();
	Titanium.AppTest.addResult('database_sync.db_close',true);
}
catch(e)
{
	Titanium.AppTest.addResult('database_sync.db_close',false,"failed with exception: "+e);
}
