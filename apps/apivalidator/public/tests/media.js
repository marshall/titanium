testSuite("ti.Media API tests", "dummy.html",
{
	run: function ()
	{
		test("top level API", function() {
			assert(ti != null);
			assert(ti.Media != null);
			assert(ti.Media.createSound != null);
			assert(ti.Media.beep != null);
		});

		test("sound object API", function() {
			assert(ti != null);
			assert(ti.Media != null);

			var sound = ti.Media.createSound("app://tests/sound.wav");
			assert(sound != null);
			assert(sound.play != null);
			assert(sound.pause != null);
			assert(sound.resume != null);
			assert(sound.stop != null);
			assert(sound.getLooping != null);
			assert(sound.setLooping != null);
			assert(sound.getVolume != null);
			assert(sound.setVolume != null);
	
			assert(sound.getVolume() == 1.0);
			assert(!sound.getLooping());
			
		});
	}
});
 
