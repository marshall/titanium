import os
import shutil
import stat
import zipfile
import time
import subprocess

# Windows imports.
if os.name == 'nt':
  import win32api
  import win32con
  import win32file
  import wmi

# Workaround permission, stat.I_WRITE not allowing delete on some systems
FULL_PERMISSION = int('777', 8)

class BaseInstaller:
  """ Handle extension installation and browser profile adjustments. """

  GUID = '{000a9d1c-beef-4f90-9363-039d445309b8}'
  BUILDS = 'output/installers'

  def _prepareProfiles(self):
    """ Unzip profiles. """
    profile_dir = 'profiles'
    profiles = os.listdir(profile_dir)
    for profile in profiles:
      profile_path = os.path.join(profile_dir, profile)
      target_path = profile[:profile.find('.')]
      if os.path.exists(target_path):
        os.chmod(target_path, FULL_PERMISSION)
        shutil.rmtree(target_path, onerror=self._handleRmError)
      profile_zip = open(profile_path, 'rb')
      self._unzip(profile_zip, target_path)
      profile_zip.close()

  def _installExtension(self, build):
    """ Locate the desired ff profile, overwrite it, and unzip build to it. 
    
    Args:
      build: local filename for the build
    """
    if os.path.exists('xpi'):
      shutil.rmtree('xpi', onerror=self._handleRmError)
    xpi = open(build, 'rb')
    self._unzip(xpi, 'xpi')
    xpi.close()
    self._copyProfileAndInstall('xpi')

  def _copyProfileAndInstall(self, extension):
    """ Profile and extension placement for mac/linux.
    
    Args:
      extension: path to folder containing extension to install
    """
    profile_folder = self._findProfileFolderIn(self.ffprofile_path)

    if profile_folder:
      print 'copying over profile...'
    else:
      print 'failed to find profile folder'
      return

    shutil.rmtree(profile_folder, onerror=self._handleRmError)
    self._copyAndChmod(self.ffprofile, profile_folder)
    ext = os.path.join(profile_folder, 'extensions')
    if not os.path.exists(ext):
      os.mkdir(ext)
    gears = os.path.join(ext, BaseInstaller.GUID)
    self._copyAndChmod(extension, gears)

  def _findProfileFolderIn(self, path):
    """ Find and return the right profile folder in directory at path.
    
    Args:
      path: path of the directory to search in
    
    Returns:
      String name of the folder for the desired profile, or False
    """
    dir = os.listdir(path)
    for folder in dir:
      if folder.find('.' + self.profile) > 0:
        profile_folder = os.path.join(path, folder)
        return profile_folder
    return False

  def _findBuildPath(self, type, directory):
    """ Find the path to the build of the given type.
  
    Args:
      type: os string eg 'win32' 'osx' or 'linux'
    
    Returns:
      String path to correct build for the type, else throw
    """
    dir_list = os.listdir(directory)
    for item in dir_list:
      if item.find(type) > -1:
        build = os.path.join(directory, item)
        build = build.replace('/', os.sep).replace('\\', os.sep)
        if os.path.isfile(build):
          return build
    raise "Can't locate build of type '%s' in '%s'" % (type, directory)

  def _saveInstalledBuild(self, build_path):
    """ Moves given build to the "current build" location. """
    if not os.path.exists(self.current_build):
      os.mkdir(self.current_build)
    shutil.copy(build_path, self.current_build)

  def _copyProfile(self, src, dst_folder, profile_name):
    """ Copy profile to correct location. 

    Args:
      src: Location of profile to copy
      dst_folder: Path of folder to copy into
      profile_name: String name of the destination profile
    """
    dst_path = os.path.join(dst_folder, profile_name)
    if not os.path.exists(dst_folder):
      os.makedirs(dst_folder)
    if os.path.exists(dst_path):
      os.chmod(dst_path, FULL_PERMISSION)
      shutil.rmtree(dst_path, onerror=self._handleRmError)
    self._copyAndChmod(src, dst_path)

  def _copyAndChmod(self, src, targ):
    shutil.copytree(src, targ)
    self._chmod(targ, FULL_PERMISSION)

  def _chmod(self, target, permission):
    """ Recursively set permissions to target and children. 

    We must set permissions recursively after unzipping and copying
    folders so that the contents will be executable.  This is necessary
    because by default zip will not mark any files as executable and this
    prevents us from launching executable files embedded in Gears.

    Args:
      target: Target file path
      permission: Int representing the new file permissions.
    """
    os.chmod(target, permission)
    if os.path.isdir(target):
      for file in os.listdir(target):
        new_target = os.path.join(target, file)
        self._chmod(new_target, permission)

  def _handleRmError(self, func, path, exc_info):
    """ Handle errors removing files with long names on nt systems.

    Args:
      func: function call that caused exception
      path: path to function call argument
      exc_info: string info about the exception
    """
    # On nt, try using win32file to delete if os.remove fails
    if os.name == 'nt':
      # DeleteFileW can only operate on an absolute path
      if not os.path.isabs(path):
        path = os.path.join(os.getcwd(), path)
      unicode_path = '\\\\?\\' + path
      # Throws an exception on error
      win32file.DeleteFileW(unicode_path)
    else:
      raise StandardError(exc_info)

  def _unzip(self, file, target):
    """ Unzip file to target dir.

    Args: 
      file: file pointer to archive
      target: path for folder to unzip to

    Returns:
      True if successful, else False
    """
    if not target:
      print 'invalid path'
      return False

    os.makedirs(target)
    try:
      zf = zipfile.ZipFile(file)
    except zipfile.BadZipfile:
      print 'invalid zip archive'
      return False

    archive = zf.namelist()
    for thing in archive:
      fullpath = thing.split('/')
      path = target
      for p in fullpath[:-1]:
        try:
          os.makedirs(os.path.join(path, p))
        except OSError:
          pass
        path = os.path.join(path, p)
      if thing[-1:] != '/':
        bytes = zf.read(thing)
        nf = open(os.path.join(target, thing), 'wb')
        nf.write(bytes)
        nf.close()
    return True


class BaseWin32Installer(BaseInstaller):
  """ Installer for Win32 machines, extends Installer. """

  def __init__(self):
    self._prepareProfiles()

  def install(self):
    """ Do installation.  """
    # First, uninstall current installed build, if any exists
    self._uninstallCurrentBuild()

    # Now install new build
    build_path = self._buildPath(BaseInstaller.BUILDS)
    print 'Installing build %s' % build_path
    c = ['msiexec', '/passive', '/i', build_path]
    p = subprocess.Popen(c)
    p.wait()
    google_path = os.path.join(self.appdata_path, 'Google')
    self._copyProfile(self.ieprofile, google_path,
                      'Appcelerator Titanium for Internet Explorer')

    # Save new build as current installed build
    self._saveInstalledBuild(build_path)

  def _uninstallCurrentBuild(self):
    """ If a known current build exists, uninstall it. """
    if os.path.exists(self.current_build):
      # Supress exceptions if uninstall fails or no build present.
      try:
        build_path = self._buildPath(self.current_build)
        print 'Uninstalling build %s' % build_path
        c = ['msiexec', '/passive', '/uninstall', build_path]
        p = subprocess.Popen(c)
        p.wait()
        os.remove(build_path)
      except:
        print 'Uninstall failed or no installer found.'
  

class WinXpInstaller(BaseWin32Installer):
  """ Installer for WinXP, extends Win32Installer. """

  def __init__(self):
    BaseWin32Installer.__init__(self)
    home = os.getenv('USERPROFILE')
    self.current_build = os.path.join(home, 'current_gears_build')
    self.appdata_path = os.path.join(home, 'Local Settings\\Application Data')
    self.ieprofile = 'permissions'

  def _buildPath(self, directory):
    return self._findBuildPath('ffie', directory)


class WinVistaInstaller(BaseWin32Installer):
  """ Installer for Vista, extends Win32Installer. """

  def __init__(self):
    BaseWin32Installer.__init__(self)
    home = os.getenv('USERPROFILE')
    self.current_build = os.path.join(home, 'current_gears_build')
    self.appdata_path = os.path.join(home, 'AppData\\LocalLow')
    self.ieprofile = 'permissions'

  def _buildPath(self, directory):
    return self._findBuildPath('ffie', directory)


class ChromeWin32Installer(BaseWin32Installer):
  """ Installer class for Win32 Google Chrome. """

  CHROME_BIN_PATH = r'..\..\..\third_party\chrome\bin\chrome-win32.zip'
  CHROME_PROFILE_PATH = r'Google\Chrome\User Data\Default\Plugin Data'
  CHROME_PATH = r'Google\Chrome'

  def __init__(self):
    self._prepareProfiles()
    self.profile = 'permissions'
    home = os.getenv('USERPROFILE')
    self.current_build = os.path.join(home, 'current_gears_build')
    appdata_xp = os.path.join(home, 'Local Settings\\Application Data')
    appdata_vista = os.path.join(home, 'AppData\\Local')
    if os.path.exists(appdata_vista):
      self.permissions_path = os.path.join(appdata_vista,
          ChromeWin32Installer.CHROME_PROFILE_PATH)
      self.chrome_path = os.path.join(appdata_vista,
          ChromeWin32Installer.CHROME_PATH)
    elif os.path.exists(appdata_xp):
      self.permissions_path = os.path.join(appdata_xp,
          ChromeWin32Installer.CHROME_PROFILE_PATH)
      self.chrome_path = os.path.join(appdata_xp,
          ChromeWin32Installer.CHROME_PATH)

  def install(self):
    """ Set up Google Chrome, run Gears installer, and set profile data. """

    # First, uninstall current installed Gears build, if any exists.
    self._uninstallCurrentBuild()

    if os.path.exists(ChromeWin32Installer.CHROME_BIN_PATH):
      print 'Unpack and replace Chrome.'
      if os.path.exists(self.chrome_path):
        os.chmod(self.chrome_path, FULL_PERMISSION)
        shutil.rmtree(self.chrome_path, onerror=self._handleRmError)
      chrome_zip = open(ChromeWin32Installer.CHROME_BIN_PATH, 'rb')
      self._unzip(chrome_zip, self.chrome_path)
      chrome_zip.close()

      # Rename unzipped folder to Application to make consistent
      # with normal installation.
      chrome_win32 = os.path.join(self.chrome_path, 'chrome-win32')
      application = os.path.join(self.chrome_path, 'Application')
      os.rename(chrome_win32, application)

    else:
      # TODO(ace): Add a mechanism to abort on install fail
      print 'Chrome installer not found.'

    build_path = self._buildPath(BaseInstaller.BUILDS)
    print 'Installing Chrome-Gears build %s' % build_path
    c = ['msiexec', '/passive', '/i', build_path]
    p = subprocess.Popen(c)
    p.wait()

    print 'Copying Gears permissions.'
    self._copyProfile(self.profile, self.permissions_path, 'Appcelerator Titanium')

    # Save new build as current installed build
    self._saveInstalledBuild(build_path)

  def _buildPath(self, directory):
    return self._findBuildPath('chrome', directory)


class WinCeInstaller(BaseInstaller):
  """ Installer for WinCE, extends Installer. """

  def __init__(self, host):
    self.ieprofile = 'permissions'
    self.host = host
  
  def type(self):
    return 'WinCeInstaller' 
  
  def install(self):
    """ Do installation. """
    print 'Installing and copying permissions.'
    self.__installCab()
    self.__copyPermissions()

  def _buildPath(self, directory):
    return self._findBuildPath('cab', directory)

  def __installCab(self):
    """ Copy installer to device and install.  """
    build_path = self._buildPath(BaseInstaller.BUILDS)

    # Requires cecopy.exe in path.
    copy_cmd = ['cecopy.exe', build_path, 'dev:\\windows\\gears.cab']
    p = subprocess.Popen(copy_cmd)
    p.wait()

    # Requires rapistart.exe in path.  Option /noui for silent install.
    install_cmd = ['rapistart.exe', '\\windows\\wceload.exe', '/noui', 
                   '\\windows\\gears.cab']

    # TODO(ace): Find a more robust solution than waiting a set timeout
    # for installation to finish.
    gears_installer_timeout = 45 #seconds
    subprocess.Popen(install_cmd)
    time.sleep(gears_installer_timeout)

    # Requires pkill.exe in path.
    kill_cmd = ['pkill.exe', 'iexplore.exe']
    p = subprocess.Popen(kill_cmd)
    p.wait()

  def __copyPermissions(self):
    """ Modify permissions file to include host address and copy to device. """
    perm_path = os.path.join(self.ieprofile, 'permissions.db')
    perm_path.replace('/', '\\')

    # Requires sqlite3.exe in path to modify permissions db file.
    modify_cmd = ['sqlite3', perm_path]
    p = subprocess.Popen(modify_cmd, stdin=subprocess.PIPE)

    # Sql commands to add gears permissions to the address of the host server.
    sql_cmd1 = 'INSERT INTO "Access" VALUES(\'http://%s:8001\',1);' % self.host
    sql_cmd2 = ('INSERT INTO "LocationAccess" '
                'VALUES(\'http://%s:8001\',1);' % self.host)
    p.stdin.write(sql_cmd1)
    p.stdin.write(sql_cmd2)
    p.stdin.close()
    p.wait()

    # Requires cecopy.exe in path to copy permissions to device.
    copy_cmd = ['cecopy.exe', self.ieprofile, 
                'dev:\\application data\\google\\'
                'Appcelerator Titanium for Internet Explorer']
    p = subprocess.Popen(copy_cmd)
    p.wait()


class BaseFirefoxMacInstaller(BaseInstaller):
  """ Abstract base class for Mac firefox installers. """

  def __init__(self, profile_name, firefox_bin, profile_loc):
    self.profile = profile_name
    self._prepareProfiles()
    home = os.getenv('HOME')
    ffprofile = 'Library/Application Support/Firefox/Profiles'
    ffcache = 'Library/Caches/Firefox/Profiles'
    self.current_build = os.path.join(home, 'current_gears_build')
    self.ffprofile_path = os.path.join(home, ffprofile)
    self.ffcache_path = os.path.join(home, ffcache)
    self.firefox_bin = firefox_bin
    self.ffprofile = profile_loc
    self.profile_arg = '-CreateProfile %s' % self.profile

  def _buildPath(self, directory):
    return self._findBuildPath('xpi', directory)

  def install(self):
    """ Do installation. """
    print 'Creating test profile and inserting extension'
    build_path = self._buildPath(BaseInstaller.BUILDS)
    os.system('%s %s' % (self.firefox_bin, self.profile_arg))
    self._installExtension(build_path)
    self._copyProfileCacheMac()
    self._saveInstalledBuild(build_path)

  def _copyProfileCacheMac(self):
    """ Copy cache portion of profile on mac. """
    profile_folder = self._findProfileFolderIn(self.ffcache_path)

    if profile_folder:
      print 'copying profile cache...'
    else:
      print 'failed to find profile folder'
      return

    # Empty cache and replace only with gears folder
    gears_folder = os.path.join(profile_folder, 'Appcelerator Titanium for Firefox')
    ffprofile_cache = 'ff2profile-mac/Appcelerator Titanium for Firefox'
    shutil.rmtree(profile_folder, onerror=self._handleRmError)
    os.mkdir(profile_folder)
    self._copyAndChmod(ffprofile_cache, gears_folder)


class Firefox2MacInstaller(BaseFirefoxMacInstaller):
  """ Firefox 2 installer for Mac OS X. """

  FIREFOX_PATH = '/Applications/Firefox.app/Contents/MacOS/firefox-bin'

  def __init__(self, profile_name):
    firefox_bin = Firefox2MacInstaller.FIREFOX_PATH
    BaseFirefoxMacInstaller.__init__(self, profile_name, 
                                     firefox_bin, 'ff2profile-mac')


class Firefox3MacInstaller(BaseFirefoxMacInstaller):
  """ Firefox 3 installer for Mac OS X. """

  FIREFOX_PATH = '/Applications/Firefox3.app/Contents/MacOS/firefox-bin'

  def __init__(self, profile_name):
    firefox_bin = Firefox3MacInstaller.FIREFOX_PATH
    BaseFirefoxMacInstaller.__init__(self, profile_name, 
                                     firefox_bin, 'ff3profile-mac')


class SafariMacInstaller(BaseInstaller):
  """ Safari installer for Mac OS X. """

  def __init__(self, build_type):
    self._prepareProfiles()
    self.build_type = build_type
    home = os.getenv('HOME')
    self.current_build = os.path.join(home, 'current_gears_build')
    self.profile_path = os.path.join(home, 'Library/Application Support/Google')
    self.profile_name = 'Appcelerator Titanium for Safari'
    self.src_profile = 'permissions'

  def _buildPath(self, directory):
    return self._findBuildPath('dmg', directory)

  def _GetRootPassword(self):
    """ Read root password from file ~/.password. """
    home = os.getenv('HOME')
    password_path = os.path.join(home, '.password')
    if os.path.exists(password_path):
      f = open(password_path, 'r')
      pw = f.read()
      f.close()
      return pw
    else:
      print ('Could not find ~/.password file.  This file '
             'must exist and contain the root password.')
      return ''

  def _RunAsRoot(self, root_cmd):
    import pexpect
    print 'Running "%s"' % root_cmd
    p = pexpect.spawn('sudo %s' % root_cmd)
    try:
      p.expect('Password', timeout=10)
      p.sendline(self._GetRootPassword())
    except pexpect.EOF:
      print 'Was not prompted for root password'
    except pexpect.TIMEOUT:
      print 'Was not prompted for password within timeout'

  def install(self):
    """ Copy extension and profile for Safari. """
    print 'Running Safari uninstall script'
    clean_command = os.path.abspath('../../tools/osx/clean_gears.sh')
    self._RunAsRoot(clean_command)

    print 'Running Safari install script'
    build_path = self._buildPath(BaseInstaller.BUILDS)
    installer = os.path.abspath('../../bin-%s/installers/Safari/gears.pkg' 
                                % self.build_type)
    install_command = ('/usr/sbin/installer -pkg %s -target /' % installer)
    self._RunAsRoot(install_command)    
    self._copyProfile(self.src_profile, self.profile_path, self.profile_name)
    self._saveInstalledBuild(build_path)


class BaseFirefoxLinuxInstaller(BaseInstaller):
  """ Abstract base class for Linux firefox installers. """

  def __init__(self, profile_name):
    self.profile = profile_name
    self._prepareProfiles()
    home = os.getenv('HOME')
    self.current_build = os.path.join(home, 'current_gears_build')
    self.ffprofile_path = os.path.join(home, '.mozilla/firefox')
    self.firefox_bin = 'firefox'
    self.profile_arg = '-CreateProfile %s' % self.profile

  def _buildPath(self, directory):
    return self._findBuildPath('xpi', directory)

  def install(self):
    """ Do installation. """
    print 'Creating test profile and inserting extension'
    build_path = self._buildPath(BaseInstaller.BUILDS)
    os.system('%s %s' % (self.firefox_bin, self.profile_arg))
    self._installExtension(build_path)
    self._saveInstalledBuild(build_path)


class Firefox2LinuxInstaller(BaseFirefoxLinuxInstaller):
  """ Firefox 2 installer for linux. """

  def __init__(self, profile_name):
    self.ffprofile = 'ff2profile-linux'
    BaseFirefoxLinuxInstaller.__init__(self, profile_name)


class Firefox3LinuxInstaller(BaseFirefoxLinuxInstaller):
  """ Firefox 3 installer for linux. """

  def __init__(self, profile_name):
    self.ffprofile = 'ff3profile-linux'
    BaseFirefoxLinuxInstaller.__init__(self, profile_name)
