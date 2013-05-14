#!/usr/bin/env python2

import kandb, Image

array_str = bitmaps_str = ''
for bmp_id, elem in enumerate(kandb.data):
    if not elem:
        continue
    kan = elem['kan']
    if kan[0] == '@':
        im = Image.open('data/img/%s' % kan[1:])
        w, h = im.size
        bytestream = [w, h]
        for j in range(h):
            byte = 0
            k = 0
            for i in range(w):
                bit = int(im.getpixel((i, h-j-1)) < 127)
                byte = byte<<1 | bit
                k += 1
                if k == 8:
                    bytestream.append(byte)
                    k = byte = 0
            if k:
                bytestream.append(byte << (7-k))
        bstr = ','.join('0x%02x' % b for b in bytestream)
        bitmaps_str += 'static const uint8_t bmp_%d[] = {%s};\n' % (bmp_id, bstr)
        array_str += '    [%d] = bmp_%d,\n' % (elem['imgid'], bmp_id)

print '''
#include <stdint.h>

%s

const uint8_t *kan_bitmaps[] = {
%s
};
''' % (bitmaps_str, array_str)
