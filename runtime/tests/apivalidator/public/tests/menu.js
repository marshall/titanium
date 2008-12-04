testSuite("ti.Menu API tests", "dummy.html", {
	run: function () {
		test("top level API", function() {
			assert(ti != null);
			assert(ti.Menu != null);
			assert(ti.Menu.createSystemMenu != null);
			assert(ti.Menu.createUserMenu != null);
		});
	}
});

