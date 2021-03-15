# EE6470 Homework 1 Gaussian Blur
## Problems andd Solutions
### Fixed point datatype declaration
Q: Failed to declare **sc_fixed<32, 24> filter** in header file. Below is error message.
```
error: 'sc_fixed' does not name a type
```
A: add fixed point define to **main.cpp** at first line, not other files or other places.
```
#define SC_INCLUDE_FX // enable the fixed-point data types
```
### Module conncection and variable
Q: Faild to access port value directly
A: Declare local variabe(in class or in process) to connect port like below.
```
sc_fifo_in<sc_int<32> > i_width; //in headr file declaration
.....
sc_int<32> width = i_width.read(); //in process defination
```

### Deadlock when read in data
Q: The process stuck at the reading point, evey process are waiting for other writing.
A: add **wait()** before read / write function to prevenct deadlock.
```
wait();
R = i_R.read();
```

## Implementation details 


### Part 1: C++ Algorithm code
I used the read/write function in template, and I only modified sobel function in this problem
#### filter parameters
I copy them from [here](https://lodev.org/cgtutor/filtering.html#Gaussian_Blur_), which is TA provied.

```
double filter[filterHeight][filterWidth] = {
  {0.077847, 0.123317, 0.077847},
  {0.123317, 0.195346, 0.123317},
  {0.077847, 0.123317, 0.077847}
};

```
#### convolution function
This function do the convolution by access **image_s**'s data then write out to **image_t** directly. And since the image written in **write_bmp** function will shift the image, so I set offset 28 to prevent it. 
```
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
```
### Part 2: SystemC code
#### Block diagram
My testbench is composed of 2 C++ IO funcion and 3 systemC module just like below.
I designed **Filter** to control the IO data and contro signal with **Input** and **Output**, in Verilog view, I expect FSM will be generated in **Filter** after HLS.

![](https://i.imgur.com/UBivFDU.jpg)


#### main function
For the above deadlock problem I faced, I set up a clock signal for the whole system. Also connected **testbench** and **filter** in this funcion.

```
sc_clock clk("clk", CLOCK_PERIOD, SC_NS);
...
sc_fifo<sc_int<32> > y;
sc_fifo<sc_int<32> > i;
sc_fifo<sc_int<32> > j;
...
tb.rst(rst);
tb.clk(clk);
tb.o_r(source_r);
tb.o_g(source_g);
.......
filt.o_r(result_r);
filt.o_g(result_g);
```

Then I set up simulation flow with 2 C++ read/write bitmap function.
```

cout << "reading bitmap..." << '\t' << sc_time_stamp()<< endl;
tb.read_bmp();

cout << "starting simulation..." << '\t' << sc_time_stamp()<< endl;

sc_start();

cout << "finish simulation" << '\t' << sc_time_stamp()<< endl;
tb.write_bmp();
cout << "writing bitmap..." << '\t' << sc_time_stamp()<< endl;
```
#### testbench
The function read in and write out bmp file is just like the function that template provied. Below is the IO module that IO data with filter.
for below 2 modules, they all waiting the control signal (i, j ,x etc.) then do the function(send data or write out data to bmp output function). Finally, filter will send a **finish** signal when writing to let  tb know that wether the whold procedure is done or not.
```
void Testbench::input_data(){
  
  int offset = 28;
  sc_int<32> i, j, x, y;
  o_width.write(width);
  o_height.write(height);
  while(1){
    i = i_i.read();
    j = i_j.read();
    x = i_x.read();
    y = i_y.read();
    o_r.write(*(image_s + byte_per_pixel * (width * (y+i) + x + j + offset) + 2));
    o_g.write(*(image_s + byte_per_pixel * (width * (y+i) + x + j + offset) + 1));
    o_b.write(*(image_s + byte_per_pixel * (width * (y+i) + x + j + offset) + 0));
  }

}

}
void Testbench::output_data(){
  
  sc_int<32> x, y;
  sc_uint<8> R, G, B;
  sc_logic finish;
  
  while(1){
    
    //cout << "reading i j x y r g b in output" << endl;
    x = i_x.read();
    y = i_y.read();
    R = i_r.read();
    G = i_g.read();
    B = i_b.read();
    finish = i_finish.read();
    
    //cout << "finish reading in output" << endl;
    
    *(image_t + byte_per_pixel * (width * y + x) + 2) = R;
    *(image_t + byte_per_pixel * (width * y + x) + 1) = G;
    *(image_t + byte_per_pixel * (width * y + x) + 0) = B;
    if(finish == true)
      sc_stop();
  }
}
```

#### filter
Here is the main for loop that computing the result then write out, it just like C++ version above, but it read and write data rather then access the class variable directly, and there is the important thing that I must add clock triiger **wait()** to prevent deadlock before r/w data.And filter will send **finish** to tb to let it know procedure is finish or not.
```
for (y = 0; y != height; ++y) {
        for (x = 0; x != width; ++x) {
            //cout << "reset RGB :" << x << " " << y << endl;
            R = G = B = 0;
            for (i=-1 ; i<filterHeight-1 ; ++i) {
                for (j=-1 ; j<filterWidth-1 ; ++j) {
                    if(0<=y+i && y+i<height && 0<=x+j && x+j<width) {
                        wait();

                        o_i.write(i);
                        o_j.write(j);
                        o_x.write(x);
                        o_y.write(y);

    
                        source_r = i_r.read();
                        source_g = i_g.read();
                        source_b = i_b.read();
                        
                        R += (sc_fixed<32, 24>)source_r * filter[i+1][j+1];
                        G += (sc_fixed<32, 24>)source_g * filter[i+1][j+1];
                        B += (sc_fixed<32, 24>)source_b * filter[i+1][j+1];
                    }
                }
            }
            
            o_x.write(x);
            o_y.write(y);
            o_r.write((sc_uint<8>)R);
            o_g.write((sc_uint<8>)G);
            o_b.write((sc_uint<8>)B);
            o_finish.write((sc_logic)(y == height-1 && x == width-1));
        
        }
    }
```
## Additional features
### wait() usage
Professor said that we don't need to add **wait()** into code, but for the deadlock reason I mentioned above, I add some **wait()** into my code, I expect that will be the clock triggered brhavior if I do the high level synthesis to generate RTL code.
## Experimental results
### wait() position
By the **cout** function I found that if without **wait()** before every accumulation reading in convolution process, it will cause the program stuck at the position before reading. I have tried adding **wait()** at **Input**, **Output**, and **Filter**, but it semm only adding **wait()** at the position shown below make effect.
```
    for (i=-1 ; i<filterHeight-1 ; ++i) {
        for (j=-1 ; j<filterWidth-1 ; ++j) {
            if(0<=y+i && y+i<height && 0<=x+j && x+j<width) {
                wait();
                cout << "x: " << x << " y: " << y << " writing i j x y in filter" << endl;
                o_i.write(i);
                o_j.write(j);
                o_x.write(x);
                o_y.write(y);
                cout << "x: " << x << " y: " << y << " reading R G B in filter" << endl;
                source_r = i_r.read();
                source_g = i_g.read();
                source_b = i_b.read();
                cout << "finish reading in filter" << endl;
                R += (sc_fixed<32, 24>)source_r * filter[i+1][j+1];
                G += (sc_fixed<32, 24>)source_g * filter[i+1][j+1];
                B += (sc_fixed<32, 24>)source_b * filter[i+1][j+1];
            }
        }
    }
```
### Filter parameters
In the reference pages show that we can choose integer parameters or floating point paratmeters, where floating point version has better effect just like below figure shown.
#### Original
![](https://i.imgur.com/lE3VTdY.png)
#### Integer parameters
![](https://i.imgur.com/ydj8QuI.png)

#### Floating point parameters
![](https://i.imgur.com/BWWftyn.png)

As the result shown, floating point parameters can make output more smoothly, but it is darker than integer version.
## Discussions and conclusions
For this homework, I refernece lab02 as the template code to design my module. At first, I want to put the control block in testbench, that determine which pixel should the **filter** compute, but in my experience of desing neural network module, I always set FSM and control signal generator in the convolution block, so I tried this way in this homework.
In my expectation, **filter** should has a FSM int the synthesised RTL code, also this FSM control the IO by sending out x, y ,i ,and j, which determine the input pixel reading address and output pixel wrting address.