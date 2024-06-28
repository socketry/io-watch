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
		expect(event[:path]).to be == root
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
		expect(event[:path]).to be == root
		
		test_file = File.join(test_dir, 'test.txt')
		File.write(test_file, 'hello')
		
		event = changes.pop
		expect(event[:path]).to be == root
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
		expect(event[:path]).to be == root
		
		Dir.rmdir(test_dir)
		
		event = changes.pop
		expect(event[:path]).to be == root
	ensure
		thread.kill
	end
end
