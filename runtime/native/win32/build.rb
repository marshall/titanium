#
# Copyright 2008 Appcelerator, Inc.
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

require 'fileutils'

desc 'build titanium win32 runtime'
task :all =>[:check_chromedir,:check_devenv,:build]

$devenv_path = nil

task :check_chromedir do
	if not ENV.has_key?("CHROMEDIR")
		STDERR.puts "Error: The environment variable CHROMEDIR needs to point to the location of the chromium \"src\" directory, but it wasn't defined.\n"
		exit -1		
	end
end

task :check_devenv do
	
	ENV["PATH"].split(File::PATH_SEPARATOR).each do |path|
		if is_cygwin?
			path = `cygpath -d "#{path}"`
		end

		if File.exists?(File.join(path, 'devenv.com'))
			$devenv_path = File.join(path, 'devenv.com')
		end
	end

	if $devenv_path.nil?
		$devenv_path = 'C:\Program Files\Microsoft Visual Studio 8\Common7\IDE\devenv.com'

		if not File.exists?($devenv_path)
			STDERR.puts "Error: Could not find devenv.com on the PATH. Please make sure you add your installation of VC++ 2005 to the PATH, including the subdirectory Common7/IDE"
			exit -1
		end
	end
end

task :build do
	FileUtils.cd 'src' do
		puts "Build is running, you can monitor the output by executing tail -f build.log"	
		command = '"' + $devenv_path + '" titanium.sln /build Release /log build.log'

		if is_cygwin?
			command = 'cmd /C ' + command
		end

		system command	
	end
end
