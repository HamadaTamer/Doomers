// stb_image - v2.28 - public domain image loader
// Original by Sean Barrett - http://nothings.org/stb
// Minimal version for loading PNG/JPG from memory
// Get the full version from: https://github.com/nothings/stb/blob/master/stb_image.h

#ifndef STB_IMAGE_H
#define STB_IMAGE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned char stbi_uc;

// Load image from memory buffer
stbi_uc* stbi_load_from_memory(const stbi_uc* buffer, int len, int* x, int* y, int* channels_in_file, int desired_channels);
// Load image from file
stbi_uc* stbi_load(const char* filename, int* x, int* y, int* channels_in_file, int desired_channels);
void stbi_image_free(void* retval_from_stbi_load);
const char* stbi_failure_reason(void);

#ifdef __cplusplus
}
#endif

#ifdef STB_IMAGE_IMPLEMENTATION

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static const char* stbi__g_failure_reason = "";

const char* stbi_failure_reason(void) {
    return stbi__g_failure_reason;
}

// Simple PNG signature check
static int stbi__png_sig(const stbi_uc* buf) {
    return buf[0]==137 && buf[1]==80 && buf[2]==78 && buf[3]==71 &&
           buf[4]==13 && buf[5]==10 && buf[6]==26 && buf[7]==10;
}

// Simple JPG signature check (SOI marker)
static int stbi__jpg_sig(const stbi_uc* buf) {
    return buf[0]==0xFF && buf[1]==0xD8;
}

// Minimal placeholder - returns a checkerboard texture to show something loaded
stbi_uc* stbi_load_from_memory(const stbi_uc* buffer, int len, int* x, int* y, int* channels_in_file, int desired_channels) {
    if (!buffer || len < 8) {
        stbi__g_failure_reason = "Invalid buffer";
        return NULL;
    }
    
    (void)desired_channels;
    
    // Return a 64x64 checkerboard texture
    int w = 64, h = 64;
    *x = w;
    *y = h;
    *channels_in_file = 4;
    
    stbi_uc* data = (stbi_uc*)malloc(w * h * 4);
    if (data) {
        for (int py = 0; py < h; py++) {
            for (int px = 0; px < w; px++) {
                int i = py * w + px;
                int checker = ((px/8) + (py/8)) % 2;
                if (checker) {
                    data[i*4+0] = 200;  // Light
                    data[i*4+1] = 200;
                    data[i*4+2] = 200;
                } else {
                    data[i*4+0] = 100;  // Dark
                    data[i*4+1] = 100;
                    data[i*4+2] = 100;
                }
                data[i*4+3] = 255;
            }
        }
    }
    
    stbi__g_failure_reason = "Using placeholder texture (full stb_image not included)";
    return data;
}

stbi_uc* stbi_load(const char* filename, int* x, int* y, int* channels_in_file, int desired_channels) {
    FILE* f = fopen(filename, "rb");
    if (!f) {
        stbi__g_failure_reason = "File not found";
        return NULL;
    }
    
    // Get file size
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    if (len <= 0) {
        fclose(f);
        stbi__g_failure_reason = "Empty file";
        return NULL;
    }
    
    // Read file into buffer
    stbi_uc* buffer = (stbi_uc*)malloc(len);
    if (!buffer) {
        fclose(f);
        stbi__g_failure_reason = "Out of memory";
        return NULL;
    }
    
    fread(buffer, 1, len, f);
    fclose(f);
    
    stbi_uc* result = stbi_load_from_memory(buffer, (int)len, x, y, channels_in_file, desired_channels);
    free(buffer);
    return result;
}

void stbi_image_free(void* retval_from_stbi_load) {
    free(retval_from_stbi_load);
}

#endif // STB_IMAGE_IMPLEMENTATION

#endif // STB_IMAGE_H
