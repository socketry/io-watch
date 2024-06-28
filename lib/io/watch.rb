# frozen_string_literal: true

# Released under the MIT License.
# Copyright, 2024, by Samuel Williams.

require_relative 'watch/version'
require_relative 'watch/monitor'

class IO
	module Watch
		def self.new(...)
			Monitor.new(...)
		end
	end
end
