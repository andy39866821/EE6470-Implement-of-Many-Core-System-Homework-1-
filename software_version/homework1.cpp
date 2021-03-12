/*
Filename    : sobel.cpp
Compiler    : Clang++ 8.0.0
Description : Demo the how to use sobel detector on gray level image
*/
#include <cassert>
#include <cmath>
#include <iostream>


const int filterWidth = 3;
const int filterHeight = 3;

unsigned char *image_s = nullptr; // source image array
unsigned char *image_t = nullptr; // target image array
FILE *fp_s = nullptr;             // source file handler
FILE *fp_t = nullptr;             // target file handler

unsigned int width = 0;               // image width
unsigned int height = 0;              // image height
unsigned int rgb_raw_data_offset = 0; // RGB raw data offset
unsigned int bit_per_pixel = 0;      // bit per pixel
unsigned int byte_per_pixel = 0;    // byte per pixel

// bitmap header
unsigned char header[54] = {
    0x42,          // identity : B
    0x4d,          // identity : M
    0,    0, 0, 0, // file size
    0,    0,       // reserved1
    0,    0,       // reserved2
    54,   0, 0, 0, // RGB data offset
    40,   0, 0, 0, // struct BITMAPINFOHEADER size
    0,    0, 0, 0, // bmp width
    0,    0, 0, 0, // bmp height
    1,    0,       // planes
    24,   0,       // bit per pixel
    0,    0, 0, 0, // compression
    0,    0, 0, 0, // data size
    0,    0, 0, 0, // h resolution
    0,    0, 0, 0, // v resolution
    0,    0, 0, 0, // used colors
    0,    0, 0, 0  // important colors
};

double filter[filterHeight][filterWidth] = {
  {0.077847, 0.123317, 0.077847},
  {0.123317, 0.195346, 0.123317},
  {0.077847, 0.123317, 0.077847}
};
double factor = 1.0;
double bias = 0.0;

int read_bmp(const char *fname_s) {
    fp_s = fopen(fname_s, "rb");
    if (fp_s == nullptr) {
        std::cerr << "fopen fp_s error" << std::endl;
        return -1;
    }

    // move offset to 10 to find rgb raw data offset
    fseek(fp_s, 10, SEEK_SET);
    assert(fread(&rgb_raw_data_offset, sizeof(unsigned int), 1, fp_s));

    // move offset to 18 to get width & height;
    fseek(fp_s, 18, SEEK_SET);
    assert(fread(&width, sizeof(unsigned int), 1, fp_s));
    assert(fread(&height, sizeof(unsigned int), 1, fp_s));

    // get bit per pixel
    fseek(fp_s, 28, SEEK_SET);
    assert(fread(&bit_per_pixel, sizeof(unsigned short), 1, fp_s));
    byte_per_pixel = bit_per_pixel / 8;

    // move offset to rgb_raw_data_offset to get RGB raw data
    fseek(fp_s, rgb_raw_data_offset, SEEK_SET);

    size_t size = width * height * byte_per_pixel;
    image_s = reinterpret_cast<unsigned char *>(new void *[size]);
    if (image_s == nullptr) {
        std::cerr << "allocate image_s error" << std::endl;
        return -1;
    }

    image_t = reinterpret_cast<unsigned char *>(new void *[size]);
    if (image_t == nullptr) {
        std::cerr << "allocate image_t error" << std::endl;
        return -1;
    }

    assert(fread(image_s, sizeof(unsigned char),
                (size_t)(long)width * height * byte_per_pixel, fp_s));
    fclose(fp_s);

    return 0;
}



int filting(double threshold) {
    int x, y, i, j; // for loop counter
    double  R, G, B;      // color of R, G, B
    int offset = 28;
    for (y = 0; y != height; ++y) {
        for (x = 0; x != width; ++x) {
            R = G = B = 0;

            for (i=-1 ; i<filterHeight-1 ; ++i) {
                for (j=-1 ; j<filterWidth-1 ; ++j) {
                    if(0<=y+i && y+i<height && 0<=x+j && x+j<width) {
                        R += (double)*(image_s + byte_per_pixel * (width * (y+i) + x + j + offset) + 2) * filter[i+1][j+1];
                        G += (double)*(image_s + byte_per_pixel * (width * (y+i) + x + j + offset) + 1) * filter[i+1][j+1];
                        B += (double)*(image_s + byte_per_pixel * (width * (y+i) + x + j + offset) + 0) * filter[i+1][j+1];
                    }
                }
            }
            *(image_t + byte_per_pixel * (width * y + x) + 2) = R;
            *(image_t + byte_per_pixel * (width * y + x) + 1) = G;
            *(image_t + byte_per_pixel * (width * y + x) + 0) = B;
        }
    }

    return 0;
}

int write_bmp(const char *fname_t) {
  unsigned int file_size = 0; // file size

  fp_t = fopen(fname_t, "wb");
  if (fp_t == nullptr) {
    std::cerr << "fopen fname_t error" << std::endl;
    return -1;
  }

  // file size
  file_size = width * height * byte_per_pixel + rgb_raw_data_offset;
  header[2] = (unsigned char)(file_size & 0x000000ff);
  header[3] = (file_size >> 8) & 0x000000ff;
  header[4] = (file_size >> 16) & 0x000000ff;
  header[5] = (file_size >> 24) & 0x000000ff;

  // width
  header[18] = width & 0x000000ff;
  header[19] = (width >> 8) & 0x000000ff;
  header[20] = (width >> 16) & 0x000000ff;
  header[21] = (width >> 24) & 0x000000ff;

  // height
  header[22] = height & 0x000000ff;
  header[23] = (height >> 8) & 0x000000ff;
  header[24] = (height >> 16) & 0x000000ff;
  header[25] = (height >> 24) & 0x000000ff;

  // bit per pixel
  header[28] = bit_per_pixel;

  // write header
  fwrite(header, sizeof(unsigned char), rgb_raw_data_offset, fp_t);

  // write image
  fwrite(image_t, sizeof(unsigned char),
         (size_t)(long)width * height * byte_per_pixel, fp_t);

  fclose(fp_t);

  return 0;
}

int main(void) {
    assert(read_bmp("lena.bmp") == 0); // 24 bit gray level image
    filting(90.0);
    assert(write_bmp("lena_sobel.bmp") == 0);
    delete (image_s);
    delete (image_t);

    return 0;
}
