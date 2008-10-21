import unittest
from runner import FirefoxWin32Launcher

class FirefoxWin32LauncherTest(unittest.TestCase):
    
    def test_launch(self):
        launcher = FirefoxWin32Launcher('ffprofile-win')
        url = "http://foobar.com"
        launcher.launch(url)
        launcher.kill()
