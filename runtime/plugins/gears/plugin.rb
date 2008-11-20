require 'fileutils'

class GearsPlugin < Titanium::Plugin
  def initialize
    super("gears_" + platform_string)
  end
  
  def install(project, basedir, executable_name)
    super(project, basedir, executable_name)
      
    if is_mac?
      FileUtils.cp_r File.join(@plugindir, 'Gears.plugin'), @plugins_path
    elsif is_win?
      FileUtils.cp File.join(@plugindir, 'gears_titanium.dll'), File.join(@plugins_path, 'npgears_titanium.dll')
	    FileUtils.chmod 0755, File.join(@plugins_path, 'npgears_titanium.dll')
    end
  end
  
  def get_native_plugin
    if is_mac?
      return File.join(@plugindir, 'Gears.plugin')
    elsif is_win?
      return File.join(@plugindir, 'gears_titanium.dll')
    end
  end
end
