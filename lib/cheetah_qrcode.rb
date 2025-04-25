# frozen_string_literal: true

module CheetahQRCode
  VERSION = '1.0.0'

  def self.encode(text, ec_level: :m, border: 4, size: 0)
    encode_text(text, ec_level, border, size)
  end
end

require 'cheetah_qrcode/cheetah_qrcode'
