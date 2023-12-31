CXX = g++
CXXFLAGS = -std=c++14 -I /usr/include/opencv4 -I /usr/include -g
LIBS = -lssl -ldlib -pthread -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_imgcodecs -lopencv_ml -lboost_system -lboost_filesystem

HTTP_SRV_OUT = index.o
HTTP_SRV_IN = index.c

ML_OUT = ml.o
ML_IN = ml.cpp

TRASH = ml.o index.o test.o OutputData object_model.xml

all: $(HTTP_SRV_OUT)

$(OUT_TARGET): $(IN_TARGET)
	$(CXX) $(CXXFLAGS) -o $(HTTP_SRV_OUT) $(HTTP_SRV_IN) $(LIBS)
	# $(CXX) $(CXXFLAGS) -o $(ML_OUT) $(ML_IN) $(LIBS)
	$(CXX) $(CXXFLAGS) -o test.o test.cpp $(LIBS)

clean:
	rm -rf $(OUT_TARGET) $(TRASH)