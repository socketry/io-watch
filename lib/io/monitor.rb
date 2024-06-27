require_relative 'monitor/version'
require_relative 'monitor/filesystem'

class IO
	module Monitor
		def self.new(...)
			Filesystem.new(...)
		end
	end
end
