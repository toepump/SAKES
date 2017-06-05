all: encRTtest2 BBB_Encoder

encRTtest2: encRTtest2.cpp
	g++ encRTtest2.cpp -o encRTtest2 `pkg-config glib-2.0 --cflags --libs` -lpthread

BBB_Encoder: BBB_Encoder.cpp
	g++ BBB_Encoder.cpp -o BBB_Encoder

Test_Motor: Test_Motor.cpp
	g++ Test_Motor.cpp -o Test_Motor `pkg-config glib-2.0 --cflags --libs` -lpthread

clean:
	rm -f encRTtest2 BBB_Encoder outputEncoder.csv probeCheck.data
