#
# Copyright 2006-2008 Appcelerator, Inc.
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
#    http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

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