# frozen_string_literal: true

require_relative "lib/io/fsevent/version"

Gem::Specification.new do |spec|
	spec.name = "io-fsevent"
	spec.version = IO::FSEvent::VERSION
	
	spec.summary = "An FSEvent monitor."
	spec.authors = ["Samuel Williams"]
	spec.license = "MIT"
	
	spec.cert_chain  = ['release.cert']
	spec.signing_key = File.expand_path('~/.gem/release.pem')
	
	spec.homepage = "https://github.com/socketry/io-fsevent"
	
	spec.files = Dir['ext/Makefile', 'ext/**/*.{c,h}', '{lib}/**/*', '*.md', base: __dir__]
	spec.require_paths = ['lib']
	
	spec.extensions = ["ext/configure"]
	
	spec.required_ruby_version = ">= 3.1"
end