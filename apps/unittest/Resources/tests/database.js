var db = openDatabase('unittest', "1.0");
Titanium.AppTest.addResult('db_open_database',(db!=null));

// load or initialize project table
db.transaction(function(tx) 
{   
	// see if project table exists
   	tx.executeSql("CREATE TABLE TEST (name TEXT)", [], function(tx,result) 
   	{
		Titanium.AppTest.addResult('db_create_table',true);
		tx.executeSql('INSERT INTO TEST (name) VALUES (?)',['foo'],function(tx,result)
		{
			Titanium.AppTest.addResult('db_insert_data',true);
			tx.executeSql('SELECT * FROM TEST',[],function(tx,result)
			{
				Titanium.AppTest.addResult('db_select',(result.rows.length ==1));
				tx.executeSql('DROP TABLE TEST');
				Titanium.AppTest.stop();
			},
			function(tx,error)
			{
				Titanium.AppTest.addResult('db_select',false,error);
				tx.executeSql('DROP TABLE TEST');
				Titanium.AppTest.stop();
			});
		},
		function(tx, error)
		{
			Titanium.AppTest.addResult('db_insert_data',false,error);
			tx.executeSql('DROP TABLE TEST');
			Titanium.AppTest.stop();
		})
   	}, 
	function(tx, error) 
   	{
		Titanium.AppTest.addResult('db_create_table',false,error);
		tx.executeSql('DROP TABLE TEST');
		Titanium.AppTest.stop();
   });
});
