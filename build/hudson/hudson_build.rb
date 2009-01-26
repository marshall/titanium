require 'erb'
require 'fileutils'

tidir = ENV['WORKSPACE']
krolldir = File.join(tidir, 'kroll')

ENV['TITANIUM_DIR'] = tidir
ENV['KROLL_DIR'] = krolldir

gbinding = binding

def copy_template(template, dest)
	contents = ''
	File.open(template, 'r') { |file| contents = file.read }
	t = ERB.new(contents)
	results = t.result(gbinding)
	File.open(dest, 'w+') { |file| file.write(results) }
end

def git_revision(dir)
	lines = 0
	FileUtils.cd dir do
		git = 'git'
		if RUBY_PLATFORM =~ /win/i
			git = 'git.cmd'
		end

		result = `#{git} log --pretty=oneline`
		result.each_line { lines += 1 }
	end
	return lines
end

KROLL_REVISION = git_revision(krolldir)
TITANIUM_REVISION = git_revision(tidir)

copy_template(File.join(krolldir, 'build', 'hudson', 'config.yml'), 
	File.join(krolldir, 'config.yml'))

FileUtils.cd krolldir do
	`rake kroll:dev`
end

copy_template(File.join(tidir, 'build', 'hudson', 'config.yml'), 
	File.join(tidir, 'config.yml'))

FileUtils.cd tidir do
	`rake titanium:dev`
end

stage = File.join(ENV['APPC_SDK'], 'stage')

FileUtils.mkdir_p File.join(tidir, 'stage')
Dir[stage+'/titanium*,'+stage+'/kroll*'].each do |f|
	FileUtils.cp_r f, File.join(tidir, 'stage')
end
