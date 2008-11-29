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

desc 'build all titanium runtimes'
task :all => [:osx,:win32,:linux] do
end

def package_pieces(os)
  build_dir = "#{File.dirname(__FILE__)}/pieces/#{os.to_s}" 
  build_config = get_config(:titanium, os)
  zipfile = build_config[:output_filename]
  
  FileUtils.mkdir_p STAGE_DIR unless File.exists? STAGE_DIR
  FileUtils.rm_rf zipfile
  
  Zip::ZipFile.open(zipfile, Zip::ZipFile::CREATE) do |zipfile|
    dofiles(build_dir) do |f|
      filename = File.basename(f)
      next if File.basename(filename[0,1]) == '.'
      zipfile.add("#{filename}",File.join(build_dir,filename))
    end
    zipfile.get_output_stream('build.yml') do |f|
      config = Marshal::load(Marshal::dump(build_config))
      config.delete :output_filename
      f.puts config.to_yaml
    end
  end
end

desc 'build titanium osx runtime'
task :osx do
  package_pieces :osx
end

desc 'build titanium win32 runtime'
task :win32 do
  package_pieces :win32
end

desc 'build titanium linux runtime'
task :linux do
  package_pieces :linux
end

