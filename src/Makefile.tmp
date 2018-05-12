

CXX = g++

CXX_FLAGS = -g -Wall -O2 -std=c++11 

CXX_INCLUDES = -I/usr/lib/llvm-3.7/include -I/usr/include/bcc 

CXX_LIBS = -lbcc -lbpf -lpthread



TARGET = main

OBJECTS = main.o option.o network_controller.o network_driver.o network_monitor.o


all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXX_FLAGS) -o main $(OBJECTS) $(CXX_LIBS) 
	@echo [INFO] Compilation accomplished.

main.o: main.cpp network_controller.h option.h
	$(CXX) $(CXX_FLAGS) $(CXX_INCLUDES) -o $@ -c $< 

option.o: option.cpp option.h
	$(CXX) $(CXX_FLAGS) $(CXX_INCLUDES) -o $@ -c $< 

network_controller.o: network_controller.cpp network_controller.h network_driver.h network_monitor.h
	$(CXX) $(CXX_FLAGS) $(CXX_INCLUDES) -o $@ -c $<

network_driver.o: network_driver.cpp network_driver.h
	$(CXX) $(CXX_FLAGS) $(CXX_INCLUDES) -o $@ -c $<

network_monitor.o: network_monitor.cpp network_monitor.h
	$(CXX) $(CXX_FLAGS) $(CXX_INCLUDES) -o $@ -c $<


.PHONY: clean
clean:
	@-rm *.o
	@-rm main
	@echo [INFO] Clean accomplished.


