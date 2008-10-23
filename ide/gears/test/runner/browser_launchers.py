import os
import subprocess
import signal
import time

# Windows imports.
if os.name == 'nt':
  import win32api
  import win32con
  import wmi

class BaseBrowserLauncher:
  """Handle launching and killing of a specific test browser."""
  # Time given to allow firefox to update test profile and close.

  FIREFOX_QUITTING_SLEEP_TIME = 15 #seconds

  def launch(self, url):
    """ Launch browser to the given url and track the returned process.

    Args:
      url: Url to launch
    """
    command = self.browser_command
    command.append(url)
    print 'launching: %s' % str(command)
    self.process = subprocess.Popen(command)


class BaseWin32Launcher(BaseBrowserLauncher):
  """Abstract base class for Win32 launchers."""

  def launch(self, url):
    """ First perform some cleanup, then launch browser. """
    self._DestroyOldSlaveProcesses()
    BaseBrowserLauncher.launch(self, url)

  def _killInstancesByName(self, process_name):
    """Kill all instances of given process name.

    Args:
      process_name: String name of process to kill.
    """
    process_list = wmi.WMI().Win32_Process(Name=process_name)
    for process in process_list:
      pid = process.ProcessID
      print 'killing process: %d' % pid
      handle = win32api.OpenProcess(win32con.PROCESS_TERMINATE, 0, pid)
      win32api.TerminateProcess(handle, 0)
      win32api.CloseHandle(handle)
    # Don't return until processes are gone
    while len(wmi.WMI().Win32_Process(Name=process_name)) > 0:
      time.sleep(1)


  def _DestroyOldSlaveProcesses(self):
    """Check for and kill any existing gears slave processes.

    Gears internal tests create some slave processes while running.
    Here we check to see if any did not shut down properly and
    destroy any that remain.
    """
    process_list = wmi.WMI().Win32_Process(Name='rundll32.exe')
    for process in process_list:
      pid = process.ProcessID
      print 'RUNDLL32 PROCESS** %s' % process.CommandLine
      if process.CommandLine.rfind('gears.dll') > 0:
        print 'Killing deadlocked slave process: %d' % pid
        handle = win32api.OpenProcess(win32con.PROCESS_TERMINATE, 0, pid)
        win32api.TerminateProcess(handle, 0)
        win32api.CloseHandle(handle)    


class BaseFirefoxWin32Launcher(BaseWin32Launcher):
  """Abstract base class for win32 firefox launchers."""

  def __init__(self, profile, firefox_path, automated=True):
    self.killAllInstances()
    program_files = os.getenv('PROGRAMFILES')
    self.browser_command = [os.path.join(program_files, firefox_path)]
    if automated:
      self.browser_command.extend(['-profile', profile])

  def killAllInstances(self):
    self._killInstancesByName('firefox.exe')
    self._killInstancesByName('crashreporter.exe')


class Firefox2Win32Launcher(BaseFirefoxWin32Launcher):
  """Launcher for ff2 in Windows."""

  FIREFOX_PATH = 'Mozilla Firefox\\firefox.exe'

  def __init__(self, profile, automated=True):
    BaseFirefoxWin32Launcher.__init__(self, profile, self.FIREFOX_PATH,
                                      automated)

  def type(self):
    return 'Firefox2Win32'


class Firefox3Win32Launcher(BaseFirefoxWin32Launcher):
  """Launcher for ff3 on Windows."""

  FIREFOX_PATH = 'Mozilla Firefox 3\\firefox.exe'

  def __init__(self, profile, automated=True):
    BaseFirefoxWin32Launcher.__init__(self, profile, self.FIREFOX_PATH,
                                      automated)

  def type(self):
    return 'Firefox3Win32'


class IExploreWin32Launcher(BaseWin32Launcher):
  """Launcher for iexplorer browser on Win32 platforms."""

  def __init__(self, automated=True):
    self.killAllInstances()
    program_files = os.getenv('PROGRAMFILES')
    ie_path = os.path.join(program_files, 'internet explorer\\iexplore.exe')
    self.browser_command = [ie_path]

  def killAllInstances(self):
    """Kill all instances of iexplore.exe

    Must kill ie by name and not pid for ie7 compatibility.
    """
    self._killInstancesByName('iexplore.exe')
    # Also kill ie's crash handler process
    self._killInstancesByName('iedw.exe')

  def type(self):
    return 'IExploreWin32'


class ChromeWin32Launcher(BaseWin32Launcher):
  """Launcher class for win32 Google Chrome."""

  CHROME_PATH = r'Google\Chrome\Application\chrome.exe'

  def __init__(self):
    self.killAllInstances()
    home = os.getenv('USERPROFILE')
    appdata_xp = os.path.join(home, 'Local Settings\\Application Data')
    appdata_vista = os.path.join(home, 'AppData\\Local')
    if os.path.exists(appdata_vista):
      self.browser_command = [os.path.join(appdata_vista,
          ChromeWin32Launcher.CHROME_PATH)]
    elif os.path.exists(appdata_xp):
      self.browser_command = [os.path.join(appdata_xp,
          ChromeWin32Launcher.CHROME_PATH)]

  def killAllInstances(self):
    self._killInstancesByName('chrome.exe')

  def type(self):
    return 'ChromeWin32'


class IExploreWinCeLauncher(BaseBrowserLauncher):
  """Launcher for pocket ie on Windows Mobile."""

  def __init__(self, automated=True):
    self.killAllInstances()

  def type(self):
    return 'IExploreWinCE'

  def launch(self, url):
    """Do launch."""
    # Requires rapistart.exe in path.
    launch_cmd = ['rapistart.exe', '\windows\iexplore.exe', url]
    subprocess.Popen(launch_cmd)

  def killAllInstances(self):
    """ Kill browser. """
    # Requires pkill.exe in path.
    print 'Killing pocket ie on device'
    kill_cmd = ['pkill.exe', 'iexplore.exe']
    subprocess.Popen(kill_cmd)


class BasePosixLauncher(BaseBrowserLauncher):
  """Abstract base class for posix platform launchers."""

  def _killInstancesByName(self, process_name):
    """Kill all instances of process process_name.

    Args:
      process_name: string name of process to kill
    """
    print 'Attempting to kill all processes named %s' % process_name
    kill_cmd = ['killall', '-9', process_name]
    p = subprocess.Popen(kill_cmd)
    p.wait()


class SafariMacLauncher(BasePosixLauncher):
  """Launcher for Safari on OS X."""

  def __init__(self, automated=True):
    self.killAllInstances()
    self.browser_command = ['open', '-a', 'Safari']

  def killAllInstances(self):
    """Kill all instances of safari.

    Must kill safari by name, as the pid can't be tracked on launch.
    """
    self._killInstancesByName('Safari')

  def type(self):
    return 'SafariMac'


class BaseFirefoxMacLauncher(BasePosixLauncher):
  """Abstract base class for firefox launchers on OSX."""

  def __init__(self, profile, automated, firefox_bin):
    """Set firefox vars."""
    self.killAllInstances()
    self.browser_command = [firefox_bin]
    if automated:
      self.browser_command.extend(['-P', profile])

  def killAllInstances(self):
    """Kill all firefox and ff crash reporter instances."""
    self._killInstancesByName('firefox-bin')
    self._killInstancesByName('crashreporter')


class Firefox2MacLauncher(BaseFirefoxMacLauncher):
  """Firefox 2 implementation for FirefoxMacLauncher."""

  FIREFOX_PATH = '/Applications/Firefox.app/Contents/MacOS/firefox-bin'

  def __init__(self, profile, automated=True):
    firefox_bin = Firefox2MacLauncher.FIREFOX_PATH
    BaseFirefoxMacLauncher.__init__(self, profile, automated, firefox_bin)

  def type(self):
    return 'Firefox2Mac'


class Firefox3MacLauncher(BaseFirefoxMacLauncher):
  """Firefox 3 implementation for FirefoxMacLauncher."""

  FIREFOX_PATH = '/Applications/Firefox3.app/Contents/MacOS/firefox-bin'

  def __init__(self, profile, automated=True):
    firefox_bin = Firefox3MacLauncher.FIREFOX_PATH
    BaseFirefoxMacLauncher.__init__(self, profile, automated, firefox_bin)

  def type(self):
    return 'Firefox3Mac'


class BaseFirefoxLinuxLauncher(BasePosixLauncher):
  """Abstract base class for firefox launchers on linux."""

  def __init__(self, profile, automated=True):
    self.killAllInstances()
    home = os.getenv('HOME')
    if automated:
      self.browser_command.extend(['-P', profile])

  def killAllInstances(self):
    self._killInstancesByName('firefox-bin')


class Firefox2LinuxLauncher(BaseFirefoxLinuxLauncher):
  """Launcher for firefox2 on linux, extends BaseFirefoxLinuxLauncher."""

  def __init__(self, profile, automated=True):
    self.browser_command = ['firefox2']
    BaseFirefoxLinuxLauncher.__init__(self, profile, automated)

  def type(self):
    return 'Firefox2Linux'


class Firefox3LinuxLauncher(BaseFirefoxLinuxLauncher):
  """Launcher for firefox3 on linux, extends BaseFirefoxLinuxLauncher."""

  def __init__(self, profile, automated=True):
    self.browser_command = ['firefox3']
    BaseFirefoxLinuxLauncher.__init__(self, profile, automated)

  def type(self):
    return 'Firefox3Linux'
