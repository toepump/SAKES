all: main.o initCounter.o taskThread.o interruptThread.o counter.o probingThread.o printOutData.o
	g++ -lpthread -o -std=c++11 `pkg-config glib-2.0 --cflags --libs` main main.o initCounter.o taskThread.o interruptThread.o counter.o probingThread.o printOutData.o

main.o: main.cpp common.h initCounter.h taskThread.h interruptThread.h
	g++ -c `pkg-config glib-2.0 --cflags --libs`  -std=c++11 -lpthread main.cpp

initCounter.o: initCounter.cpp common.h
	g++ -c `pkg-config glib-2.0 --cflags --libs` -std=c++11 -lpthread initCounter.cpp

taskThread.o: taskThread.cpp taskThread.h common.h probingThread.h
	g++ -c `pkg-config glib-2.0 --cflags --libs` -std=c++11 -lpthread taskThread.cpp

interruptThread.o: interruptThread.cpp interruptThread.h common.h counter.h
	g++ -c `pkg-config glib-2.0 --cflags --libs` -std=c++11 -lpthread interruptThread.cpp

counter.o: counter.h common.h printOutData.h
	g++ -c `pkg-config glib-2.0 --cflags --libs` -std=c++11 -lpthread counter.cpp

probingThread.o: probingThread.h common.h
	g++ -c `pkg-config glib-2.0 --cflags --libs` -std=c++11 -lpthread probingThread.cpp

printOutData.o: printOutData.h common.h
	g++ -c `pkg-config glib-2.0 --cflags --libs` -std=c++11 -lpthread printOutData.cpp

clean:
	rm -f main.o initCounter.o taskThread.o interruptThread.o counter.o probingThread.o printOutData.o
