# frozen_string_literal: true

# Released under the MIT License.
# Copyright, 2024, by Samuel Williams.

source 'https://rubygems.org'

gemspec

group :test do
	gem "sus"
	gem "covered"
	gem "decode"
	gem "rubocop"
	
	gem "bake-test"
	gem "bake-test-external"
end

group :maintenance, optional: true do
	gem "bake-gem"
	gem "bake-modernize"
	
	gem "utopia-project"
end
