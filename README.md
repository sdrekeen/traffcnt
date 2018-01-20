# traffcnt
This program is for traffic counting and is based on the OpenCV project (mainly using background-substraction and find-contours in OpenCV). The idea for object tracking algorithm I found in the coding from Chris D. see https://github.com/MicrocontrollersAndMore/OpenCV_3_Car_Counting_Cpp

To build and run this you must install the OpenCV-libraries version 3.4 or later, here a guide for Linux: https://docs.opencv.org/trunk/d7/d9f/tutorial_linux_install.html 


Changes: 
- suitable now for ordinary g++ on linux, changing some syntax
- detects direction and has distinct counters per direction 
- added a simple approach to build different classes of moving objects just by object's-size
- german explainations in source-code

At the beginning of the code a picture legend.png is loaded, which give a beautified background for the top-bar with the counters. It is not included here, as there is no public license for this. So be creative and find or draw your own ... :->

Compile hint: Let's say <OPENCV_PATH> is the path, where opencv files are placed then compile it by 

`g++ main.cpp -o traffcnt -I<OPENCV_PATH>/include -L<OPENCV_PATH>/lib -lopencv_highgui -lopencv_videoio -lopencv_imgcodecs -lopencv_video -lopencv_imgproc -lopencv_core`

For instance <OPENCV_PATH> is /usr/local/Cellar/opencv/3.4.0 then:

`g++ main.cpp -o traffcnt -I/usr/local/Cellar/opencv/3.4.0/include -L/usr/local/Cellar/opencv/3.4.0/lib -lopencv_highgui -lopencv_videoio -lopencv_imgcodecs -lopencv_video -lopencv_imgproc -lopencv_core`
