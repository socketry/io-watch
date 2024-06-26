class IO
	module FSEvent
		class Monitor
			def initialize(roots, latency = 0.1, &block)
				@roots = roots
				@latency = latency
				@block = block
			end
			
			def start
				
			end
		end
	end
end