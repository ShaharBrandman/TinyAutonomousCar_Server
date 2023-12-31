ML_CXX = g++
ML_CXXFLAGS = -std=c++14 -I /usr/include/opencv4 -I /usr/include -I $(PWD)/include
ML_LIBS = -ldlib -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_imgcodecs -lopencv_ml -lboost_system -lboost_filesystem

SRV_CXX = gcc
SRV_LIBS = -lssl -lcrypto

HTTP_SRV_OUT = index.o
HTTP_SRV_IN = index.c

ML_OUT = ml.o
ML_IN = ml.cpp

TRASH = ml.o index.o test.o OutputData object_model.xml

all: $(HTTP_SRV_OUT) $(ML_OUT)

$(ML_OUT): $(ML_IN)
	$(ML_CXX) $(ML_CXXFLAGS) -o $(ML_OUT) $(ML_IN) $(ML_LIBS)
	$(ML_CXX) $(ML_CXXFLAGS) -o test.o test.cpp $(ML_LIBS)

$(HTTP_SRV_OUT): $(HTTP_SRV_IN)
	$(SRV_CXX) -o $(HTTP_SRV_OUT) $(HTTP_SRV_IN) $(SRV_LIBS)

clean:
	rm -rf $(OUT_TARGET) $(TRASH)