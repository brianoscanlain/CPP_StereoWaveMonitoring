# CPP_StereoWaveMonitoring
C++ solutions for acquiring images from a custom stereo camera system (Pointgrey - FLIR blkfly GIGE)

masterGige:

masterGige is a C++ application built upon the point-grey SDK libraries which oversees the operation of
two point-grey blkfly 5MP cameras simultaneously. The script is designed to run on a server PC, which
is connected to the cameras using gigabit ethernet cables (CAT6). The script initialises and configures 
the cameras, and then manages the storing of the images on the server PC. The code is written in C++ 
due to the high-frequency and large-data-size throughput (25+ GB/hr). 

Some useful information I found when debugging; it is important to configure the exposure settings on the
cameras so that the images are taken and are returned with the same timestamp. The cameras are triggered 
using an arduino signal (this code is also available).

ImViewBS:

ImViewBS is a C++ file which searches a HDD for the most recent image files written, it then calls on openCV
and displays them. It is intended as a complementary debugging and visualisation tool to masterGige. One 
can run the function and observe the incoming RAW images in real time. The RAW images have no header and 
so this custom Image Viewing solution is required!

