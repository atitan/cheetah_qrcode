# frozen_string_literal: true

require_relative 'lib/cheetah_qrcode/version'

Gem::Specification.new do |s|
  s.name          = 'cheetah_qrcode'
  s.summary       = 'super fast qrcode generator'
  s.version       = CheetahQRCode::VERSION
  s.authors       = ['atitan']
  s.email         = ['commit@atifans.net']
  s.homepage      = 'https://github.com/atitan/cheetah_qrcode'
  s.licenses      = ['MIT']
  s.require_path  = 'lib'

  s.files       = Dir['{lib,ext}/**/*.{rb,h,c,LICENSE}', 'LICENSE', 'README.md']
  s.extensions  = ['ext/cheetah_qrcode/extconf.rb']

  s.required_ruby_version = '>= 2.6.0'

  s.add_development_dependency 'rake'
  s.add_development_dependency 'rake-compiler'
end
