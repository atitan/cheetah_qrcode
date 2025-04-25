require File.expand_path("../lib/cheetah_qrcode.rb", __FILE__)

Gem::Specification.new do |s|
  s.name          = 'cheetah_qrcode'
  s.version       = CheetahQRCode::VERSION
  s.authors       = ['atitan']
  s.email         = ['commit@atifans.net']
  s.require_path  = 'lib'

  s.files       = Dir['{lib,ext}/**/*.{rb,h,c}']
  s.extensions  = ['ext/cheetah_qrcode/extconf.rb']

  s.add_development_dependency 'rake-compiler'
  s.add_development_dependency 'rake'
end
