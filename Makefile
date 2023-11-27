CC = g++
LIBS = -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_objdetect -lopencv_videoio -lopencv_imgcodecs -lopencv_video -I /usr/include/opencv4

OUT_TARGET = index.o

IN_TARGET = index.cpp

all: $(OUT_TARGET)

$(OUT_TARGET): $(IN_TARGET)
	$(CC) -o $(OUT_TARGET) $(IN_TARGET) $(LIBS)

clean:
	rm -rf $(OUT_TARGET) OutputData