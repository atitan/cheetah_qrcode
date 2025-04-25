#include <ruby.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include "qrcodegen.h"
#include "spng.h"

static VALUE encode(int argc, VALUE* argv, VALUE self);

static VALUE encode(int argc, VALUE* argv, VALUE self) {
        VALUE text, errCorLvl, png_string = Qnil;
        bool ok;
        int qrcode_modules, qrcode_border, qrcode_size;
        int image_size, image_length, ret;
        float image_scale;
        size_t png_size;
        uint8_t qrcode[qrcodegen_BUFFER_LEN_MAX];
        uint8_t qrcode_buffer[qrcodegen_BUFFER_LEN_MAX];
        uint8_t *image = NULL;
        spng_ctx *ctx = NULL;
        void *png_buffer = NULL;

        rb_scan_args(argc, argv, "20", &text, &errCorLvl);

        if (TYPE(text) != T_STRING) {
                rb_raise(rb_eTypeError, "Invalid text");
        }

        if (TYPE(errCorLvl) != T_FIXNUM) {
                rb_raise(rb_eTypeError, "Invalid error correction level");
        }

        switch (NUM2UINT(errCorLvl)) {
        case qrcodegen_Ecc_LOW:
        case qrcodegen_Ecc_MEDIUM:
        case qrcodegen_Ecc_QUARTILE:
        case qrcodegen_Ecc_HIGH:
                break;
        default:
                rb_raise(rb_eTypeError, "Invalid error correction level");
        }

        ok = qrcodegen_encodeText(
                RSTRING_PTR(text),
                qrcode_buffer,
                qrcode,
                NUM2UINT(errCorLvl),
                qrcodegen_VERSION_MIN,
                qrcodegen_VERSION_MAX,
                qrcodegen_Mask_AUTO,
                true
        );

        if (!ok) {
                rb_raise(rb_eRuntimeError, "Unable to create QR Code");
        }

        qrcode_modules = qrcodegen_getSize(qrcode);
        qrcode_border = 4;
        qrcode_size = qrcode_modules + (qrcode_border * 2);
        image_size = 580;
        image_length = image_size * image_size;
        image_scale = (float)qrcode_size / image_size;

        image = calloc(image_length, sizeof(uint8_t));
        if (image == NULL) {
                rb_raise(rb_eRuntimeError, "Unable to create image buffer");
        }

        for (int y = 0; y < image_size; y++) {
                for (int x = 0; x < image_size; x++) {
                        int i = (y * image_size) + x;

                        int qrcode_x = (int)round(x * image_scale) - qrcode_border;
                        int qrcode_y = (int)round(y * image_scale) - qrcode_border;

                        if (qrcodegen_getModule(qrcode, qrcode_x, qrcode_y)) {
                                image[i] = 0; // Black
                        } else {
                                image[i] = 255; // White
                        }
                }
        }

        /* Creating an encoder context requires a flag */
        ctx = spng_ctx_new(SPNG_CTX_ENCODER);

        /* Encode to internal buffer managed by the library */
        spng_set_option(ctx, SPNG_ENCODE_TO_BUFFER, 1);

        struct spng_ihdr ihdr = {0};
        ihdr.width = image_size;
        ihdr.height = image_size;
        ihdr.color_type = SPNG_COLOR_TYPE_GRAYSCALE;
        ihdr.bit_depth = 8;

        spng_set_ihdr(ctx, &ihdr);

        /* SPNG_ENCODE_FINALIZE will finalize the PNG with the end-of-file marker */
        ret = spng_encode_image(ctx, image, image_length, SPNG_FMT_PNG, SPNG_ENCODE_FINALIZE);
        if (ret) {
                goto encode_error;
        }

        png_buffer = spng_get_png_buffer(ctx, &png_size, &ret);
        if (png_buffer != NULL) {
                png_string = rb_str_new(png_buffer, png_size);
        }

encode_error:
        spng_ctx_free(ctx);
        free(image);
        free(png_buffer);

        return png_string;
}

void Init_cheetah_qrcode(void) {
        VALUE cCheetahQRCode = rb_const_get(rb_cObject, rb_intern("CheetahQRCode"));

        rb_define_const(cCheetahQRCode, "ECC_L", UINT2NUM(qrcodegen_Ecc_LOW));
        rb_define_const(cCheetahQRCode, "ECC_M", UINT2NUM(qrcodegen_Ecc_MEDIUM));
        rb_define_const(cCheetahQRCode, "ECC_Q", UINT2NUM(qrcodegen_Ecc_QUARTILE));
        rb_define_const(cCheetahQRCode, "ECC_H", UINT2NUM(qrcodegen_Ecc_HIGH));

        rb_define_singleton_method(cCheetahQRCode, "encode", encode, -1);
}
