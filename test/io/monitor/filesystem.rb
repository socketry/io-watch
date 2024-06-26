require 'io/monitor/filesystem'
require 'tmpdir'

describe IO::Monitor::Filesystem do
	attr :root
	
	def around
		Dir.mktmpdir do |dir|
			@root = dir
			
			super
		end
	end
	
	it 'should detect changes to a file' do
		changes = Thread::Queue.new
		
		thread = Thread.new do
			monitor = IO::Monitor::Filesystem.new([root])
			monitor.run do |event|
				changes << event
			end
		end
		
		# Wait for the monitor to start...
		changes.pop
		
		test_file = File.join(root, 'test.txt')
		File.write(test_file, 'hello')
		
		event = changes.pop
		expect(event[:path]).to be == root
	end
end
