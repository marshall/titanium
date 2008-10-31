require 'fileutils'

class GearsPlugin < Titanium::Plugin
  def initialize
    super("gears_" + platform_string)
  end
  
  def install(project, basedir, executable_name)
    super(project, basedir, executable_name)
    if is_mac?
      plugins_dir = File.join(basedir, executable_name+".app", 'Contents', 'Plug-ins')
      FileUtils.mkdir_p plugins_dir
      FileUtils.cp_r File.join(@plugindir, 'Gears.plugin'), plugins_dir
    end
  end
end