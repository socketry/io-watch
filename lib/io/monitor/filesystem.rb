require 'json'

class IO
	module Monitor
		class Filesystem
			def self.command_path
				if extensions_path = Gem.loaded_specs['io-monitor']&.extensions_dir
					if File.exist?(extensions_path)
						return File.join(extensions_path, 'io-monitor')
					end
				end
				
				return File.join(__dir__, '../../../ext/bin/io-monitor')
			end
			
			COMMAND_PATH = self.command_path
			
			def initialize(roots, latency = 0.1)
				@roots = roots
				@latency = latency
			end
			
			def run
				environment = {'IO_MONITOR_LATENCY' => @latency.to_s}
				IO.pipe do |input, output|
					pid = Process.spawn(environment, self.class.command_path, *@roots, out: output)
					output.close
					
					input.each_line do |line|
						event = JSON.parse(line, symbolize_names: true)
						
						if index = event[:index]
							event[:path] = @roots[index]
						end
						
						yield event
					end
				ensure
					Process.kill('TERM', pid) if pid
				end
			end
		end
	end
end