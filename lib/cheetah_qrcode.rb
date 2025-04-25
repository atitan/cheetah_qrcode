# frozen_string_literal: true

require_relative "cheetah_qrcode/version"

module CheetahQRCode
  def self.encode(text, ec_level: :m, border: 4, size: 0)
    encode_text(text, ec_level, border, size)
  end
end

require 'cheetah_qrcode/cheetah_qrcode'
