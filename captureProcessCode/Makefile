OBJS = wbMain.o paolMat.o fileIO.o
OBJS2 = compMain.o paolMat.o fileIO.o
OBJS1 = cameraCapture.cpp
OBJS3 = multiDeviceCapture.cpp
CXX = gcc
CXXFLAGS = -g -Wno-write-strings -g -O2 
LIBS = `pkg-config --libs opencv` -I/opt/local/include -L/opt/local/lib -lstdc++ -lm

all: wbMain compMain camCap multiCap

wbMain : $(OBJS)
	$(CXX) $(CXXFLAGS) -o processWB $(OBJS) $(LIBS)
compMain : $(OBJS2)
	$(CXX) $(CXXFLAGS) -o processCOMP $(OBJS2) $(LIBS)
camCap : $(OBJS1)
	$(CXX) $(CXXFLAGS) -o camCap $(OBJS1) $(LIBS)
multiCap : $(OBJS3)
	$(CXX) $(CXXFLAGS) -o multiCap $(OBJS3) $(LIBS)

clean:
	rm camCap process* *.o *~


