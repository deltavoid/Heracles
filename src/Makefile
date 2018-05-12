

CXX = g++

CXX_FLAGS = -g -Wall -O2 -std=c++11 

CXX_INCLUDES = -I../lib/headers -I/usr/lib/llvm-3.7/include -I/usr/include/bcc

CXX_LIBS = -L../lib/so -lpqos -lsqlite3 -lbcc -lbpf -lpthread


TARGET = heracles

OBJECTS = main.o top_controller.o core_memory_controller.o \
		  tap.o info_puller.o helpers.o console.o\
		  db_driver.o cpu_driver.o memory_driver.o cache_driver.o \
		  network_controller.o network_monitor.o network_driver.o

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXX_FLAGS) -o ../bin/heracles $(OBJECTS) $(CXX_LIBS)
	@echo [INFO] Compilation accomplished.

main.o: main.cpp heracles.h
	$(CXX) $(CXX_FLAGS) $(CXX_INCLUDES) -o $@ -c $<

top_controller.o: top_controller.cpp top_controller.h
	$(CXX) $(CXX_FLAGS) $(CXX_INCLUDES) -o $@ -c $<

core_memory_controller.o: core_memory_controller.cpp core_memory_controller.h
	$(CXX) $(CXX_FLAGS) $(CXX_INCLUDES) -o $@ -c $<

info_puller.o: info_puller.cpp info_puller.h
	$(CXX) $(CXX_FLAGS) $(CXX_INCLUDES) -o $@ -c $<

tap.o: tap.cpp tap.h
	$(CXX) $(CXX_FLAGS) $(CXX_INCLUDES) -o $@ -c $<

console.o: console.cpp console.h
	$(CXX) $(CXX_FLAGS) $(CXX_INCLUDES) -o $@ -c $<

db_driver.o: db_driver.cpp db_driver.h
	$(CXX) $(CXX_FLAGS) $(CXX_INCLUDES) -o $@ -c $<

cpu_driver.o: cpu_driver.cpp cpu_driver.h
	$(CXX) $(CXX_FLAGS) $(CXX_INCLUDES) -o $@ -c $<

cache_driver.o: cache_driver.cpp cache_driver.h
	$(CXX) $(CXX_FLAGS) $(CXX_INCLUDES) -o $@ -c $<

memory_driver.o: memory_driver.cpp memory_driver.h
	$(CXX) $(CXX_FLAGS) $(CXX_INCLUDES) -o $@ -c $<

network_controller.o: network_controller.cpp network_controller.h network_monitor.h network_driver.h
	$(CXX) $(CXX_FLAGS) $(CXX_INCLUDES) -o $@ -c $<

network_monitor.o: network_monitor.cpp network_monitor.h
	$(CXX) $(CXX_FLAGS) $(CXX_INCLUDES) -o $@ -c $<

network_driver.o: network_driver.cpp network_driver.h
	$(CXX) $(CXX_FLAGS) $(CXX_INCLUDES) -o $@ -c $<

helpers.o: helpers.cpp helpers.h
	$(CXX) $(CXX_FLAGS) $(CXX_INCLUDES) -o $@ -c $<


.PHONY: clean
clean:
	@-rm *.o
	@-rm ../bin/heracles
	@echo [INFO] Clean accomplished.


