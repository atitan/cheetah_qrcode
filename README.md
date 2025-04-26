# Cheetah QR Code

Fast QR Code png generator for Ruby

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

## Credits

This project includes files from the following repos:

- [libspng](https://github.com/randy408/libspng) by randy408
- [qrcodegen](https://github.com/nayuki/QR-Code-generator) by nayuki

License notice for their work is included either in file header or by license file besides included files.
