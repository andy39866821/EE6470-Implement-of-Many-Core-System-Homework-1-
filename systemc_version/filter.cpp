#include "filter.h"


Filter::Filter(sc_module_name n): sc_module(n){
  SC_THREAD(convolution);
    sensitive << clk.pos();
  dont_initialize();
}

void Filter::convolution(){
    
    sc_int<32> height, width, i, j, x, y;
    sc_uint<8> source_r, source_g, source_b;
    sc_fixed<32, 24>  R, G, B;      // color of R, G, B
    
    width = i_width.read();
    height = i_height.read();
    for (y = 0; y != height; ++y) {
        for (x = 0; x != width; ++x) {
            //cout << "reset RGB :" << x << " " << y << endl;
            R = G = B = 0;
            for (i=-1 ; i<filterHeight-1 ; ++i) {
                for (j=-1 ; j<filterWidth-1 ; ++j) {
                    if(0<=y+i && y+i<height && 0<=x+j && x+j<width) {
                        wait();
                        //cout << "x: " << x << " y: " << y << " writing i j x y in filter" << endl;
                        o_i.write(i);
                        o_j.write(j);
                        o_x.write(x);
                        o_y.write(y);
                        //cout << "x: " << x << " y: " << y << " reading R G B in filter" << endl;
                        source_r = i_r.read();
                        source_g = i_g.read();
                        source_b = i_b.read();
                        //cout << "finish reading in filter" << endl;
                        R += (sc_fixed<32, 24>)source_r * filter[i+1][j+1];
                        G += (sc_fixed<32, 24>)source_g * filter[i+1][j+1];
                        B += (sc_fixed<32, 24>)source_b * filter[i+1][j+1];
                    }
                }
            }
            
            o_x.write(x);
            o_y.write(y);
            o_r.write((sc_uint<8>)(R/factor));
            o_g.write((sc_uint<8>)(G/factor));
            o_b.write((sc_uint<8>)(B/factor));
        }
    }



    sc_stop();
}
