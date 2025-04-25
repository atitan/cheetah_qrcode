# frozen_string_literal: true

require 'mkmf'

REQUIRED_HEADER = %w[
  ruby.h
].freeze

REQUIRED_HEADER.each do |header|
  abort "missing header: #{header}" unless have_header(header)
end

create_makefile 'cheetah_qrcode/cheetah_qrcode'
