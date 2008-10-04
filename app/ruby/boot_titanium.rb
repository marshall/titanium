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

titanium = TitaniumBoot.new

def die(msg=nil, exit_value=1)
	titanium.die(msg) unless msg.nil?
  exit exit_value
end

def ask_with_validation(q,msg,regexp,mask=false)
  response = nil
  while true
    response = ask(q,mask)
    break if response =~ regexp
    STDERR.puts msg if response
  end
  response
end

def ask(q,mask=false)
  if OPTIONS[:subprocess]
    STDOUT.puts "__MAGIC__|ask|#{q}|#{mask}|__MAGIC__"
  else
    STDOUT.print "#{q} "
  end
  STDOUT.flush
  mask = mask and STDIN.isatty and not OPTIONS[:subprocess]
  if mask
    system 'stty -echo' rescue nil
  end
  answer = ''
  while true
    ch = STDIN.getc
    break if ch==10
    answer << ch
  end
  if mask
    system 'stty echo' rescue nil
  end
  puts if mask # newline after password
  answer
end

def confirm(q,canforce=true,die_if_fails=true,default='y')
    return true if OPTIONS[:force]
		answer = titanium.confirm(q,canforce)
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

#
# set up temp directory and delete it on exit
#
tmp_dir = ENV['TMPDIR'] || ENV['TEMP'] || ENV['TMP']
if (not(tmp_dir) and File.directory?('/tmp'))
    tmp_dir = '/tmp'
else
    tmp_dir = '.'
end
APP_TEMP_DIR=FileUtils.mkdir_p(File.join(tmp_dir, "appcelerator.#{rand(10000)}.#{$$}"))

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
        
        puts 
        puts '*' * 80
        puts ' ' * 20 + 'Welcome to the Appcelerator RIA Platform'
        puts '*' * 80
        puts
        puts 'Before we can continue, you will need your Appcelerator Developer Network login'
        puts 'credentials.  If you have not yet created a (free) developer account, you can '
        puts 'either visit http://www.appcelerator.org to create one and return.  Or, you can'
        puts 'create an account now.'
        puts 
        puts 
        
        yn = ask 'Do you have an account? (Y)es or (N)o [Y]'

        if ['y','Y',''].index(yn)
    			Installer.login(username,password)
          puts "Welcome back ...."
          puts
                    
        else
          #
          # don't have an account - we need to collect information for signup
          #
          puts 
          puts "OK, let's sign you up to the Appcelerator Developer Network.  We'll need "
          puts "a little bit of information to get you signed up.  Here we go:"
          puts
          username = Installer.prompt_username 'Email'
          Installer.prompt_proxy(true)
          firstname = ask 'Firstname:'
          lastname = ask 'Lastname:'
          password = Installer.prompt_password
          Installer.prompt_password password
          
          puts 
          puts "Signing you up .... one moment please."
          puts
          
          result = Installer.signup(username,firstname,lastname,password)
          
          if not result['success']
            die "Signup failed. #{result['msg']}"
          end
          
          puts "Signup almost complete.  You will now need to check your email address"
          puts "at #{username} for a validation email.  In this email, you will find a "
          puts "four-character verification code.  Please enter that code below or press"
          puts "return if you have already verified your account by clicking on the URL"
          puts "in the email."
          puts
          
          attempt_login = true
          
          while true
            verification = ask 'Verification code:'
            if verification.nil? or verification == ''
              if Installer.network_login(username,password)
                attempt_login = false
                break
              else
                puts "Couldn't not validate your account. Please try again."
              end
            else
              result = Installer.validate_signup(username,password,verification)
              if result and result['success']
                break
              else
                puts "Error validating your verification code. #{result['msg']}"
              end
            end
          end
          
          puts
          puts "Congratulations! You're now signed up and verified."
          puts
          puts '*' * 80
          puts
    			
          Installer.network_login(username,password) if attempt_login
          
        end

        save = true
      end
      
      # save the config
      Installer.save_config(username,password) if save
      
    end
    
  end
end

