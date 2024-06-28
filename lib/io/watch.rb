require_relative 'watch/version'
require_relative 'watch/monitor'

class IO
	module Watch
		def self.new(...)
			Monitor.new(...)
		end
	end
end
