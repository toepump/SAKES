all: encRTtest2 Test_Motor encRTtest1 newStructureEncoder TestRT4 TestRT5 Test_PRU

newStructureEncoder: newStructureEncoder.cpp
	g++ newStructureEncoder.cpp -o newStructureEncoder `pkg-config glib-2.0 --cflags --libs` -lpthread

encRTtest1: encRTtest1.cpp
	g++ encRTtest1.cpp -o encRTtest1 `pkg-config glib-2.0 --cflags --libs` -lpthread

encRTtest2: encRTtest2.cpp
	g++ encRTtest2.cpp -o encRTtest2 `pkg-config glib-2.0 --cflags --libs` -lpthread

BBB_Encoder: BBB_Encoder.cpp
	g++ BBB_Encoder.cpp -o BBB_Encoder

Test_Motor: Test_Motor.cpp
	g++ Test_Motor.cpp -o Test_Motor `pkg-config glib-2.0 --cflags --libs` -lpthread

TestRT4: TestRT4.cpp
	g++ TestRT4.cpp -o TestRT4 `pkg-config glib-2.0 --cflags --libs` -lpthread

TestRT5: TestRT5.cpp
	g++ TestRT5.cpp -o TestRT5 `pkg-config glib-2.0 --cflags --libs` -lpthread

Test_PRU: Test_PRU.cpp
	g++ Test_PRU.cpp -o Test_PRU `pkg-config glib-2.0 --cflags --libs` -lpthread

clean:
	rm -f encRTtest2 BBB_Encoder outputEncoder.csv probeCheck.data
