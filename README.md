# RLE_codec

A simple codec using RLE for 24bit bmp images

Compress each channel with RLE

# What we need
stb_image.h & stb_image_write.h

# How to use it

```jsx
#encode
%s -e test.bmp test.rle
#decode
%s -d test.rle test.bmp
```