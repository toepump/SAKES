all: encRTtest2 BBB_Encoder

encRTtest2: encRTtest2.cpp
	g++ encRTtest2.cpp -o encRTtest2 `pkg-config glib-2.0 --cflags --libs` -lpthread

BBB_Encoder: BBB_Encoder.cpp
	g++ BBB_Encoder.cpp -o BBB_Encoder

clean:
	rm -f encRTtest2 BBB_Encoder outputEncoder.csv probeCheck.data
