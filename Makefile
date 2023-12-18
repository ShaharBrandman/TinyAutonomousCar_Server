CXX = g++
CXXFLAGS = -std=c++14 -I/usr/include
LIBS = -ldlib -lopencv_core -lopencv_highgui -lopencv_imgproc -llapack -lcblas -lm -lgif

OUT_TARGET = metric_learning_example.o
IN_TARGET = metric_learning_example.cpp

all: $(OUT_TARGET)

$(OUT_TARGET): $(IN_TARGET)
	$(CXX) $(CXXFLAGS) -o $(OUT_TARGET) $(IN_TARGET) $(LIBS)

clean:
	rm -rf $(OUT_TARGET) OutputData