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

require 'open-uri'
require 'digest/md5'
require 'set'

module Appcelerator
  class Installer
    
    @@client = nil
    @@username = nil
    @@password = nil
    @@config = nil
    @@notice = false
    @@loggedin = false
    @@distributions = nil
    @@site_config = nil
    @@site_config_file = nil
    @@installed_this_session = Set.new
    
    def Installer.user_home
      if ENV['HOME']
        ENV['HOME']
      elsif ENV['USERPROFILE']
        ENV['USERPROFILE']
      elsif ENV['HOMEDRIVE'] and ENV['HOMEPATH']
        "#{ENV['HOMEDRIVE']}:#{ENV['HOMEPATH']}"
      else
        begin
          File.expand_path '~'
        rescue
          File.expand_path '/'
        end
      end
    end

    def Installer.get_config
      return @@config if @@config
      load_config
    end

    def Installer.load_config
      # load our config
      @@config_file = File.join(RELEASE_DIR,'login.yml')
      @@config = YAML::load_file(@@config_file) if File.exists?(@@config_file)
      @@config||={}
    end

    def Installer.forget_credentials
      Installer.load_config unless @@config
      @@config.delete :username
      @@config.delete :password
      Installer.save_config
    end

    def Installer.save_config(username=nil,password=nil)
      @@config[:username]=username if username
      @@config[:password]=password if password
      f = File.open(@@config_file,'w+')
      f.puts @@config.to_yaml
      f.flush
      f.close
    end

    def Installer.signup(email,firstname,lastname,password)
      client = get_client
      result = client.send 'account.signup.request', {'offline'=>true,'email'=>email,'password'=>password,'firstname'=>firstname,'lastname'=>lastname}
      $titanium.debug "result=>#{result.to_yaml}" if OPTIONS[:debug] and result
      result ? result[:data] : {'success'=>false,'msg'=>'invalid response from server'}
    end

    def Installer.validate_signup(email,password,verification)
      client = get_client
      result = client.send 'account.confirmation.request', {'offline'=>true,'confirmation'=>verification}
      $titanium.debug "result=>#{result.to_yaml}" if OPTIONS[:debug] and result
      result ? result[:data] : {'success'=>false,'msg'=>'invalid response from server'}
    end

    def Installer.network_login(email,password,silent=false)
      if OPTIONS[:no_remote]
        # special exit value for external tools (like the ide)
        die("--no-remote has been specified and you need to go to the Dev Network for content.", 2)
      end
      $titanium.debug "Using network URL: #{OPTIONS[:server]}" if OPTIONS[:debug]
      $titanium.info "Connecting to update server ..." unless OPTIONS[:silent] or silent or OPTIONS[:quiet]
      client = get_client
      result = client.send 'account.login.request', {'email'=>email,'password'=>password}
      $titanium.debug "result=>#{result.to_yaml}" if OPTIONS[:debug] and result
      return result[:data]['success'] if result
      false
    end
    
    def Installer.login_if_required(quiet=false)
      Installer.login(nil,nil,false,quiet) unless @@loggedin
    end

    def Installer.get_client
      if @@client
        @@client
      else
        @@client = ServiceBrokerClient.new(OPTIONS[:server], OPTIONS[:debug])
      end
    end

    def Installer.get_proxy
      if !@@config[:proxy].nil?
        $titanium.debug "proxy in config: #{@@config[:proxy]}" if OPTIONS[:debug]
        return @@config[:proxy]
      elsif !@@config[:proxy_host].nil? and !@@config[:proxy_port].nil?
        $titanium.debug "proxy in config: http://#{@@config[:proxy_host]}:#{@@config[:proxy_port]}" if OPTIONS[:debug]
        return "http://#{@@config[:proxy_host]}:#{@@config[:proxy_port]}"
      elsif !ENV['http_proxy'].nil? and !(ENV['http_proxy']=="")
        $titanium.debug "proxy in ENV['http_proxy']: #{ENV['http_proxy']}" if OPTIONS[:debug]
        return ENV['http_proxy']
      elsif !ENV['HTTP_PROXY'].nil? and !(ENV['HTTP_PROXY']=="")
        $titanium.debug "proxy in ENV['HTTP_PROXY']: #{ENV['HTTP_PROXY']}" if OPTIONS[:debug]
        return ENV['HTTP_PROXY']
      else
        $titanium.debug "proxy nil" if OPTIONS[:debug]
        return nil
      end
    end

    def Installer.handleResponse(response,callback)
      if response["action"] == "login"
        
      elsif response["action"] == "signup"
        
      elsif response["action"] == "verify"
        
      end
    end
    
    def Installer.login(un=nil,pw=nil,exit_on_failure=false,force_quiet=false)
      if OPTIONS[:no_remote]
        die("--no-remote has been specified and you need to go to the Dev Network for content.", 2)
      end
      username = un.nil? ? @@config[:username] : un
      password = pw.nil? ? @@config[:password] : pw
      if not @@loggedin or (username.nil? or password.nil?) or (@@loggedin and (username != @@config[:username] or password != @@config[:password]))
        response = $titanium.showLoginForm()
        action = response["action"]
        if action == "login"
          if Installer.network_login(response["username"],response["password"],false)
              @@loggedin = true
              @@username = response["username"]
              @@password = response["password"]

              $titanium.info "Logged in" if OPTIONS[:verbose]

              Installer.save_config(@@username,@@password)
              if response.has_key?("proxy")
                Installer.save_proxy(response["proxy"])
              end
          
              @@loggedin
          else
            $titanium.showLoginFormError("Invalid credentials")
          end
        elsif action == "signup"
          signup_response = Installer.signup(response["email"],response["firstname"],response["lastname"],response["password"])

          if not signup_response['success']
            die "Signup failed. #{result['msg']}"
          end

          @@username = response["username"]
          @@password = response["password"]
          verification_response = $titanium.showVerificationForm()
          verification = verification_response["verification"]
          attempt_login = true
          if verification.nil? or verification == ''
            if Installer.network_login(@@username,@@password)
              attempt_login = false
              break
            else
              $titanium.error "Couldn't not validate your account. Please try again."
            end
          else
            validate_result = Installer.validate_signup(@@username,@@password,verification)
            if validate_result and validate_result['success']
              Installer.network_login(@@username, @@passowrd) if attempt_login
              break
            else
              $titanium.showVerificationFormError("Error validating your verification code. #{result['msg']}")
            end
          end
        end
      end
    end
    
    def Installer.logged_in
      @@loggedin
    end

    def Installer.save_proxy(proxy)
      @@config[:proxy]=proxy
      @@config[:proxy_host]=nil
      @@config[:proxy_port]=nil
      f = File.open(@@config_file,'w+')
      f.puts @@config.to_yaml
      f.flush
      f.close
    end

    def Installer.prompt_proxy(save=false)
      envproxy = ENV['HTTP_PROXY'] || ENV['http_proxy']
      if !envproxy.nil? && !(envproxy == '')
        yn = ask "Detected http_proxy environment variable #{envproxy}, do you want to use this? (Y)es or (N)o [Y]"
        if ['y','Y',''].index(yn)
          if save
            Installer.save_proxy(nil)
          end
          return nil
        else
          proxy = ask('Proxy url (ex: http://myuser:mypass@my.example.com:3128):')
          if save
            Installer.save_proxy(proxy)
          end
          return proxy
        end
      end
      yn = ask 'Are you using a proxy server? (Y)es or (N)o [N]'
      if ['y','Y'].index(yn)
        proxy = ask('Proxy url (ex: http://myuser:mypass@my.example.com:3128):')
        if save
          Installer.save_proxy(proxy)
        end
        return proxy
      else
        return nil
      end
    end

    def Installer.current_user
      #TODO: windows?
      require 'etc'
      uid=File.stat(APP_TEMP_DIR.to_s).uid
      Etc.getpwuid(uid).name
    end
    
    def Installer.admin_user?
      # ignore for win32 system
      return true if RUBY_PLATFORM =~ /mswin32$/
      
      # must be root user for unix
      un = current_user
      'root'==un
    end
    
    def Installer.require_admin_user
      if not admin_user?
        STDERR.puts
        STDERR.puts "*" * 80
        STDERR.puts
        STDERR.puts "ERROR: Administrative Privileges Required"
        STDERR.puts
        STDERR.puts "This operation requires you to be logged in as root/administrator user."
        STDERR.puts "Please login or sudo as root/administrator and re-run this command again."
        STDERR.puts
        STDERR.puts "*" * 80
        STDERR.puts
        exit 1
      end
    end
    
    def Installer.http_fetch_into(name,url,target)
      temp_dir = http_fetch(name,url)
      if temp_dir
        puts "extracting #{temp_dir} to #{target}" if OPTIONS[:debug]
        FileUtils.cp_r "#{temp_dir}/.", target
        true
      end
      false
    end
    
    def Installer.same_host?(a,b)
      a == b or (a =~ /localhost|127\.0\.0\.1|0\.0\.0\.0/ and b =~ /localhost|127\.0\.0\.1|0\.0\.0\.0/)
    end

    def Installer.http_fetch(name,url)
      $titanium.debug "Attempting to fetch #{name} from #{url}" if OPTIONS[:debug]
      begin
        Installer.login_if_required
      rescue => e
        $titanium.error "Failed to connect to network for login, exception => #{e}" 
        return nil
      end
      pbar=nil
      dirname=nil
      uri = URI.parse(url)
      home_uri = URI.parse(OPTIONS[:server])
      # workaround!
      #if same_host?(uri.host,home_uri.host) and uri.port == home_uri.port
      cookies = @@client.cookies.to_s
      
      puts "Session cookies: #{cookies}" if OPTIONS[:debug]

      proxy = Installer.get_proxy()
      if proxy.nil? or proxy==""
        proxy = false
      else
        puts "using proxy #{proxy}"
      end

      content_length_proc = lambda do |t|
        if t && 0 < t
          if not OPTIONS[:quiet]
            require "#{LIB_DIR}/progressbar"
            pbar = ProgressBar.new(name, t)
            pbar.file_transfer_mode
          end
        end
      end

      progress_proc = lambda do |s|
        pbar.set s if pbar
      end

      open(url,'Cookie'=>cookies,
      :proxy=>proxy,
      :content_length_proc => content_length_proc,
      :progress_proc => progress_proc) do |f|

        if f.status[0] == '200'
          tmpdir = tempdir
          t = tempfile(tmpdir)
          t.write f.read
          t.flush
          t.close
          size = File.size(t.path)
          dirname = File.dirname(t.path)
          if url =~ /\.tgz$/
            #FIXME - deal with windows or bundle in windows or something - or figure out how to do in ruby
            #system "tar xfz #{t.path} -C #{dirname}"
          elsif url =~ /\.zip$/ or f.content_type =~ /^application\/zip/
            Installer.unzip dirname,t.path
            FileUtils.rm_r t.path
            puts "Downloaded: #{size} bytes" if OPTIONS[:debug]
          end
        else
          die "Error fetching #{name}. Distribution server returned status code: #{f.status.join(' ')}."
        end
      end

      puts if pbar
      dirname
    end

    def Installer.unzip(dirname,path)
      require 'zip/zip'
      puts "Extracting #{path} to #{dirname}" if OPTIONS[:debug]
      Zip::ZipFile::open(path) do |zf|
        zf.each do |e|
          fpath = File.join(dirname, e.name)
          FileUtils.mkdir_p(File.dirname(fpath))
          puts "extracting ... #{fpath}" if OPTIONS[:debug]
          if File.file?(fpath)
            confirm("Overwrite [#{fpath}]?") {
              |answer|
              if answer == "Yes" || answer == "Always"
                FileUtils.rm_rf(fpath)
                zf.extract(e, fpath)
              end
            } unless OPTIONS[:quiet] or OPTIONS[:force]
          else
            zf.extract(e, fpath)
          end
        end
        zf.close
      end
    end

    def Installer.tempdir
      dir = File.expand_path(File.join(APP_TEMP_DIR,"#{$$}_#{rand(100000)}"))
      FileUtils.mkdir_p dir
      dir
    end

    def Installer.tempfile(dir=APP_TEMP_DIR)
      f = File.new(File.expand_path(File.join(dir,"#{$$}_#{rand(100000)}")),'wb')
      APP_TEMP_FILES << f
      f
    end
    
    def Installer.put_raw(path, content)
      f = File.open(path,'w+')
      f.puts content
      f.flush
      f.close
    end
    
    def Installer.put(path,content,force=false)
      save=true
      if File.exists?(path)
        $titanium.debug("Writing content to #{path}") OPTIONS[:debug]
        confirm("overwrite #{path}?") { 
          |answer|
          if answer == "Yes" || answer == "Always"
            Installer.put_raw(path, content)
          end
        } if not OPTIONS[:quiet] and not OPTIONS[:force] and not force
      else
        Installer.put_raw(path, content)
      end
    end

    #TODO: tx?
    def Installer.mkdir(path)
      case path.class.to_s
      when 'String'
        FileUtils.mkdir_p path unless File.exists?(path)
      when 'Array'
        path.each do |p|
          mkdir p
        end
      end
    end
    
    def Installer.confirm_copy(from,destination)
      return :force if OPTIONS[:force]
      return :force unless File.exists?(destination)
      return :skip if Installer.same_file?(from,destination)
      while true
        STDOUT.print "overwrite #{destination}? (enter \"h\" for help) [Ynbaqdh] "
        STDOUT.flush
        case STDIN.gets.chomp
          when /\Ad\z/i
            #FIXME: win32 package sdiff in installer
            fork do
               exec "sdiff \"#{destination}\" \"#{from}\" | less"
            end
            Process.wait
          when /\Aa\z/i
            STDOUT.puts "forcing #{from}"
            OPTIONS[:force] = true
            return :force
          when /\Ab\z/i
            return :backup
          when /\Aq\z/i
            STDOUT.puts "aborting from copy #{from}"
            raise SystemExit
          when /\An\z/i
            return :skip
          when /\Ah\z/i
            STDOUT.puts <<-HELP
Y - yes, overwrite
n - no, do not overwrite -- skip copy of this file
b - no, do not overwrite but create backup file as <name>.backup
a - all, overwrite this and all others
q - quit, abort command
d - diff, show the differences between the old and the new
h - help, show this help
HELP
          else
            return :force
        end
      end
    end

    def Installer.do_cp_confirm(tx,from_path,to_path,force=false)
      if force
        tx.cp from_path,to_path
        return true
      end
      case confirm_copy from_path,to_path
        when :force
          tx.cp from_path,to_path
        when :backup
          tx.cp from_path,"#{to_path}.backup"
          tx.cp from_path,to_path
        when :skip
      end
      true
    end


    def Installer.copy(tx,from_path,to_path,excludes=nil,force=false)

      if from_path.class == Array
        from_path.each do |e|
          Installer.copy(tx,e,to_path,excludes,force)
        end
        return true
      end

      puts "Copy called from: #{from_path} => to: #{to_path}, excludes=> #{excludes}, force=#{force}" if OPTIONS[:debug]

      if File.exists?(from_path) and File.file?(from_path)
        if File.directory?(to_path)
          return do_cp_confirm(tx,from_path,File.join(to_path,File.basename(from_path)),force)
        end
        return do_cp_confirm(tx,from_path,to_path,force)
      end

      Dir.glob("#{from_path}/**/*", File::FNM_DOTMATCH).each do |file|
        if excludes
          found = false
          excludes.each do |e|
            if file =~ Regexp.new("#{e}$")
              found = true
              next
            end
          end
          next if found
        end
        target_file = file.gsub(from_path,'')
        target = File.join(to_path,target_file)
        if File.directory?(file) and not File.exists?(target)
          puts "Creating directory #{target}" if OPTIONS[:verbose]
          tx.mkdir(target) unless File.exists?(target)
        end
        if File.file?(file) 
          puts "Copying #{file} to #{target}" if OPTIONS[:verbose]
          tx.mkdir File.dirname(target) unless File.exists?(target)
          do_cp_confirm tx,file,target,force
        end
      end
      true
    end
        
    def Installer.fetch_distribution_list(ping=false)
        return @@distributions if @@distributions # caching this
        login_if_required
        client = get_client
        puts "Fetching release info from distribution server..." unless OPTIONS[:quiet]
        config = get_config
        args = {'ping'=>ping,'sid'=>config[:sid],'os'=>RUBY_PLATFORM}
        response = client.send('distribution.query.request',args)
        if config[:sid].nil? or config[:sid]!=response[:data]['sid']
          config[:sid] = response[:data]['sid']
          save_config
        end
        @@distributions = response[:data]['distributions'].keys_to_sym
        with_site_config do |site_config|
          site_config[:distributions] = @@distributions
        end
        @@distributions
    end


    def Installer.each_installed_component_type
      with_site_config(false) do |site_config|
        installed = site_config[:installed] || {}
        installed.keys.each do |key|
          yield key.to_s
        end
      end
    end

    def Installer.installed_components(type)
      components = load_site_config[:installed][type.to_sym] rescue nil
      components || []
    end

    def Installer.get_websdk
      with_site_config(false) do |site_config|
        installed = site_config[:installed] || {}
        members = installed[:websdk] || []
        members.each do |member|
          if member[:name] == 'websdk'
            return member
          end
        end
      end
      nil
    end

    def Installer.find_dependencies_for(component)
      with_site_config(false) do |site_config|
        distro = site_config[:distributions]
        return nil unless distro
        members = distro[component[:type].to_sym]
        return nil unless members
        members.each do |m|
          if m[:name]==component[:name]
            return m[:dependencies]
          end
        end
      end
      nil
    end

    def Installer.dependency_specified?(component,dependencies)
      dependencies.each do |d|
        return true if d[:name]==component[:name] and d[:type]==component[:type]
      end
      false
    end

    def Installer.same?(a,b)
      a[:name]==b[:name] or a[:type]==b[:type]
    end

    def Installer.get_dependencies(component,dependencies=[])

      return [] unless component
      depends = component[:dependencies]
      return unless depends

      checked = Array.new

      depends.each do |d|
        next if same?(component,d)
        next if dependency_specified?(d,dependencies)
        next if installed_this_session? d
        dependencies << d
        checked << d[:name]
      end

      # recursively resolve dependencies
      sweeps = 0
      while sweeps < 10 # we can only sweep once per finger
        count = 0
        dependencies.each do |d|
          sweeps += 1
          depends = find_dependencies_for(d)
          next unless depends
          depends.each do |dd|
            next if Installer.same?(dd,component)
            next if checked.include?(dd[:name])
            next if installed_this_session? dd
            dependencies << dd
            count += 1
            checked << dd[:name]
          end
        end
        break unless count > 0
      end

      dependencies
    end

    def Installer.number_to_human_size(size, precision=1)
      size = size.nil? ? 0 : Kernel.Float(size)
      case
      when size.to_i == 1;    "1 Byte"
      when size < 1.kilobyte; "%d Bytes" % size
      when size < 1.megabyte; "%.#{precision}f KB"  % (size / 1.0.kilobyte)
      when size < 1.gigabyte; "%.#{precision}f MB"  % (size / 1.0.megabyte)
      when size < 1.terabyte; "%.#{precision}f GB"  % (size / 1.0.gigabyte)
      else                    "%.#{precision}f TB"  % (size / 1.0.terabyte)
      end.sub(/([0-9])\.?0+ /, '\1 ' )
    rescue
      size.to_s + ' bytes'
    end

    def Installer.get_and_install_dependencies(component,quiet=false)
      dependencies = get_dependencies(component)
      iterator_dependencies(dependencies,component,quiet) do |d,idx,len|
        yield d,idx,len
      end
    end

    def Installer.iterator_dependencies(dependencies,component,quiet=false)
      return nil unless dependencies
      if dependencies.length > 0

        if not OPTIONS[:force_update]
          dependencies = dependencies.inject([]) do |a,d|
            if not get_installed_components(d).empty?
              puts "Dependent #{d[:type]} #{d[:name]}, #{d[:version]} already installed - skipping..."  if OPTIONS[:verbose]
            else
              a << d
            end
            a
          end
          return nil if dependencies.empty?
        end

        str = dependencies.length > 1 ? 'ies' : 'y'
        if not OPTIONS[:quiet]
          puts
          puts "(#{dependencies.length}) Dependenc#{str} resolved that requires download:"
          puts "-"*120
          puts "| " + 'Type'.center(9) + ' | ' + 'Name'.ljust(72) + ' | ' + 'Version'.center(10) + ' | ' + 'Filesize'.rjust(16) + ' |'
          puts "-"*120
          total = 0
          dependencies.each do |d|
            type = d[:type]
            name = d[:name]
            version = d[:version]
            filesize = d[:filesize]
            puts "| #{type.center(9)} | #{name.ljust(72)} | #{version.center(10)} | #{number_to_human_size(filesize).rjust(16)} |"
            puts "-"*120
            total+=filesize
          end
          filesize = 0
          filesize = component[:filesize] if component
          puts "Total download size (including component):" + number_to_human_size(total+filesize).rjust(76)
          puts
        end

        dependencies.each_with_index do |d,idx|
          yield d,idx,dependencies.length
        end
      end
    end

    def Installer.get_build_from_zip(zipfile)
      require 'zip/zip'
      Zip::ZipFile::open(zipfile) do |zf|
        build_contents = zf.read 'build.yml' rescue nil
        return YAML::load(build_contents)
      end
      nil
    end

    #
    # Components are uniquely identified by type+name+version
    # With those things we can find where a component is (or would be) installed
    #
    def Installer.get_component_directory(component)
      Installer.get_release_directory(component[:type],component[:name],component[:version])
    end
    
    def Installer.get_release_directory(type,name,version)
      "#{RELEASE_DIR}/#{type}/#{name.gsub(':','_')}/#{version}"
    end


    def Installer.fetch_network_component(component,count=1,total=1)
      puts "fetch network component from url: #{component[:url]}" if OPTIONS[:verbose]
      Installer.fetch_component(component,count,total)
      component
    end

    def Installer.fetch_network_url(component,url)
      component[:url] = url
      Installer.fetch_component(component,1,1)
    end


    def Installer.fetch_component(component,idx,total)
      dir = component[:dir] = Installer.get_component_directory(component)
      url = component[:url]
      
      puts "fetching into: #{dir} => #{url}" if OPTIONS[:debug]
      FileUtils.mkdir_p dir unless File.exists? dir
      
      event = component.clone_keys(:dir, :url, :type, :name, :version)
      PluginManager.dispatchEvents('install_component',event) do
        Installer.http_fetch_into("(#{idx}/#{total}) #{component[:name]}",url,dir)
      end
    end
    
    def Installer.install_from_zipfile(type,zip_path)

      begin
        component = Installer.get_build_from_zip zip_path
      rescue Zip::ZipError => msg
        die "Could not install #{zip_path}: #{msg}"
      end

      die "Invalid package file #{zip_path}. Missing build.yml" unless component
      # this is strict, probably won't catch any blunders
      #die "Expected component of type '#{type}' in zipfile, found '#{component[:type]}'" unless type == component[:type]
      
      component[:checksum] = Installer.checksum(zip_path)
      dir = component[:dir] = Installer.get_component_directory(component)
      
      event = component.clone_keys(:dir, :type, :name, :version)
      event[:from] = zip_path
      
      PluginManager.dispatchEvents('install_component',event) do
        Installer.unzip dir,zip_path
      end
      component
    end


    # to be called by user-level commands
    # if the component is not installed, this will install it from the devnetwork,
    # if an older version of the component is installed, this will prompt the user to upgrade
    # if the user declines to upgrade, the installed version will be returned
    def Installer.require_component(type,name,version,options={})      
      
      options[:from] = name
      component_info = {:name=>name,:type=>type,:version=>version}
      
      if name =~ /^http:\/\//
        #FIXME
        STDERR.puts "not yet supported - download directly and then re-run with zipfile"
        exit 1
        
      elsif name =~ /\.zip$/
        # if given a zipfile, we install it without doing version or network checking
        component = install_from_zipfile(type,name)
        finish_install(component)
        
      elsif OPTIONS[:no_remote]
        # user doesn't want to connect to the network
        component = get_component(:local, component_info)
        die "--no-remote has been specified and you need to go to the Dev Network for content." unless component
        
      else
        # the name isn't a resource
        if version.nil?
          # user doesn't care about version, give them the latest
          component_info[:name] = component_info[:name].to_s
          local = get_current_installed_component(component_info)
          begin
            remote = get_current_remote_component(component_info)
          rescue SocketError => e
            # if we have no internet connection, just use local version
          end
          
          if (local.nil? and remote)
            # first install
            component = install_from_devnetwork(remote, options)
            finish_install(component, options)
                        
          elsif remote and should_update(local[:version],remote[:version],local[:checksum],remote[:checksum])
            # upgrading
            if OPTIONS[:force_update]
              
              confirm_yes("There is a newer version of '#{local[:name]}' (yours: #{local[:version]}, available: #{remote[:version]})  Install?")
              {
                |answer|
                if answer == "Yes" || answer == "Always"
                  component = install_from_devnetwork(remote, options)
                  finish_install(component, options)
                end
              }
              
            else
              # user is resisting upgrading
              component = local
              skip_install(component, options)              
            end
          
          elsif local # if we have this cached
            component = local
            skip_install(component, options)
          
          else
            msg = 'Component was not found locally or remotely'
            msg += ' (no network connection available)' if e
            raise UserError.new(msg)
          end
        
        else # user wants a specific version, try to use local
          
          local = get_installed_components(component_info).last
          
          if local and not OPTIONS[:force_update]
            # user asks for an installed version, no network hit
            component = local
            skip_install(component, options)
          else
            begin
              remote = get_remote_components(component_info).last
              component = install_from_devnetwork(remote, options)
              finish_install(component, options)
            rescue SocketError => e
              raise UserError.new("Component was not found locally or remotely (no network connection available)")
            end
          end
        end
      end
      
      component[:dir] = get_component_directory(component)     
      component
    end
    
    def Installer.finish_install(component, options={})
      puts "Fetched into #{component[:dir]}" if OPTIONS[:debug]

      # run pre_flight if it exists
      pre_flight = File.join(component[:dir],'pre_flight.rb')
      if File.exists?(pre_flight)
        puts "Running pre-flight file at #{pre_flight}" if OPTIONS[:verbose]
        
        $Installer = component.clone_keys(:type,:name,:version,:checksum)
        $Installer[:to_dir] = component[:dir]
        $Installer[:from] = options[:from]
        $Installer[:tx] = options[:tx]
        
        require pre_flight 
      end

      Installer.add_installed_component(component)
      
      puts unless OPTIONS[:quiet]
      puts "Installed #{component[:name]} #{component[:version]}" unless OPTIONS[:quiet]
    end
    
    def Installer.skip_install(component, options)
      puts "#{component[:name]} #{component[:version]} is already installed" unless OPTIONS[:quiet] or options[:quiet] or options[:quiet_if_installed]
      puts "NOTE: you can force a re-install with --force-update" if OPTIONS[:verbose]      
    end
    
    def Installer.add_installed_component(cm)
      type,name,version = cm[:type],cm[:name],cm[:version]
      
      with_site_config do |site_config|
        installed = site_config[:installed] ||= {}
        components = installed[type.to_sym] ||= []
        
        # don't re-add same one twice
        components.delete_if do |e|
          e[:name]==name and e[:type]==type and e[:version]==version
        end
        components << cm.clone_keys(:name,:type,:version,:checksum)
        @@installed_this_session << "#{type}_#{name}_#{version}"
      end
    end
    
    def Installer.installed_this_session?(cm)
      @@installed_this_session.include? "#{cm[:type]}_#{cm[:name]}_#{cm[:version]}"
    end
    
    def Installer.describe_component(component)
      case component[:type]
      when :service
        'Service'
      when :plugin
        component[:name]+' Plugin'
      when :widget
        'Widget'
      when :websdk
        'Web SDK'
      when :update
        'Self-Update'
      end
    end
    
    #
    #
    def Installer.install_from_devnetwork(component_info, options={})
      force = options[:force].nil? ? OPTIONS[:force_update] : options[:force]
      quiet_if_installed = options[:quiet_if_installed]
      
      Installer.selfupdate(nil, options)

      puts "Install from Dev Network: #{component_info[:type]},#{component_info[:name]},#{component_info[:version]} (force=#{force}) " if OPTIONS[:verbose]
      
      die "Couldn't find #{type.to_s} #{component_info[:name]}" unless component_info
      
      count = 0

      # check to see if we've already installed this component during this session
      # if so, don't try to get it again
      installed = Installer.installed_this_session? component_info

      if not options[:skip_dependencies]
        Installer.get_and_install_dependencies(component_info,quiet_if_installed) do |d,idx,total|
          local_cache = Installer.get_installed_components(d).last # need the right version
          if not local_cache or force
            t = total + (installed ? 1 : 0)
            component = Installer.fetch_network_component(d,idx+1,t)
            Installer.add_installed_component(component)
          end
          count+=1
        end
      end

      if not installed or force
        component = Installer.fetch_network_component(component_info,count+1,count+1)
      else
        component = component_info
      end
      
      component[:dir] = Installer.get_component_directory(component)
      component
    end
      
    def Installer.most_recent_version(components)
      if components and not components.empty?
        components.compact!
        components.sort! do |a,b|
          compare_versions(a[:version],b[:version])
        end
        return components.last
      end
      nil
    end
      
    def Installer.get_components(location, component_info)
      case location
      when :remote
        all_components = Installer.fetch_distribution_list
      when :local
        all_components = load_site_config[:installed]
      end
      
      begin
        name = component_info[:name]
        type = component_info[:type].to_sym
        version = component_info[:version]

        components = all_components[type] || []
        matching_components = components.select do |cm|
          cm[:name] == name and (version.nil? or cm[:version] == version)
        end
        
        if matching_components.empty? and location == :remote and component_info[:version]
          # only try to get an exact version if we can't find it normally
          [get_exact_remote_component(component_info)]
        else
          matching_components
        end
      rescue StandardError => e
        p e if OPTIONS[:debug]
        [] # odd cases, when there are no services installed locally, etc
      end
    end

    def Installer.get_component(location, component_info)
      components = get_components(location, component_info)
      if component_info[:version]
        components.last # there should only be one component per version
      else
        most_recent_version(components)
      end
    end
    
    def Installer.get_installed_components(component_info)
      get_components(:local, component_info)
    end
    
    def Installer.get_remote_components(component_info)
      get_components(:remote, component_info)
    end
    
    def Installer.get_current_installed_component(component_info)
      component_info = component_info.clone
      component_info[:version] = nil
      most_recent_version(get_installed_components(component_info))
    end
    
    def Installer.get_current_remote_component(component_info)
      component_info = component_info.clone
      component_info[:version] = nil
      most_recent_version(get_remote_components(component_info))
    end
    
    def Installer.get_current_available_component(component_info)
      local = get_current_installed_component(component_info)
      begin
        remote = get_current_remote_component(component_info)
        return most_recent_version([remote, local])
      rescue SocketError => e
        # maybe we're disconnected
        return local
      end
    end
    
    def Installer.get_exact_remote_component(component_info)
      message = {'sid'=>get_config[:sid],'os'=>RUBY_PLATFORM,'name'=>component_info[:name],'version'=>component_info[:version]}
      response = get_client.send('distribution.release.request', message)[:data]
      if response['success']
        component = response['release'].keys_to_sym
        component[:type] = component[:type].to_sym
        component
      else
        die 'Sorry, that version isn\'t available'
      end
    end
    
    
    def Installer.same_file?(file1,file2)
      return false unless File.exists?(file2)
      checksum1 = checksum file1
      checksum2 = checksum file2
      checksum1 == checksum2
    end
    
    def Installer.checksum(file)
      f = File.open file,'rb'
      md5 = Digest::MD5.hexdigest f.read
      f.close
      md5
    end
    
    
    def Installer.compare_with_local(component)
        if (component.class == String) # path to a zipfile
            component = Installer.get_build_from_zip(component)
        end
        installed_component = get_component(:local, component)

        if component
            return compare_versions(component, installed_component)
        else
            # no local version installed, so
            # this component is considered newer
            return 1 
        end
    end

    #
    # Site Configuration (Things installed locally in the $APPCELERATOR/releases directory) 
    #
    def Installer.load_site_config
      return @@site_config if @@site_config
      FileUtils.mkdir_p RELEASE_DIR unless File.exists? RELEASE_DIR
      @@site_config_file = "#{RELEASE_DIR}/config.yml"
      @@site_config = YAML::load_file @@site_config_file if File.exists?(@@site_config_file)
      @@site_config||={}
      @@site_config
    end
    
    def Installer.save_site_config
      if @@site_config
        FileUtils.mkdir_p RELEASE_DIR unless File.exists? RELEASE_DIR
        f = File.open @@site_config_file, 'w+'
        f.puts @@site_config.to_yaml
        f.close
      end
    end
    
    def Installer.with_site_config(save=true)
      config = load_site_config
      yield config
      save_site_config if save
    end
    
    #
    # Project Configuration (Things added to the current project)
    #
    def Installer.get_project_config(dir)
      config = YAML::load_file "#{dir}/config/appcelerator.config" if File.exists? "#{dir}/config/appcelerator.config"
      config||={}
      config
    end
    
    def Installer.is_project_dir?(dir)
      File.exists? "#{dir}/config/appcelerator.config"
    end
    
    def Installer.save_project_config(dir,config)
      puts "saving project config = #{dir}" if OPTIONS[:debug]
      out = YAML::dump(config)
      Installer.put "#{dir}/config/appcelerator.config", out, true
    end
    
    def Installer.with_project_config(dir,save=true)
      config = get_project_config dir
      yield config
      save_project_config dir,config if save
    end
    
    #
    # Version Utils
    #
    def Installer.compare_versions(first, second, *checksums) 
      return 0 unless (first and second)

      result = first.to_s.split('.').map {|n| n.to_i} <=> second.to_s.split('.').map {|n| n.to_i}

      if (result == 0 and checksums.length >= 2)
          return -1 unless (checksums[0] == checksums[1])
      end

      result
    end

    def Installer.should_update(local_version, remote_version, local_checksum, remote_checksum)
      case compare_versions(local_version, remote_version, local_checksum, remote_checksum)
        when 1
          false # mine is newer
        when 0
          OPTIONS[:force_update] # both the same, are we forcing?
        when -1
          true # mine is older
      end
    end
  
    
    def Installer.selfupdate(version=nil,options={})
    
      build_config = YAML::load_file(File.expand_path("#{SCRIPTDIR}/build.yml"))
      cm = {:type=>:update, :name=>'update', :version=>version}
      update = Installer.get_component(:remote, cm)

      if update
        
        if compare_versions(build_config[:version], update[:version]) == -1
          if confirm_yes "Self-update this program from #{build_config[:version]} to #{update[:version]} ? [Yna]"

            update_component = Installer.fetch_network_component(update,1,1)
            finish_install(update_component, options)

            build_config[:version] = update[:version]
            cf = File.open("#{SCRIPTDIR}/build.yml",'w+')
            cf.puts(build_config.to_yaml)
            cf.close
            puts "This program has been self-updated. Please run your command again."
          else
            return build_config[:version]
          end
        elsif version
          die "You can't update to an previous version of the command-line tool"
        else
          return build_config[:version]
        end
      end
    end    
  end
  
  class UserError < StandardError
  end
  
end

#
# bring these in since we can't depend on rails in this script
#

module NumericExtensions
    def bytes
      self
    end
    def kilobytes
      self * 1024
    end
    def megabytes
      self * 1024.kilobytes
    end
    def gigabytes
      self * 1024.megabytes
    end
    def terabytes
      self * 1024.gigabytes
    end
    alias :byte :bytes
    alias :kilobyte :kilobytes
    alias :megabyte :megabytes
    alias :gigabyte :gigabytes
    alias :terabyte :terabytes
end

class Fixnum
  include NumericExtensions
end

class Float
  include NumericExtensions
end

class Hash
  def clone_keys *keys
    result = {}
    keys.each do |k|
      result[k] = self[k]
    end
    result
  end
  def keys_to_sym
    result = {}
    each do |k,v|
      result[k.to_sym] = v.keys_to_sym rescue v
    end
    result
  end
end

class Array
  def keys_to_sym
    map do |v|
      v.keys_to_sym rescue v
    end
  end
end
