#define SC_INCLUDE_FX // enable the fixed-point data types

#include <iostream>
#include <string>
#include <systemc>
#include <sys/time.h>
#include "filter.h"
#include "testbench.h"

#define CLOCK_PERIOD 5

using namespace std;
using namespace sc_core;
using namespace sc_dt;


int sc_main(int argc, char *argv[]) {
    
    Testbench tb("tb");
    Filter filt("filt");
    sc_clock clk("clk", CLOCK_PERIOD, SC_NS);
    sc_signal<sc_logic> rst("rst");
    sc_fifo<sc_logic> finish;
    sc_fifo<sc_int<32> > x;
    sc_fifo<sc_int<32> > y;
    sc_fifo<sc_int<32> > i;
    sc_fifo<sc_int<32> > j;
    sc_fifo<sc_int<32> > img_width;
    sc_fifo<sc_int<32> > img_height;
    sc_fifo<sc_uint<8> > source_r;
    sc_fifo<sc_uint<8> > source_g;
    sc_fifo<sc_uint<8> > source_b;
    sc_fifo<sc_uint<8> > result_r;
    sc_fifo<sc_uint<8> > result_g;
    sc_fifo<sc_uint<8> > result_b;
    
    tb.rst(rst);
    tb.clk(clk);
    tb.o_r(source_r);
    tb.o_g(source_g);
    tb.o_b(source_b);
    tb.i_finish(finish);
    tb.i_r(result_r);
    tb.i_g(result_g);
    tb.i_b(result_b);
    tb.i_x(x);
    tb.i_y(y);
    tb.i_i(i);
    tb.i_j(j);
    tb.o_height(img_height);
    tb.o_width(img_width);
    filt.clk(clk);
    filt.rst(rst);
    filt.i_r(source_r);
    filt.i_g(source_g);
    filt.i_b(source_b);
    filt.o_r(result_r);
    filt.o_g(result_g);
    filt.o_b(result_b);
    filt.o_x(x);
    filt.o_y(y);
    filt.o_i(i);
    filt.o_j(j);
    filt.i_height(img_height);
    filt.i_width(img_width);
    filt.o_finish(finish);

    
    cout << "reading bitmap..." << '\t' << sc_time_stamp()<< endl;
    tb.read_bmp();
    
    cout << "starting simulation..." << '\t' << sc_time_stamp()<< endl;

    sc_start();
    
    cout << "finish simulation" << '\t' << sc_time_stamp()<< endl;
    tb.write_bmp();
    cout << "writing bitmap..." << '\t' << sc_time_stamp()<< endl;
    return (0);
}