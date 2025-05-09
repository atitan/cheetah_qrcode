#include <ruby.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "qrcodegen.h"
#include "spng.h"

static VALUE encode_text(int argc, VALUE* argv, VALUE self);

static VALUE encode_text(int argc, VALUE* argv, VALUE self) {
        VALUE arg_text, arg_ec_level, arg_border, arg_size, png_string = Qnil;
        size_t qrcode_ec_level, qrcode_modules, qrcode_border, qrcode_size;
        size_t image_size, image_scanline_width, image_length, png_size;
        float image_scale;
        uint8_t qrcode[qrcodegen_BUFFER_LEN_MAX];
        uint8_t qrcode_buffer[qrcodegen_BUFFER_LEN_MAX];
        uint8_t *image = NULL;
        spng_ctx *ctx = NULL;
        void *png_buffer = NULL;

        rb_scan_args(argc, argv, "40", &arg_text, &arg_ec_level, &arg_border, &arg_size);

        if (rb_type(arg_text) != T_STRING) {
                rb_raise(rb_eTypeError, "Invalid text");
        }

        if (rb_type(arg_ec_level) != T_SYMBOL) {
                rb_raise(rb_eTypeError, "Invalid error correction level, use :L, :M, :Q, :H");
        }

        if (rb_type(arg_border) != T_FIXNUM) {
                rb_raise(rb_eTypeError, "Invalid border");
        }

        if (rb_type(arg_size) != T_FIXNUM) {
                rb_raise(rb_eTypeError, "Invalid size");
        }

        arg_ec_level = RB_SYM2ID(arg_ec_level);

        if (arg_ec_level == rb_intern("L") || arg_ec_level == rb_intern("l")) {
                qrcode_ec_level = qrcodegen_Ecc_LOW;
        } else if (arg_ec_level == rb_intern("M") || arg_ec_level == rb_intern("m")) {
                qrcode_ec_level = qrcodegen_Ecc_MEDIUM;
        } else if (arg_ec_level == rb_intern("Q") || arg_ec_level == rb_intern("q")) {
                qrcode_ec_level = qrcodegen_Ecc_QUARTILE;
        } else if (arg_ec_level == rb_intern("H") || arg_ec_level == rb_intern("h")) {
                qrcode_ec_level = qrcodegen_Ecc_HIGH;
        } else {
                rb_raise(rb_eTypeError, "Invalid error correction level, use :L, :M, :Q, :H");
        }

        qrcode_border = RB_NUM2UINT(arg_border);

        // The max version qrcode is only 117 wide
        if (qrcode_border > 128) {
                rb_raise(rb_eTypeError, "Border too large");
        }

        image_size = RB_NUM2UINT(arg_size);

        // 204800x204800 will use 5000MiB memory for image buffer
        if (image_size > 204800) {
                rb_raise(rb_eTypeError, "Image size too large");
        }

        bool ok = qrcodegen_encodeText(
                RSTRING_PTR(arg_text),
                qrcode_buffer,
                qrcode,
                qrcode_ec_level,
                qrcodegen_VERSION_MIN,
                qrcodegen_VERSION_MAX,
                qrcodegen_Mask_AUTO,
                true
        );
        if (!ok) {
                rb_raise(rb_eRuntimeError, "Unable to create QR Code, maybe it's too large");
        }

        // Dimension of original qrcode
        qrcode_modules = qrcodegen_getSize(qrcode);
        qrcode_size = qrcode_modules + (qrcode_border * 2);

        // Do not resize if no size is supplied
        if (image_size == 0) {
                image_size = qrcode_size;
        }

        // Prevent downscale
        if (image_size < qrcode_size) {
                rb_raise(rb_eArgError, "Downscale QR Code will result in data loss");
        }

        // Dimension of output image
        // Image is consist of scanline_width * lines(height)
        // For 100x100 image, lines(height) is always 100
        // @8bit: scanline_width = 8 bit * 100 = 800 bits = 100 bytes
        //        buffer size needed = 100 bytes * 100 lines = 10000 bytes
        // @1bit: scanline_width = 1 bit * 100 = 100 bits = 13 bytes(round up)
        //        buffer size needed = 13 bytes * 100 lines = 1300 bytes
        image_scanline_width = (image_size + 7) / 8;
        image_length = image_scanline_width * image_size;
        image_scale = (float)image_size / qrcode_size;

        // Create image initialized to 0 (Entirely black image)
        image = calloc(image_length, sizeof(uint8_t));
        if (!image) {
                rb_raise(rb_eRuntimeError, "Unable to create image buffer for %lu bytes", image_length);
        }

        // Lookup map for qrcode-to-image pixel conversion
        size_t image_scale_map[qrcode_size + 1];

        // First pixel always start at 0
        image_scale_map[0] = 0;

        // Last pixel always end at image_size
        image_scale_map[qrcode_size] = image_size;

        // Precompute image scale pixels
        for (size_t i = 1; i < qrcode_size; i++) {
                image_scale_map[i] = (size_t)(i * image_scale);
        }

        // Loop through qrcode to find white modules to write into image
        for (size_t qy = 0; qy < qrcode_size; qy++) {
                for (size_t qx = 0; qx < qrcode_size; qx++) {
                        // Skip black qrcode module
                        if (qrcodegen_getModule(qrcode, qx - qrcode_border, qy - qrcode_border)) {
                                continue;
                        }

                        // Map current qrcode module to image coordinates
                        size_t ix_begin = image_scale_map[qx];
                        size_t ix_end = image_scale_map[qx + 1];
                        size_t iy_begin = image_scale_map[qy];
                        size_t iy_end = image_scale_map[qy + 1];

                        // Fill white pixels into image at unit of bytes
                        for (size_t iy = iy_begin; iy < iy_end; iy++) {
                                size_t ix = ix_begin;
                                size_t bits_remaining = ix_end - ix_begin;

                                while (bits_remaining > 0) {
                                        size_t i = (iy * image_scanline_width) + (ix / 8);

                                        uint8_t byte_value = 0xFF;
                                        uint8_t bits_needed = 8 - (ix % 8);

                                        if (bits_needed > bits_remaining) {
                                                bits_needed = bits_remaining;
                                                byte_value <<= (8 - bits_needed);
                                        }

                                        byte_value >>= (ix % 8);
                                        image[i] |= byte_value;

                                        ix += bits_needed;
                                        bits_remaining -= bits_needed;
                                }
                        }
                }
        }

        // Proceed to create PNG
        ctx = spng_ctx_new(SPNG_CTX_ENCODER);

        // Use internal buffer provided by spng
        spng_set_option(ctx, SPNG_ENCODE_TO_BUFFER, 1);

        // Set PNG IHDR
        struct spng_ihdr ihdr = {0};
        ihdr.width = image_size;
        ihdr.height = image_size;
        ihdr.color_type = SPNG_COLOR_TYPE_GRAYSCALE;
        ihdr.bit_depth = 1;
        spng_set_ihdr(ctx, &ihdr);

        // SPNG_ENCODE_FINALIZE will finalize the PNG with the end-of-file marker
        int error_code = spng_encode_image(ctx, image, image_length, SPNG_FMT_PNG, SPNG_ENCODE_FINALIZE);
        if (!error_code) {
                // Retrieve png from spng internal buffer
                png_buffer = spng_get_png_buffer(ctx, &png_size, &error_code);
                if (png_buffer) {
                        png_string = rb_str_new(png_buffer, png_size);
                }
        }

        // After calling spng_get_png_buffer(), png_buffer is then owned by us
        // We have to free it manually
        spng_ctx_free(ctx);
        free(image);
        free(png_buffer);

        if (error_code) {
                rb_raise(rb_eRuntimeError, "Unable to encode image: %s", spng_strerror(error_code));
        }

        return png_string;
}

void Init_cheetah_qrcode(void) {
        VALUE cCheetahQRCode = rb_const_get(rb_cObject, rb_intern("CheetahQRCode"));

        rb_define_singleton_method(cCheetahQRCode, "encode_text", encode_text, -1);
}
