def is_mac?
  RUBY_PLATFORM.downcase.include?("darwin")  
end

def is_windows?
  RUBY_PLATFORM.downcase.include?("mswin")
  
end

def is_linux?
  RUBY_PLATFORM.downcase.include?("linux")
end


module Titanium
  WEBKIT_SHELL_PATH = File.join(SYSTEMDIR, 'titanium', 'webkit_shell', (is_mac? ? "osx" : (is_windows? ? "win32" : "linux")))
  WEBKIT_SHELL_OSX_APP = File.join(WEBKIT_SHELL_PATH, "webkit_shell.app")
  WEBKIT_SHELL = File.join(WEBKIT_SHELL_PATH, (is_mac? ? "webkit_shell.app/Contents/MacOS/webkit_shell" : (is_windows? ? "bin/webkit_shell.exe" : "bin/webkit_shell")))
  
  class Launcher
    def Launcher.launchProject()
      command = WEBKIT_SHELL + ' -file ' + File.join(Dir.pwd, 'public', 'index.html')
      puts "launching #{command}"
      
      system command
    end
  end
end