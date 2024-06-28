# frozen_string_literal: true

require_relative "lib/io/watch/version"

Gem::Specification.new do |spec|
	spec.name = "io-watch"
	spec.version = IO::Watch::VERSION
	
	spec.summary = "A tool for watching changes to the filesystem."
	spec.authors = ["Samuel Williams"]
	spec.license = "MIT"
	
	spec.cert_chain  = ['release.cert']
	spec.signing_key = File.expand_path('~/.gem/release.pem')
	
	spec.homepage = "https://github.com/socketry/io-watch"
	
	spec.metadata = {
		"documentation_uri" => "https://socketry.github.io/io-watch/",
		"source_code_uri" => "https://github.com/socketry/io-watch.git",
	}
	
	spec.files = Dir['{bin,ext,lib}/**/*', '*.md', base: __dir__]
	spec.require_paths = ['lib']
	
	spec.executables = ['io-watch']
	
	spec.extensions = ["ext/configure"]
	
	spec.required_ruby_version = ">= 3.1"
end
