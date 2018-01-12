# traffcnt
Traffic counting based on opencv (Background-substraction and findContours) and after the basic ideas for object tracking through the different frames in a reference-program by Chris D. see https://github.com/MicrocontrollersAndMore/OpenCV_3_Car_Counting_Cpp
To build and run the opencv-libraries must be installed, developed under version opencv 3.4 - download or build f.i. see: https://docs.opencv.org/trunk/d7/d9f/tutorial_linux_install.html 

Changes: 
- suitable now for ordinary g++ on linux, changing some syntax
- detects direction and has distinct counters per direction 
- added a simple approach to build different classes of moving objects just by object's-size
- german explainations in source-code

Info picture used for legend.png, which is loaded to have more beatufied counting panel on windows top is not included here on github, as there is no public license for this. So be creative and find or draw your own ... :->
