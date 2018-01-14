# traffcnt
Traffic counting based on opencv (Background-substraction and findContours) and after the basic ideas for object tracking through the different frames in a reference-program by Chris D. see https://github.com/MicrocontrollersAndMore/OpenCV_3_Car_Counting_Cpp
To build and run the opencv-libraries must be installed, developed under version opencv 3.4 - download or build f.i. see: https://docs.opencv.org/trunk/d7/d9f/tutorial_linux_install.html 


Changes: 
- suitable now for ordinary g++ on linux, changing some syntax
- detects direction and has distinct counters per direction 
- added a simple approach to build different classes of moving objects just by object's-size
- german explainations in source-code

Info picture used for legend.png, which is loaded to have more beatufied counting panel on windows top is not included here on github, as there is no public license for this. So be creative and find or draw your own ... :->

Compile hint: Let's say <OPENCV_PATH> is the path, where opencv files are placed then compile it by 
g++ main.cpp -o traffcnt -I<OPENCV_PATH>/include -L<OPENCV_PATH>/lib -lopencv_highgui -lopencv_videoio -lopencv_imgcodecs -lopencv_video -lopencv_imgproc -lopencv_core
For instance <OPENCV_PATH> is /usr/local/Cellar/opencv/3.4.0 then:
g++ main.cpp -o traffcnt -I/usr/local/Cellar/opencv/3.4.0/include -L/usr/local/Cellar/opencv/3.4.0/lib -lopencv_highgui -lopencv_videoio -lopencv_imgcodecs -lopencv_video -lopencv_imgproc -lopencv_core
