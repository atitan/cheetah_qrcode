#include <ruby.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include "qrcodegen.h"
#include "spng.h"

static VALUE encode_text(int argc, VALUE* argv, VALUE self);

static VALUE encode_text(int argc, VALUE* argv, VALUE self) {
        VALUE arg_text, arg_ec_level, arg_border, arg_size, png_string = Qnil;
        bool ok;
        int error_code;
        size_t qrcode_ec_level, qrcode_modules, qrcode_border, qrcode_size;
        size_t image_size, image_length, png_size;
        float image_scale;
        uint8_t qrcode[qrcodegen_BUFFER_LEN_MAX];
        uint8_t qrcode_buffer[qrcodegen_BUFFER_LEN_MAX];
        uint8_t *image = NULL;
        spng_ctx *ctx = NULL;
        void *png_buffer = NULL;

        rb_scan_args(argc, argv, "40", &arg_text, &arg_ec_level, &arg_border, &arg_size);

        if (TYPE(arg_text) != T_STRING) {
                rb_raise(rb_eTypeError, "Invalid text");
        }

        if (TYPE(arg_ec_level) == T_SYMBOL) {
                arg_ec_level = SYM2ID(arg_ec_level);
        } else {
                rb_raise(rb_eTypeError, "Invalid error correction level");
        }

        if (arg_ec_level == rb_intern("L") || arg_ec_level == rb_intern("l")) {
                qrcode_ec_level = qrcodegen_Ecc_LOW;
        } else if (arg_ec_level == rb_intern("M") || arg_ec_level == rb_intern("m")) {
                qrcode_ec_level = qrcodegen_Ecc_MEDIUM;
        } else if (arg_ec_level == rb_intern("Q") || arg_ec_level == rb_intern("q")) {
                qrcode_ec_level = qrcodegen_Ecc_QUARTILE;
        } else if (arg_ec_level == rb_intern("H") || arg_ec_level == rb_intern("h")) {
                qrcode_ec_level = qrcodegen_Ecc_HIGH;
        } else {
                rb_raise(rb_eTypeError, "Invalid error correction level");
        }

        if (TYPE(arg_border) == T_FIXNUM) {
                qrcode_border = NUM2UINT(arg_border);
        } else {
                rb_raise(rb_eTypeError, "Invalid border");
        }

        if (TYPE(arg_size) == T_FIXNUM) {
                image_size = NUM2UINT(arg_size);
        } else {
                rb_raise(rb_eTypeError, "Invalid size");
        }

        ok = qrcodegen_encodeText(
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

        // Dimension of image to output
        image_length = image_size * image_size;
        image_scale = (float)qrcode_size / image_size;

        image = calloc(image_length, sizeof(uint8_t));
        if (!image) {
                rb_raise(rb_eRuntimeError, "Unable to create image buffer");
        }

        // Map every dot back to original qrcode using image_scale
        // to resize and fill pixels into image buffer at the same time
        for (size_t y = 0; y < image_size; y++) {
                for (size_t x = 0; x < image_size; x++) {
                        size_t i = (y * image_size) + x;

                        int qrcode_x = (int)floor(x * image_scale) - qrcode_border;
                        int qrcode_y = (int)floor(y * image_scale) - qrcode_border;

                        if (qrcodegen_getModule(qrcode, qrcode_x, qrcode_y)) {
                                image[i] = 0; // Black
                        } else {
                                image[i] = 255; // White
                        }
                }
        }

        // Proceed to create PNG after image resize
        ctx = spng_ctx_new(SPNG_CTX_ENCODER);

        // Use internal buffer provided by spng
        spng_set_option(ctx, SPNG_ENCODE_TO_BUFFER, 1);

        // Set PNG IHDR
        struct spng_ihdr ihdr = {0};
        ihdr.width = image_size;
        ihdr.height = image_size;
        ihdr.color_type = SPNG_COLOR_TYPE_GRAYSCALE;
        ihdr.bit_depth = 8;
        spng_set_ihdr(ctx, &ihdr);

        // SPNG_ENCODE_FINALIZE will finalize the PNG with the end-of-file marker
        error_code = spng_encode_image(ctx, image, image_length, SPNG_FMT_PNG, SPNG_ENCODE_FINALIZE);
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

        if (png_string == Qnil) {
                rb_raise(rb_eRuntimeError, "Unable to encode image");
        }

        return png_string;
}

void Init_cheetah_qrcode(void) {
        VALUE cCheetahQRCode = rb_const_get(rb_cObject, rb_intern("CheetahQRCode"));

        rb_define_singleton_method(cCheetahQRCode, "encode_text", encode_text, -1);
}
