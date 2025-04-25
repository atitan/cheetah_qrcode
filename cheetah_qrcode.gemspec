# frozen_string_literal: true

require File.expand_path('lib/cheetah_qrcode.rb', __dir__)

Gem::Specification.new do |s|
  s.name          = 'cheetah_qrcode'
  s.version       = CheetahQRCode::VERSION
  s.authors       = ['atitan']
  s.email         = ['commit@atifans.net']
  s.licenses      = ['MIT']
  s.require_path  = 'lib'

  s.files       = Dir['{lib,ext}/**/*.{rb,h,c}']
  s.extensions  = ['ext/cheetah_qrcode/extconf.rb']

  s.required_ruby_version = '>= 2.6'

  s.add_development_dependency 'rake'
  s.add_development_dependency 'rake-compiler'
end
