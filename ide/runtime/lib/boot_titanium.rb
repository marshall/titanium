#
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

require 'yaml'

def die(msg=nil, exit_value=1)
  $titanium.die(msg)
  exit exit_value
end

def confirm(q,canforce=true,die_if_fails=true,default='y')
    return true if OPTIONS[:force]
		answer = $titanium.confirm(q,canforce)
		answer = default if not answer or answer == ''
    OPTIONS[:force]=true if canforce and answer == 'Always'
		if answer != 'Yes' or answer != 'Always'
      die('Aborted by User') if die_if_fails
      return false
    end
    true
end

def confirm_yes(q)
  confirm(q,true,false,'y')
end

# overwrite puts and print
def puts(message)
  $titanium.info(message)
end
def print(message)
  $titanium.info(message)
end

class TitaniumProgressBar
  @bar_id = nil
  
  def initialize(id, max)
    @bar_id = id
    $titanium.openProgress(@bar_id, max)
  end
  
  def setMessage(msg)
    $titanium.setProgressMessage(@bar_id, msg)
  end
  
  def increment(amount)
    $titanium.incrementProgress(@bar_id, amount)
  end
  
  def close()
    $titanium.closeProgress(@bar_id)
  end
end

#
# set up temp directory and delete it on exit
#
require 'tmpdir'
APP_TEMP_DIR=FileUtils.mkdir_p(File.join(Dir::tmpdir, "appcelerator.#{rand(10000)}.#{$$}"))

APP_TEMP_FILES = Array.new

def recursive_deltree(dir)
  APP_TEMP_FILES.each do |file|
    file.close rescue nil
  end
  Dir["#{dir}/**/**"].each do |f|
    FileUtils.rm_rf f
  end
  FileUtils.rm_rf dir
  FileUtils.rmdir dir rescue nil
end

at_exit { recursive_deltree APP_TEMP_DIR unless OPTIONS[:debug] }

if OPTIONS[:version] and ACTION
  if not OPTIONS[:version] =~ /[0-9]+\.[0-9]+(\.[0-9]+)?/
    die "Invalid version format. Must be in the format: X.X.X such as 2.0.1"
  end
end

module Appcelerator
  class Boot

    def Boot.boot

      config = Appcelerator::Installer.load_config
      
      username=nil
      password=nil
      save=false
      
      if not OPTIONS[:server]
        OPTIONS[:server] = config[:server] || ENV['UPDATESITE'] || ET_PHONE_HOME
      end
      
      return nil if OPTIONS[:no_remote]
      
      if config[:username] and config[:password]

        # nothing required
        
      elsif ACTION != 'login'  

        # first time user - we need to either allow them to signup or allow them to login
        
    		Installer.login(username,password)
        save = true
      end
      
      # save the config
      Installer.save_config(username,password) if save
      
    end
    
  end
end

