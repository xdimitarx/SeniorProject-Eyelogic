OBJS = main.o EyeLogic.o
CC = g++
LIBS= -L /usr/local/lib/ 
INC = -I /usr/local/include/ 
DEBUG = -g
CFLAGS = -Wall -w -std=c++11 
LFLAGS = -lopencv_calib3d -lopencv_core -lopencv_features2d -lopencv_flann -lopencv_highgui -lopencv_imgcodecs -lopencv_imgproc -lopencv_ml -lopencv_objdetect -lopencv_photo -lopencv_shape -lopencv_stitching -lopencv_superres -lopencv_video -lopencv_videoio -lopencv_videostab -lboost_system -lboost_system -lboost_filesystem

main.out: $(OBJS)
	$(CC) $(LIBS) $(CFLAGS)  -o $@ $(OBJS) $(LFLAGS)

main.o: main.cpp EyeLogic.hpp
	$(CC) $(INC) $(CFLAGS) -c main.cpp

EyeLogic.o: EyeLogic.cpp EyeLogic.hpp
	$(CC) $(INC) $(CFLAGS) -c EyeLogic.cpp 

clean:
	rm *.o *.out