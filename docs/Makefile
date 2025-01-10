.PHONY=runserver

install:
	bundle install

vendor/bundle: install

Gemfile: vendor/bundle install

runserver: Gemfile
	bundle exec jekyll serve --livereload --baseurl "/MaSzyna-API-wrapper"
