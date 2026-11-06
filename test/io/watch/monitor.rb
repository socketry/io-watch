# frozen_string_literal: true

# Released under the MIT License.
# Copyright, 2024, by Samuel Williams.

require 'io/watch/monitor'
require 'tmpdir'

describe IO::Watch::Monitor do
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
			watch = subject.new([root])
			watch.run do |event|
				changes << event
			end
		end
		
		# Wait for the watch to start...
		changes.pop
		
		test_file = File.join(root, 'test.txt')
		File.write(test_file, 'hello')
		
		event = changes.pop
		expect(event[:root]).to be == root
	ensure
		thread.kill
	end
	
	it 'should detect a new directory and changes within it' do
		changes = Thread::Queue.new
		
		thread = Thread.new do
			watch = subject.new([root])
			watch.run do |event|
				changes << event
			end
		end
		
		# Wait for the watch to start...
		changes.pop
		
		test_dir = File.join(root, 'test')
		Dir.mkdir(test_dir)
		
		event = changes.pop
		expect(event[:root]).to be == root
		
		test_file = File.join(test_dir, 'test.txt')
		File.write(test_file, 'hello')
		
		event = changes.pop
		expect(event[:root]).to be == root
	ensure
		thread.kill
	end
	
	it "should detect directory additions and removals" do
		changes = Thread::Queue.new
		
		thread = Thread.new do
			watch = subject.new([root])
			watch.run do |event|
				changes << event
			end
		end
		
		# Wait for the watch to start...
		changes.pop
		
		test_dir = File.join(root, 'test')
		Dir.mkdir(test_dir)
		
		event = changes.pop
		expect(event[:root]).to be == root
		
		Dir.rmdir(test_dir)
		
		event = changes.pop
		expect(event[:root]).to be == root
	ensure
		thread.kill
	end
	
	it "should ignore file accesses" do
		# Create a test file:
		test_file = File.join(root, 'test.txt')
		File.write(test_file, 'hello')
		
		changes = Thread::Queue.new
		
		# Start watching the directory:
		thread = Thread.new do
			watch = subject.new([root])
			watch.run do |event|
				changes << event
			end
		end
		
		# Wait for the watch to start...
		changes.pop
		
		File.read(test_file)
		
		expect(changes).to be(:empty?)
	ensure
		thread.kill
	end
	
	it "coallesce multiple events" do
		changes = Thread::Queue.new
		
		thread = Thread.new do
			watch = subject.new([root])
			watch.run do |event|
				changes << event
			end
		end
		
		# Wait for the watch to start...
		changes.pop
		
		test_file = File.join(root, 'test.txt')
		File.write(test_file, 'hello')
		File.write(test_file, 'world')
		
		event = changes.pop
		expect(event[:root]).to be == root
		
		expect(changes).to be(:empty?)
	ensure
		thread.kill
	end
	
	it "does not leak inheritable file descriptors into the helper" do
		# For example, a server socket bound with `close_on_exec: false`:
		input, output = IO.pipe
		output.close_on_exec = false
		
		changes = Thread::Queue.new
		
		thread = Thread.new do
			watch = subject.new([root])
			watch.run do |event|
				changes << event
			end
		end
		
		# Wait for the watch to start...
		changes.pop
		
		# If the helper inherited the write end, this would never reach EOF:
		output.close
		expect(input.wait_readable(1)).to be_truthy
		expect(input.read_nonblock(1, exception: false)).to be_nil
	ensure
		thread.kill
		input&.close
		output&.close
	end
end
