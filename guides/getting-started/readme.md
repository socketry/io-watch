# Getting Started

This guide explains how to use the `io-watch` gem for watching files and directories for changes.

## Installation

Add the gem to your project:

~~~ bash
$ gem install io-watch
~~~

## Usage

Use a instance of {ruby IO::Watch::Monitor} to watch files and directories for changes.

~~~ ruby
require "io/watch"

# Provide a list of root directories to watch:
monitor = IO::Watch::Monitor.new(["."])

monitor.run do |event|
	puts event
end
~~~
