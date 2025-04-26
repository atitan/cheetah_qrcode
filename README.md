[![Gem Version](https://badge.fury.io/rb/cheetah_qrcode.svg)](https://badge.fury.io/rb/cheetah_qrcode)

# Cheetah QR Code

Fast QR Code png generator for Ruby

This library sacrificed everything for speed. It is only suitable for people who just need a plain QR Code png.

## Install

```shell
gem install cheetah_qrcode
```

## Usage

```ruby
require 'cheetah_qrcode'

# ==== Attributes
# * +text+ - Text to generate QR Code, required
# * +:ec_level+ - Error correction level for QR Code, accepts :L, :M, :Q, :H, default is :M
# * +:border+ - Quiet zone modules outside of QR Code, default is 4
# * +:size+ - Output image size, cannot be smaller than QR Code modules size, default is QR Code modules size
CheetahQRCode.encode('test', ec_level: :q, border: 0, size: 600)
```
## Benchmark

### script

```ruby
require 'benchmark'
require 'rqrcode'
require 'cheetah_qrcode'

text = 'xtufa+wH4Fh/gx2uKgv/r8IRv2ZNV94b4v983GUouFRMT6k5'

puts "RQRCode: #{RQRCode::VERSION}, RQRCodeCore: #{RQRCodeCore::VERSION}, ChunkyPNG: #{ChunkyPNG::VERSION}"
Benchmark.bm do |x|
  x.report do
    1000.times do
      RQRCode::QRCode.new(text, level: :q).as_png(border_modules: 4, size: 300).to_blob
    end
  end
end

puts "CheetahQRCode: #{CheetahQRCode::VERSION}"
Benchmark.bm do |x|
  x.report do
    1000.times do
      CheetahQRCode.encode(text, ec_level: :q, border: 4, size: 300)
    end
  end
end
```

### result

```
RQRCode: 2.2.0, RQRCodeCore: 1.2.0, ChunkyPNG: 1.4.0
       user     system      total        real
  43.523532   0.041370  43.564902 ( 43.567882)

CheetahQRCode: 1.0.0
       user     system      total        real
   0.485618   0.000000   0.485618 (  0.485627)
```

## Credits

This project includes files from the following repos:

- [libspng](https://github.com/randy408/libspng) by randy408
- [qrcodegen](https://github.com/nayuki/QR-Code-generator) by nayuki

License notices for their work are included either in file headers or by license file besides included files.
