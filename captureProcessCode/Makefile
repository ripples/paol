OBJS = wbMain.o paolMat.o fileIO.o
OBJS2 = compMain.o paolMat.o fileIO.o
OBJS1 = cameraCapture.cpp
CXX = gcc
CXXFLAGS = -g -Wno-write-strings -g -O2 
LIBS = `pkg-config --libs opencv` -I/opt/local/include -L/opt/local/lib -lboost_thread-mt -lboost_program_options -lboost_filesystem -lboost_system

all: wbMain compMain camCap

wbMain : $(OBJS)
	$(CXX) $(CXXFLAGS) -o processWB $(OBJS) $(LIBS)
compMain : $(OBJS2)
	$(CXX) $(CXXFLAGS) -o processCOMP $(OBJS2) $(LIBS)
camCap : $(OBJS1)
	$(CXX) $(CXXFLAGS) -o camCap $(OBJS1) $(LIBS)

clean:
	rm camCap process* *.o *~


