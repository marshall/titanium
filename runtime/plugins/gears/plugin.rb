require 'fileutils'

class GearsPlugin < Titanium::Plugin
  def initialize(platform)
    super("gears_" + platform, platform)
  end
  
  def install(project, basedir, executable_name)
    super(project, basedir, executable_name)
      
    if @platform == "osx"
      FileUtils.cp_r File.join(@plugindir, 'GearsTitanium.plugin'), @plugins_path
    elsif @platform == "win32"
      FileUtils.cp File.join(@plugindir, 'gears_titanium.dll'), File.join(@plugins_path, 'npgears_titanium.dll')
	    FileUtils.chmod 0755, File.join(@plugins_path, 'npgears_titanium.dll')
    end
  end
  
  def get_native_plugin
    if @platform == "osx"
      return File.join(@plugindir, 'GearsTitanium.plugin')
    elsif @platform == "win32"
      return File.join(@plugindir, 'gears_titanium.dll')
    end
  end
end
