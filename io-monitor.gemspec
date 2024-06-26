# frozen_string_literal: true

require_relative "lib/io/monitor/version"

Gem::Specification.new do |spec|
	spec.name = "io-monitor"
	spec.version = IO::Monitor::VERSION
	
	spec.summary = "A tool for monitoring changes to the filesystem."
	spec.authors = ["Samuel Williams"]
	spec.license = "MIT"
	
	spec.cert_chain  = ['release.cert']
	spec.signing_key = File.expand_path('~/.gem/release.pem')
	
	spec.homepage = "https://github.com/socketry/io-monitor"
	
	spec.metadata = {
		"source_code_uri" => "https://github.com/socketry/io-monitor.git",
	}
	
	spec.files = Dir['{ext,lib}/**/*', '*.md', base: __dir__]
	spec.require_paths = ['lib']
	
	spec.extensions = ["ext/configure"]
	
	spec.required_ruby_version = ">= 3.1"
end
