CC = g++
LIBS = -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_objdetect -lopencv_videoio -lopencv_imgcodecs -lopencv_video -lopencv_dnn -I /usr/include/opencv4

OUT_TARGET = dev.o

IN_TARGET = dev.cpp

all: $(OUT_TARGET)

$(OUT_TARGET): $(IN_TARGET)
	$(CC) -o $(OUT_TARGET) $(IN_TARGET) $(LIBS)

clean:
	rm -rf $(OUT_TARGET) OutputData
