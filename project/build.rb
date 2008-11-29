#
# This file is part of Appcelerator.
#
# Copyright 2008 Appcelerator, Inc.
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#

desc 'build titanium project files'
task :project do 
  build_dir = "#{File.dirname(__FILE__)}/pieces" 
  build_config = get_config(:titanium, :project)
  zipfile = build_config[:output_filename]
  
  FileUtils.mkdir_p STAGE_DIR unless File.exists? STAGE_DIR
  FileUtils.rm_rf zipfile

  # these are the directories to be included in the zip
  dirs = %w(public config)
  
  Zip::ZipFile.open(zipfile, Zip::ZipFile::CREATE) do |zipfile|
    dirs.each do |dir|
      dofiles(File.join(build_dir,dir)) do |f|
        filename = File.basename(f)
        next if File.basename(filename[0,1]) == '.'
        zipfile.add("#{dir}/#{filename}",File.join(build_dir,dir,filename))
      end
    end
    zipfile.add('installer.rb',File.join(build_dir,'installer.rb'));
    zipfile.get_output_stream('build.yml') do |f|
      config = Marshal::load(Marshal::dump(build_config))
      config.delete :output_filename
      f.puts config.to_yaml
    end
  end
end
