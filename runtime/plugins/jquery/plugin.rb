require 'fileutils'

class JQueryPlugin < Titanium::Plugin
  def initialize(platform)
    super("jquery",platform)
  end
end
