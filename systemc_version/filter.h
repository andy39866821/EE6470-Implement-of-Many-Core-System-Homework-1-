#define SC_INCLUDE_FX // enable the fixed-point data types
#include <systemc>
#include <sys/time.h> 


using namespace std;
using namespace sc_core;
using namespace sc_dt;

class Filter: public sc_module {
private:

    const int filterWidth = 3;
    const int filterHeight = 3;
    sc_fixed <32, 24> filter[3][3] = {
        {0.077847, 0.123317, 0.077847},
        {0.123317, 0.195346, 0.123317},
        {0.077847, 0.123317, 0.077847}
    };
    sc_fixed <32, 24> factor = 1;


    void convolution();
public:
    sc_in_clk clk;
    sc_in<sc_logic> rst;
    sc_fifo_out<sc_uint<8> > o_r;
    sc_fifo_out<sc_uint<8> > o_g;
    sc_fifo_out<sc_uint<8> > o_b;
    sc_fifo_in<sc_uint<8> > i_r;
    sc_fifo_in<sc_uint<8> > i_g;
    sc_fifo_in<sc_uint<8> > i_b;
    sc_fifo_out<sc_int<32> > o_x;
    sc_fifo_out<sc_int<32> > o_y;
    sc_fifo_out<sc_int<32> > o_i;
    sc_fifo_out<sc_int<32> > o_j;
    sc_fifo_in<sc_int<32> > i_height;
    sc_fifo_in<sc_int<32> > i_width;

    SC_HAS_PROCESS(Filter);
    Filter(sc_module_name n);
    ~Filter() = default;
   
};