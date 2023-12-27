CXX = g++
CXXFLAGS = -std=c++14 -I /usr/include/opencv4 -I /usr/include -g
LIBS = -lssl -ldlib -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_imgcodecs -lopencv_ml -lboost_system -lboost_filesystem

OUT_TARGET = index.o
IN_TARGET = index.c

TRASH = test.o OutputData object_model.xml

all: $(OUT_TARGET)

$(OUT_TARGET): $(IN_TARGET)
	$(CXX) $(CXXFLAGS) -o $(OUT_TARGET) $(IN_TARGET) $(LIBS)
	$(CXX) $(CXXFLAGS) -o test.o test.cpp $(LIBS)

clean:
	rm -rf $(OUT_TARGET) $(TRASH)