#include "testbench.h"
#include <systemc>

Testbench::Testbench(sc_module_name n): sc_module(n){
  SC_THREAD(input_data);
    sensitive << clk.pos();
  SC_THREAD(output_data);
    sensitive << clk.pos();
  dont_initialize();
}

int Testbench::read_bmp(){
    FILE* fp_s = fopen("input.bmp", "rb");
    if (fp_s == nullptr) {
        std::cerr << "fopen fp_s error" << std::endl;
        return -1;
    }

    fseek(fp_s, 10, SEEK_SET);
    assert(fread(&rgb_raw_data_offset, sizeof(unsigned int), 1, fp_s));

    fseek(fp_s, 18, SEEK_SET);
    assert(fread(&width, sizeof(unsigned int), 1, fp_s));
    assert(fread(&height, sizeof(unsigned int), 1, fp_s));

    fseek(fp_s, 28, SEEK_SET);
    assert(fread(&bit_per_pixel, sizeof(unsigned short), 1, fp_s));
    byte_per_pixel = bit_per_pixel / 8;

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
int Testbench::write_bmp(){
    unsigned int file_size = 0; // file size
    FILE* fp_t = fopen("output.bmp", "wb");
    if (fp_t == nullptr) {
        std::cerr << "fopen fname_t error" << std::endl;
        return -1;
    }
    file_size = width * height * byte_per_pixel + rgb_raw_data_offset;
    header[2] = (unsigned char)(file_size & 0x000000ff);
    header[3] = (file_size >> 8) & 0x000000ff;
    header[4] = (file_size >> 16) & 0x000000ff;
    header[5] = (file_size >> 24) & 0x000000ff;
    header[18] = width & 0x000000ff;
    header[19] = (width >> 8) & 0x000000ff;
    header[20] = (width >> 16) & 0x000000ff;
    header[21] = (width >> 24) & 0x000000ff;
    header[22] = height & 0x000000ff;
    header[23] = (height >> 8) & 0x000000ff;
    header[24] = (height >> 16) & 0x000000ff;
    header[25] = (height >> 24) & 0x000000ff;
    header[28] = bit_per_pixel;
    fwrite(header, sizeof(unsigned char), rgb_raw_data_offset, fp_t);
    fwrite(image_t, sizeof(unsigned char),
            (size_t)(long)width * height * byte_per_pixel, fp_t);

    fclose(fp_t);

    return 0;
}
void Testbench::input_data(){
  
  int offset = 28;
  sc_int<32> i, j, x, y;
  o_width.write(width);
  o_height.write(height);
  while(1){
    
    //cout << "reading i j x y  in input" << endl;
    i = i_i.read();
    j = i_j.read();
    x = i_x.read();
    y = i_y.read();
    
    //cout << "writing r g b  in input" << endl;
    o_r.write(*(image_s + byte_per_pixel * (width * (y+i) + x + j + offset) + 2));
    o_g.write(*(image_s + byte_per_pixel * (width * (y+i) + x + j + offset) + 1));
    o_b.write(*(image_s + byte_per_pixel * (width * (y+i) + x + j + offset) + 0));
    //cout << "finish writing in input" << endl;
  }

}
void Testbench::output_data(){
  
  sc_int<32> x, y;
  sc_uint<8> R, G, B;
  
  while(1){
    
    //cout << "reading i j x y r g b in output" << endl;
    x = i_x.read();
    y = i_y.read();
    R = i_r.read();
    G = i_g.read();
    B = i_b.read();
    //cout << "finish reading in output" << endl;
    
    *(image_t + byte_per_pixel * (width * y + x) + 2) = R;
    *(image_t + byte_per_pixel * (width * y + x) + 1) = G;
    *(image_t + byte_per_pixel * (width * y + x) + 0) = B;
  }
}