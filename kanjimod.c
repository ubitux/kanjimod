#define _GNU_SOURCE 1
#include <dlfcn.h>

#include <stdint.h>
#include <stdio.h>
#include <GL/gl.h>

#include <ft2build.h>
#include FT_FREETYPE_H

static FT_Library library;
static FT_Face face;

extern const uint8_t *kan_bitmaps[];

void glutInit(int *argcp, char **argv)
{
    void (*orig_glutInit)(int*, char**) = dlsym(RTLD_NEXT, "glutInit");
    orig_glutInit(argcp, argv);

    printf("Init FT library\n");
    if (FT_Init_FreeType(&library)) {
        fprintf(stderr, "FT init error\n");
        return;
    }
    if (FT_New_Face(library, "/usr/share/fonts/TTF/HanaMinA.ttf", 0, &face)) {
        fprintf(stderr, "FT font face load error\n");
        return;
    }
    if (FT_Set_Char_Size(face, 0, 8<<6, 300, 300)) {
        fprintf(stderr, "FT set char size error\n");
        return;
    }
}

static inline void bitmap_mode_init(void)
{
    glPushClientAttrib(GL_CLIENT_PIXEL_STORE_BIT);
    glPixelStorei(GL_UNPACK_SWAP_BYTES,  GL_FALSE);
    glPixelStorei(GL_UNPACK_LSB_FIRST,   GL_FALSE);
    glPixelStorei(GL_UNPACK_ROW_LENGTH,  0);
    glPixelStorei(GL_UNPACK_SKIP_ROWS,   0);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
    glPixelStorei(GL_UNPACK_ALIGNMENT,   1);
}

static inline void bitmap_mode_end(void)
{
    glPopClientAttrib();
}

static void display_label(const uint32_t *s)
{
    int i;

    bitmap_mode_init();

    for (i = 0; s[i]; i++) {
        FT_GlyphSlot slot;
        FT_Bitmap *bitmap;
        uint8_t buf[4096];
        int w, h, advance, x_start, y_start, y, ft_pitch, gl_pitch;

        if (FT_Load_Char(face, s[i], FT_LOAD_RENDER|FT_LOAD_TARGET_MONO)) {
            fprintf(stderr, "FT loading/rendering char error\n");
            break;
        }

        slot = face->glyph;
        bitmap = &slot->bitmap;
        w = bitmap->width;
        h = bitmap->rows;

        /* freetype insanity, not even working properly in all cases yet */
        advance = slot->advance.x >> 6;
        x_start = advance - slot->bitmap_left;
        y_start = h - slot->bitmap_top;

        /* freetype and opengl bitmap linesizes may mistmatch; typically in
         * the case of of a very thin characters such as "l", ",", "'". */
        ft_pitch = bitmap->pitch;
        gl_pitch = (w + 7) >> 3; // up-rounded div by 8

        /* reverse bitmap (GL has the y origin on the bottom) */
        for (y = 0; y < h; y++)
            memcpy(buf + y*gl_pitch, bitmap->buffer + (h-y-1)*ft_pitch, gl_pitch);

        glBitmap(w, h, x_start, y_start, advance, 0, buf);
    }

    bitmap_mode_end();
}

static void display_bitmap(const uint8_t *buf, int w, int h)
{
    bitmap_mode_init();
    glBitmap(w, h, w/2, 0, w, 0, buf);
    bitmap_mode_end();
}

void glutBitmapCharacter(void *font, int character)
{
    static uint32_t buf[256];
    static int buf_pos;
    static uint32_t val;
    static uint32_t top;
    static int wait_next_char;

    static int wait_close_bracket;
    static int pic_id;

    uint8_t chr = character;

    /* handle special radicals (pictures) */
    if (wait_close_bracket) {
        if (character == ']') {
            const uint8_t *kan = kan_bitmaps[pic_id];
            display_bitmap(kan + 2, kan[0], kan[1]);
            wait_close_bracket = pic_id = 0;
        } else {
            pic_id = pic_id*10 + character - '0';
        }
        return;
    } else {
        wait_close_bracket = character == '[';
        if (wait_close_bracket)
            return;
    }

    /* re-construct utf-32 codes */
    if (wait_next_char) {
        const int tmp = chr - 128;
        if (tmp >> 6)
            goto err;
        val = (val << 6) + tmp;
        top <<= 5;
    } else {
        val = chr;
        if ((val & 0xc0) == 0x80 || val >= 0xfe)
            goto err;
        top = (val & 128) >> 1;
    }
    wait_next_char = (val & top);
    if (!wait_next_char)
        val &= (top << 1) - 1;

    /* character is now complete, check if the string is complete */
    if (!wait_next_char) {
        if (val == '@') { // End of string hack
            buf[buf_pos] = 0;
            buf_pos = 0;
            display_label(buf);
        } else if (buf_pos < sizeof(buf)/sizeof(*buf) - 1) {
            buf[buf_pos++] = val;
        }
    }
    return;

err:
    fprintf(stderr, "Invalid utf-8 byte code %02x\n", chr);
    val = top = wait_next_char = 0;
}
