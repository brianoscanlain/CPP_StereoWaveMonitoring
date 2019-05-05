//=============================================================================
/*
* Set grabmode!!!
*=================
*
*->BUFFER_FRAMES::
*Images accumulate in the user buffer, and the oldest image is
* grabbed for handling before being discarded.This member can be
* used to guarantee that each image is seen.However, image processing
* time must not exceed transmission time from the camera to the
* buffer.Grabbing blocks if the camera has not finished transmitting
* the next available image.The buffer size is controlled by the
* numBuffers parameter in the FC2Config struct.Note that this mode is
* the equivalent of flycaptureLockNext in earlier versions of the
* FlyCapture SDK.
*
*->DROP_FRAMES::
* Grabs the newest image in the user buffer each time the
* RetrieveBuffer() function is called. Older images are dropped
* instead of accumulating in the user buffer. Grabbing blocks if the
* camera has not finished transmitting the next available image. If
* the camera is transmitting images faster than the application can
* grab them, images may be dropped and only the most recent image
* is stored for grabbing. Note that this mode is the equivalent of
* flycaptureLockLatest in earlier versions of the FlyCapture SDK.



*/
//=============================================================================
//=============================================================================
// $Id: GigEGrabEx.cpp 316528 2017-02-22 00:03:53Z alin $
//=============================================================================

// load libraries::
//=============================================================================
#include "stdafx.h"
#include "FlyCapture2.h"
#include <iomanip>
#include <iostream>
#include <sstream>
#include <Windows.h>
//===========================


//Define Name spaces: 
//=============================================================================
using namespace FlyCapture2;
using namespace std;
//===========================
//Define project variables and constants::  (taken from multi Camera Write)
//=============================================================================
const string csDestinationDirectory = "F:\\EKOFISK_Images\\"; //Root image directory to save acquired images
int JpegQuality = 100;
GrabMode grabmode = BUFFER_FRAMES;   //set to DROP_FRAMES or BUFFER_FRAMES (see above)
int JUMBOsize = 8000; //packet size, in bytes    (9000 results in packet loss for some reason)
int PacketDelayCam1 = 35, PacketDelayCam2=35; //packet delay, in milliseconds
int grabTime_out = 130;  //the maximum length of time waited to retrieve image from buffer
int numberBuffers = 3;  //number of images kept on the buffer
bool Mode_Binning = true; //set to true to bin image pixels by two (reducing Image dimensions by 1/2)
//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
//		                        Sub functions
//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
//&
//&
//=============================================================================
void PrintBuildInfo()   //Prints flycapture library version information
{
	FC2Version fc2Version;
	Utilities::GetLibraryVersion(&fc2Version);

	ostringstream version;
	version << "FlyCapture2 library version: " << fc2Version.major << "."
		<< fc2Version.minor << "." << fc2Version.type << "."
		<< fc2Version.build;
	cout << version.str() << endl;

	ostringstream timeStamp;
	timeStamp << "Application build date: " << __DATE__ << " " << __TIME__;
	cout << timeStamp.str() << endl << endl;
}//===========================


 //===============================
// Subfunction to read the system 
//timestamp (yyyymmddHHMMSSFFF), 
//using windows.h library           (I use this to keep track of time for logging etc.)
//================================
void sysTimeStamp(char* dteSYS)
// To note: we have defined dteSYS so there is no need to redefine it!! 
{
	SYSTEMTIME stime;  //structure to store system time (in usual time format)
	FILETIME ltime;    //structure to store local time (local time in 64 bits)
	FILETIME ftTimeStamp;    //structure to store 
							 //
	GetSystemTimeAsFileTime(&ftTimeStamp); //get current system time:
	FileTimeToLocalFileTime(&ftTimeStamp, &ltime);  // F(in,out) ..convert time to local format
	FileTimeToSystemTime(&ltime, &stime); //F(in,out) ..convert the localized time to system time	
										  //Here we print the timestamp values to TimeStamp using sprintf. Here, %d prints a signed decimal integer
	sprintf(dteSYS, "%04d%02d%02d%02d%02d%02d%03d", stime.wYear,
		stime.wMonth, stime.wDay, stime.wHour, stime.wMinute,
		stime.wSecond, stime.wMilliseconds);
}//================================

void dayOfYear(string dte, int * DOY, double * DOYfrac)
{ //This function takes in the timestamp, and finds out what day of the year it is.
  //requires timestamp of format 'yyyymmdd....'
	int monthAdder[12] = { 0,31,59,90,120,151,181,212,243,273,304,334 };
	int monthAdderLeap[12] = { 0,31,60,91,121,152,182,213,244,274,305,335 };
	//extract sub string values:
	string yyyy = dte.substr(0, 4);
	string mm = dte.substr(4, 2);
	string dd = dte.substr(6, 2);
	string HH = dte.substr(8, 2);
	string MM = dte.substr(10, 2);
	string SS = dte.substr(12, 2);
	string MS = dte.substr(14, 3);
	//convert substrings to integer values:
	int iYear = stoi(yyyy);
	int iMon = stoi(mm);
	int iDay = stoi(dd);
	int iH = stoi(HH);
	int iM = stoi(MM);
	int iS = stoi(SS);
	int iMS = stoi(MS);
	bool leap;
	//determine if leap year?
	if ((iYear % 4 == 0 && iYear % 100 != 0) || iYear % 400 == 0)
		leap = true;
	else
		leap = false;

	//calc DOY
	if (leap == false)
	{
		*DOY = iDay + monthAdder[iMon - 1];
		*DOYfrac = (double)*DOY + (double)iH / 24 + (double)iM / 1440 + (double)iS / 86400 + (double)iMS / 86400000;
	}
	else
	{
		*DOY = iDay + monthAdderLeap[iMon - 1];
		*DOYfrac = (double)*DOY + (double)iH / 24 + (double)iM / 1440 + (double)iS / 86400 + (double)iMS / 86400000;
	}
	
}//===========================

 //=================================
 //  Find the sun rise and set times
 //=================================
void sunRiseSet(int DOY, double (&Daylight)[2]){
	//calc Daylight (sunrise and sunset)
	double sunrise[366] = { 1.34722, 2.34722, 3.34653, 4.34653, 5.34583, 6.34583, 7.34514, 8.34514, 
		9.34444, 10.34375, 11.34306, 12.34236, 13.34167, 14.34097, 15.34028, 16.33958, 17.33819, 
		18.33750, 19.33681, 20.33542, 21.33472, 22.33333, 23.33264, 24.33125, 25.33056, 26.32917, 
		27.32778, 28.32708, 29.32569, 30.32431, 31.32292, 32.32153, 33.32014, 34.31875, 35.31736, 
		36.31597, 37.31458, 38.31319, 39.31181, 40.31042, 41.30833, 42.30694, 43.30556, 44.30417, 
		45.30208, 46.30069, 47.29861, 48.29722, 49.29583, 50.29375, 51.29236, 52.29028, 53.28889, 
		54.28681, 55.28542, 56.28333, 57.28194, 58.27986, 59.27847, 60.27639, 61.27500, 62.27292, 
		63.27083, 64.26944, 65.26736, 66.26597, 67.26389, 68.26181, 69.26042, 70.25833, 71.25625, 
		72.25486, 73.25278, 74.25139, 75.24931, 76.24722, 77.24583, 78.24375, 79.24167, 80.24028, 
		81.23819, 82.23611, 83.23472, 84.23264, 85.23056, 86.22917, 87.22708, 88.22500, 89.22361, 
		90.22153, 91.21944, 92.21806, 93.21597, 94.21389, 95.21250, 96.21042, 97.20833, 98.20694, 
		99.20486, 100.20278, 101.20139, 102.19931, 103.19722, 104.19583, 105.19375, 106.19236, 
		107.19028, 108.18819, 109.18681, 110.18472, 111.18333, 112.18125, 113.17986, 114.17847, 
		115.17639, 116.17500, 117.17292, 118.17153, 119.17014, 120.16806, 121.16667, 122.16528, 
		123.16389, 124.16181, 125.16042, 126.15903, 127.15764, 128.15625, 129.15486, 130.15347, 
		131.15208, 132.15069, 133.14931, 134.14792, 135.14653, 136.14514, 137.14375, 138.14236, 
		139.14097, 140.14028, 141.13889, 142.13750, 143.13681, 144.13542, 145.13472, 146.13333, 
		147.13264, 148.13125, 149.13056, 150.12986, 151.12917, 152.12778, 153.12708, 154.12639, 
		155.12569, 156.12500, 157.12500, 158.12431, 159.12361, 160.12292, 161.12292, 162.12222, 
		163.12222, 164.12153, 165.12153, 166.12153, 167.12153, 168.12153, 169.12153, 170.12153, 
		171.12153, 172.12153, 173.12153, 174.12153, 175.12222, 176.12222, 177.12292, 178.12292, 
		179.12361, 180.12431, 181.12431, 182.12500, 183.12569, 184.12639, 185.12708, 186.12778, 
		187.12847, 188.12917, 189.12986, 190.13125, 191.13194, 192.13264, 193.13403, 194.13472, 
		195.13611, 196.13681, 197.13819, 198.13889, 199.14028, 200.14167, 201.14236, 202.14375, 
		203.14514, 204.14583, 205.14722, 206.14861, 207.15000, 208.15139, 209.15208, 210.15347, 
		211.15486, 212.15625, 213.15764, 214.15903, 215.16042, 216.16181, 217.16319, 218.16458, 
		219.16597, 220.16667, 221.16806, 222.16944, 223.17083, 224.17222, 225.17361, 226.17500, 
		227.17639, 228.17778, 229.17917, 230.18056, 231.18194, 232.18333, 233.18472, 234.18611, 
		235.18750, 236.18889, 237.19028, 238.19167, 239.19306, 240.19444, 241.19583, 242.19722, 
		243.19861, 244.20000, 245.20208, 246.20347, 247.20486, 248.20625, 249.20764, 250.20903, 
		251.21042, 252.21181, 253.21319, 254.21458, 255.21597, 256.21736, 257.21875, 258.22014, 
		259.22153, 260.22292, 261.22431, 262.22569, 263.22708, 264.22847, 265.22986, 266.23125, 
		267.23264, 268.23403, 269.23542, 270.23681, 271.23819, 272.23958, 273.24097, 274.24236, 
		275.24375, 276.24514, 277.24653, 278.24792, 279.24931, 280.25069, 281.25208, 282.25347, 
		283.25486, 284.25625, 285.25764, 286.25903, 287.26042, 288.26181, 289.26319, 290.26528, 
		291.26667, 292.26806, 293.26944, 294.27083, 295.27222, 296.27361, 297.27569, 298.27708, 
		299.27847, 300.27986, 301.28125, 302.28264, 303.28472, 304.28611, 305.28750, 306.28889, 
		307.29028, 308.29236, 309.29375, 310.29514, 311.29653, 312.29792, 313.30000, 314.30139, 
		315.30278, 316.30417, 317.30556, 318.30694, 319.30903, 320.31042, 321.31181, 322.31319, 
		323.31458, 324.31597, 325.31736, 326.31875, 327.32014, 328.32153, 329.32292, 330.32431, 
		331.32569, 332.32639, 333.32778, 334.32917, 335.33056, 336.33125, 337.33264, 338.33333, 
		339.33472, 340.33542, 341.33681, 342.33750, 343.33819, 344.33958, 345.34028, 346.34097, 
		347.34167, 348.34236, 349.34306, 350.34375, 351.34444, 352.34514, 353.34514, 354.34583, 
		355.34653, 356.34653, 357.34722, 358.34722, 359.34722, 360.34722, 361.34792, 362.34792, 
		363.34792, 364.34792, 365.34722, 366.34722 };

	double sunset[366] = { 1.63819, 2.63889, 3.63958, 4.64097,	5.64167, 6.64306, 7.64375, 8.64514,
		9.64583, 10.64722, 11.64861, 12.65000, 13.65069, 14.65208, 15.65347, 16.65486, 17.65625,
		18.65764, 19.65903, 20.66042, 21.66181, 22.66319, 23.66458, 24.66597, 25.66736, 26.66875,
		27.67014, 28.67222, 29.67361, 30.67500, 31.67639, 32.67778, 33.67986, 34.68125,	35.68264,
		36.68403, 37.68542,	38.68750, 39.68889,	40.69028, 41.69236,	42.69375, 43.69514,	44.69653,
		45.69861, 46.70000,	47.70139, 48.70278,	49.70486, 50.70625,	51.70764, 52.70903,	53.71111,
		54.71250, 55.71389,	56.71528, 57.71736,	58.71875, 59.72014,	60.72153, 61.72292,	62.72500,
		63.72639, 64.72778,	65.72917, 66.73056,	67.73194, 68.73333,	69.73542, 70.73681,	71.73819,
		72.73958, 73.74097,	74.74236, 75.74375,	76.74514, 77.74653,	78.74792, 79.74931,	80.75069,
		81.75208, 82.75417,	83.75556, 84.75694,	85.75833, 86.75972,	87.76111, 88.76250,	89.76389,
		90.76528, 91.76667,	92.76806, 93.76944,	94.77083, 95.77292,	96.77431, 97.77569,	98.77708,
		99.77847, 100.77986, 101.78125,	102.78264, 103.78403, 104.78542, 105.78750,	106.78889,
		107.79028, 108.79167, 109.79306, 110.79444, 111.79583, 112.79722, 113.79861, 114.80069,
		115.80208, 116.80347, 117.80486, 118.80625, 119.80764, 120.80903, 121.81042, 122.81181,
		123.81319, 124.81458, 125.81597, 126.81736, 127.81875, 128.82014, 129.82153, 130.82292,
		131.82431, 132.82569, 133.82708, 134.82847, 135.82986, 136.83125, 137.83264, 138.83403,
		139.83472, 140.83611, 141.83750, 142.83889, 143.83958, 144.84097, 145.84236, 146.84306,
		147.84444, 148.84583, 149.84653, 150.84792,	151.84861, 152.84931, 153.85069, 154.85139,
		155.85208, 156.85347, 157.85417, 158.85486, 159.85556, 160.85625, 161.85694, 162.85764,
		163.85764, 164.85833, 165.85903, 166.85972, 167.85972, 168.86042, 169.86042, 170.86042,
		171.86111, 172.86111, 173.86111, 174.86111, 175.86111, 176.86111, 177.86111, 178.86111,
		179.86111, 180.86042, 181.86042, 182.85972,	183.85972, 184.85903, 185.85903, 186.85833,
		187.85764, 188.85694, 189.85625, 190.85556, 191.85486, 192.85417, 193.85347, 194.85278,
		195.85139, 196.85069, 197.85000, 198.84861, 199.84792, 200.84653, 201.84583,202.84444,
		203.84375, 204.84236,205.84097, 206.83958, 207.83889, 208.83750, 209.83611, 210.83472,
		211.83333, 212.83194, 213.83056, 214.82917, 215.82778, 216.82639, 217.82500, 218.82361,
		219.82153, 220.82014, 221.81875, 222.81736, 223.81528, 224.81389, 225.81250, 226.81042,
		227.80903, 228.80694, 229.80556, 230.80347, 231.80208, 232.80000, 233.79861, 234.79653,
		235.79514, 236.79306, 237.79167, 238.78958, 239.78819, 240.78611, 241.78403, 242.78264,
		243.78056, 244.77847, 245.77708, 246.77500, 247.77292, 248.77153, 249.76944, 250.76736,
		251.76597, 252.76389, 253.76181, 254.76042,	255.75833, 256.75625, 257.75486, 258.75278,
		259.75069, 260.74931, 261.74722, 262.74514,	263.74375, 264.74167, 265.73958, 266.73819,
		267.73611, 268.73403, 269.73264, 270.73056, 271.72847, 272.72708, 273.72500, 274.72292,
		275.72153, 276.71944, 277.71736, 278.71597, 279.71389, 280.71181, 281.71042, 282.70833,
		283.70625, 284.70486, 285.70278, 286.70069, 287.69931, 288.69722, 289.69583, 290.69375,
		291.69236, 292.69028, 293.68889, 294.68681, 295.68542, 296.68333, 297.68194, 298.68056,
		299.67847, 300.67708, 301.67569, 302.67361, 303.67222, 304.67083, 305.66944, 306.66736,
		307.66597, 308.66458, 309.66319, 310.66181, 311.66042, 312.65903, 313.65764, 314.65625,
		315.65486, 316.65347, 317.65208, 318.65139, 319.65000, 320.64861, 321.64722, 322.64653,
		323.64514, 324.64444, 325.64306, 326.64236, 327.64097, 328.64028, 329.63958, 330.63819,
		331.63750, 332.63681, 333.63611, 334.63542, 335.63472, 336.63403, 337.63333, 338.63264,
		339.63264, 340.63194, 341.63125, 342.63125, 343.63056, 344.63056, 345.63056, 346.63056,
		347.62986, 348.62986, 349.62986, 350.62986, 351.63056, 352.63056, 353.63056, 354.63056,
		355.63125, 356.63125, 357.63194, 358.63264, 359.63264, 360.63333, 361.63403, 362.63472,
		363.63542, 364.63611, 365.63681, 366.63681 };

	Daylight[0] = sunrise[DOY];
	Daylight[1] = sunset[DOY];
	}//===========================


//===========================
// Print Camera Info                    (Code taken straight from Flycapture SRC examples)
//===========================
void PrintCameraInfo(CameraInfo *pCamInfo)  //prints GigE camera information
{
	ostringstream macAddress;
	macAddress << hex << setw(2) << setfill('0')
		<< (unsigned int)pCamInfo->macAddress.octets[0] << ":" << hex
		<< setw(2) << setfill('0')
		<< (unsigned int)pCamInfo->macAddress.octets[1] << ":" << hex
		<< setw(2) << setfill('0')
		<< (unsigned int)pCamInfo->macAddress.octets[2] << ":" << hex
		<< setw(2) << setfill('0')
		<< (unsigned int)pCamInfo->macAddress.octets[3] << ":" << hex
		<< setw(2) << setfill('0')
		<< (unsigned int)pCamInfo->macAddress.octets[4] << ":" << hex
		<< setw(2) << setfill('0')
		<< (unsigned int)pCamInfo->macAddress.octets[5];
	//========
	ostringstream ipAddress;
	ipAddress << (unsigned int)pCamInfo->ipAddress.octets[0] << "."
		<< (unsigned int)pCamInfo->ipAddress.octets[1] << "."
		<< (unsigned int)pCamInfo->ipAddress.octets[2] << "."
		<< (unsigned int)pCamInfo->ipAddress.octets[3];
	//========
	ostringstream subnetMask;
	subnetMask << (unsigned int)pCamInfo->subnetMask.octets[0] << "."
		<< (unsigned int)pCamInfo->subnetMask.octets[1] << "."
		<< (unsigned int)pCamInfo->subnetMask.octets[2] << "."
		<< (unsigned int)pCamInfo->subnetMask.octets[3];
	//========
	ostringstream defaultGateway;
	defaultGateway << (unsigned int)pCamInfo->defaultGateway.octets[0] << "."
		<< (unsigned int)pCamInfo->defaultGateway.octets[1] << "."
		<< (unsigned int)pCamInfo->defaultGateway.octets[2] << "."
		<< (unsigned int)pCamInfo->defaultGateway.octets[3];
	//========
	cout << endl;
	cout << "*** CAMERA INFORMATION ***" << endl;
	cout << "Serial number - " << pCamInfo->serialNumber << endl;
	cout << "Camera model - " << pCamInfo->modelName << endl;
	cout << "Camera vendor - " << pCamInfo->vendorName << endl;
	cout << "Sensor - " << pCamInfo->sensorInfo << endl;
	cout << "Resolution - " << pCamInfo->sensorResolution << endl;
	cout << "Firmware version - " << pCamInfo->firmwareVersion << endl;
	cout << "Firmware build time - " << pCamInfo->firmwareBuildTime << endl;
	cout << "GigE version - " << pCamInfo->gigEMajorVersion << "."
		<< pCamInfo->gigEMinorVersion << endl;
	cout << "User defined name - " << pCamInfo->userDefinedName << endl;
	cout << "XML URL 1 - " << pCamInfo->xmlURL1 << endl;
	cout << "XML URL 2 - " << pCamInfo->xmlURL2 << endl;
	cout << "MAC address - " << macAddress.str() << endl;
	cout << "IP address - " << ipAddress.str() << endl;
	cout << "Subnet mask - " << subnetMask.str() << endl;
	cout << "Default gateway - " << defaultGateway.str() << endl << endl;
}//===========================

//===========================
// Print Stream Channel info (Code taken straight from Flycapture SRC examples)
//===========================
void PrintStreamChannelInfo(GigEStreamChannel *pStreamChannel) //print channel info
{
	// char ipAddress[32];
	ostringstream ipAddress;
	ipAddress << (unsigned int)pStreamChannel->destinationIpAddress.octets[0]
		<< "."
		<< (unsigned int)pStreamChannel->destinationIpAddress.octets[1]
		<< "."
		<< (unsigned int)pStreamChannel->destinationIpAddress.octets[2]
		<< "."
		<< (unsigned int)pStreamChannel->destinationIpAddress.octets[3];

	cout << "Network interface - " << pStreamChannel->networkInterfaceIndex
		<< endl;
	cout << "Host Port - " << pStreamChannel->hostPort << endl;
	cout << "Do not fragment bit - "
		<< (pStreamChannel->doNotFragment ? "Enabled" : "Disabled") << endl;
	cout << "Packet size - " << pStreamChannel->packetSize << endl;
	cout << "Inter packet delay - " << pStreamChannel->interPacketDelay << endl;
	cout << "Destination IP address - " << ipAddress.str() << endl;
	cout << "Source port (on camera) - " << pStreamChannel->sourcePort << endl
		<< endl;
}//===========================

//===========================
void PrintError(Error error) { error.PrintErrorTrace(); } //(Code taken straight from Flycapture SRC examples)


int TestWritetoDirectory(string hh, string mmdd)
{	//=======================================
	//Test for creating files and directories 
	//=======================================
	// Since this application saves images in the current folder
	// we must ensure that we have permission to write to this folder.
	// If we do not have permission, we will make the code exit right away (to avoid confusion).
	cout << "\n%%%%%%%%%%%%%%%%%%%%%%%%%\nAttempting to create a sample file in the directory..." << endl;
	cout << csDestinationDirectory << mmdd << "\\" << hh << "\\test.txt" << endl;
	//choose appropriate directory:
	string Filepath;
	string FileDir1;
	string FileDir2;

	FileDir1 = csDestinationDirectory + mmdd;    //Here mmdd is the month-day in numbers, hh is hour (24-hour val)
	FileDir2 = FileDir1 + "\\" + hh;
	Filepath = FileDir2 + "\\test.txt";
	cout << Filepath << endl;

	const char * fileD1 = FileDir1.c_str();
	const char * fileD2 = FileDir2.c_str();
	const char * fileP = Filepath.c_str();
	const char * fileD = csDestinationDirectory.c_str();
	if (CreateDirectory(fileD, NULL))
	{		// Directory created
		cout << "directory:  " << fileD1 << " created successfully" << endl;
	}
	else if (ERROR_ALREADY_EXISTS == GetLastError())
	{		// Directory already exists
		cout << "directory:   " << fileD1 << " already exists, not recreating" << endl;
	}
	else
	{		// Failed for some other reason
		cout << "directory:   " << fileD1 << " failed to be created, for some reason \n ....investigate premissions" << endl;
	}

	CreateDirectory(fileD1, NULL);
	CreateDirectory(fileD2, NULL);
	FILE *tempFile = fopen(fileP, "w+");
	if (tempFile == NULL)
	{
		cout << "Failed to create file in current folder.  Please check "
			"permissions."
			<< endl;
		return -1;
	}
	fclose(tempFile);
	remove("test.txt");
	cout << "\n..File writing and removing test successful!!\n%%%%%%%%%%%%%%%%%%%%%%%%%\n" << endl;
	//==========================
	return 0;
}//=============================




//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
//		                        Main function
//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
//&
//&
 int main()
{
	PrintBuildInfo();
	Error error;

	//=============================================================================
	//print the timestamp
	//=============================================================================
	int n = 17; //number of characters in the timestamp
	char systime[18];
	sysTimeStamp(systime);
	string dte(systime);
	string hh;   // get the daily hour value ('hh' in 24h format)
	string mmdd; // get the month and day value ('mm' with range 01-12, and 'dd' with range 01-31')		 //
	hh = dte.substr(8, 2);
	mmdd = dte.substr(4, 4);
	TestWritetoDirectory(hh, mmdd);
	//=============================================================================


	//=============================================================================
	//     +++ Detect cameras +++
	//=============================================================================
	cout << "%%%%%%%%%%%%%%%%%%%%%%%%%\n..Detecting cameras...\n" << endl;
	PrintBuildInfo(); //prints FlyCap library version and App build date

	BusManager busMgr;
	CameraInfo camInfo[2];
	unsigned int numCam=2;
	error = busMgr.DiscoverGigECameras(camInfo,&numCam);
	if (error != PGRERROR_OK)
	{
		PrintError(error);
		return -1;
		cout << "Bus manager is having issues Discovering GigECameras" << endl;
	}
	cout << "Number of GigE cameras discovered: " << numCam << endl;

	

	if (numCam == 0)
	{
		cout << "No suitable GigE cameras found. Press Enter to exit..."
			<< endl;
		cin.ignore();
		return 0;
	}

	// Check to make sure enough cameras are connected
	if (numCam < 2)
	{
		cout << "Insufficient number of cameras..." << endl;
		cout << "Please connect at least two GigE cameras for script to run."
			<< endl;
		cout << "Press Enter to exit..." << endl;
		cin.ignore();
		return -1;
	}
	//=============================================================================

	
	
	


	//=============================================================================
	//Connect to cameras:
	//=============================================================================
	GigECamera *Cam = new GigECamera[numCam];
	unsigned int numStreamChannels = 0;
	for (unsigned int i = 0; i < numCam; i++)
	{
		PGRGuid guid;
		CameraInfo cNFO;
		error = busMgr.GetCameraFromIndex(i, &guid);
		if (error != PGRERROR_OK)
		{
			PrintError(error);
			return -1;
			cout << "failed to get camera from index" << endl;
		}

		// Connect to a camera
		error = Cam[i].Connect(&guid);
		if (error != PGRERROR_OK)
		{
			PrintError(error);
			delete[] Cam;
			cout << "Press Enter to exit." << endl;
			cin.ignore();
			return -1;
		}

		//retrieve camerainfo  
		error = Cam[i].GetCameraInfo(&cNFO);
		if (error != PGRERROR_OK)
		{
			PrintError(error);
			return -1;
		}
		PrintCameraInfo(&cNFO);
		
		//Checking the camera interface type::
		InterfaceType interfaceType;
		error = busMgr.GetInterfaceTypeFromGuid(&guid, &interfaceType);
		if (error != PGRERROR_OK)
		{
			PrintError(error);
			return -1;
		}
		//make sure the Camera is a GigE camera::
		if (interfaceType != INTERFACE_GIGE)
		{
			cout << "This is not a GigEcamera, exiting...." << endl;
			return 0;
		}

		//=================================================================
		//Setup the GigE stream options
		//=================================================================
		error = Cam[i].GetNumStreamChannels(&numStreamChannels);
		if (error != PGRERROR_OK)
		{
			PrintError(error);
			return -1;
		}

		for (unsigned int ii = 0; ii < numStreamChannels; ii++)
		{
			GigEStreamChannel streamChannel;
			error = Cam[i].GetGigEStreamChannelInfo(ii, &streamChannel);
			if (error != PGRERROR_OK)
			{
				PrintError(error);
				return -1;
			}
			//configure the settings:
			streamChannel.destinationIpAddress.octets[0] = 224;
			streamChannel.destinationIpAddress.octets[1] = 0;
			streamChannel.destinationIpAddress.octets[2] = 0;
			streamChannel.destinationIpAddress.octets[3] = 1;
			streamChannel.packetSize = JUMBOsize;  //I found out that 9000 doesn't work for my setup!!
			streamChannel.doNotFragment = true;   //Transfer the image in one packet (safe, as image size < 8000kb)
			// set the packet delays::
			if (i == 0)  
			{ //Cam 1
				streamChannel.interPacketDelay = PacketDelayCam1;
			}
			else if (i == 1)
			{ //Cam 2
				streamChannel.interPacketDelay = PacketDelayCam2;
			}
			//force the new settings:
			error = Cam[i].SetGigEStreamChannelInfo(ii, &streamChannel);
			if (error != PGRERROR_OK)
			{
				PrintError(error);
				return -1;
			}
			cout << "Printing stream channel information for channel " << ii << endl;
			PrintStreamChannelInfo(&streamChannel);
		}


		//=================================================================
		//Set appropriate  videomode::
		//=================================================================
		//try binning the horizontal and vertical pixels:
		unsigned int horizBinVal, vertBinVal;
		if (Mode_Binning == true)
		{
			horizBinVal = 2; 
			vertBinVal = 2;
			error = Cam[i].SetGigEImageBinningSettings(horizBinVal, vertBinVal);
		}
		else
		{
			error = Cam[i].GetGigEImageBinningSettings(&horizBinVal,&vertBinVal);
		}
		cout << "Querying GigE image setting information..." << endl;
		cout << "Current Image binning settings;" << endl;
		cout << "horizontal binning value = " << horizBinVal << endl;
		cout << "Vertical   binning value = " << vertBinVal << endl;

		GigEImageSettingsInfo imageSettingsInfo;
		error = Cam[i].GetGigEImageSettingsInfo(&imageSettingsInfo);
		if (error != PGRERROR_OK)
		{
			PrintError(error);
			return -1;
		}
        //Configure image settings:
		GigEImageSettings imageSettings;
		imageSettings.offsetX = 0;
		imageSettings.offsetY = 0;
		imageSettings.height = imageSettingsInfo.maxHeight;
		imageSettings.width = imageSettingsInfo.maxWidth;
		imageSettings.pixelFormat = PIXEL_FORMAT_MONO8;

		cout << "Setting GigE image settings..." << endl;

		error = Cam[i].SetGigEImageSettings(&imageSettings);
		if (error != PGRERROR_OK)
		{
			PrintError(error);
			return -1;
		}

		//=================================================================
		//Set trigger mode          (I use a Arduino Uno to provide a 35 ms 
		//                           voltage-high step signal at a fq = 5 Hz)
		//=================================================================
		//set trigger//
		Property trig;
		trig.type = TRIGGER_MODE;
		error = Cam[i].GetProperty(&trig);
		if (error != PGRERROR_OK)
		{
			PrintError(error);
			return -1;
		}
		trig.onOff = true;
		error = Cam[i].SetProperty(&trig);

		//=================================================================
		//set grab mode, resend and timeout settings  (Buffer settings)
		//=================================================================
		FC2Config conf;
		error = Cam[i].GetConfiguration(&conf);
		if (error != PGRERROR_OK)
		{
			PrintError(error);
			return -1;
		}
		// Set the grab timeout to defined value        (defined in preamble of this code)
		conf.grabTimeout = grabTime_out;
		conf.grabMode = grabmode;  //choose the latest or oldest image held in camera buffer (see preamble)
		conf.numBuffers = numberBuffers; //number of images to be stored in the buffer
		conf.highPerformanceRetrieveBuffer = true;
		// Set the camera configuration
		error = Cam[i].SetConfiguration(&conf);
		if (error != PGRERROR_OK)
		{
			PrintError(error);
			return -1;
		}

		//=================================================================
		//Set up the buffer communication settings
		//=================================================================
		GigEConfig Gconf;
		error = Cam[i].GetGigEConfig(&Gconf);
		if (error != PGRERROR_OK)
		{
			PrintError(error);
			return -1;
		}
		Gconf.enablePacketResend = true; //Turn on/off packet resend functionality default=false
		Gconf.registerTimeoutRetries = 2; //Number of retries to perform when a register read / write timeout is received by the library.The default value is 0.
		//Gconf.registerTimeout = 100; // Register read/write timeout value, in microseconds.
		error = Cam[i].SetGigEConfig(&Gconf);
		if (error != PGRERROR_OK)
		{
			PrintError(error);
			return -1;
		}
		else
		{
			cout << "GigE connection has been configured" << endl;
		}
		
		//=================================================================
		//Set up embedded image information
		//=================================================================
		EmbeddedImageInfo embedImgInfo;
		error = Cam[i].GetEmbeddedImageInfo(&embedImgInfo);
		if (error != PGRERROR_OK)
		{
			PrintError(error);
			return -1;
		}
		embedImgInfo.brightness.onOff = true;    //record the brightness value
		embedImgInfo.timestamp.onOff = true;     //record the camera's timestamp (time at shutter)
		embedImgInfo.frameCounter.onOff = true;   //I don't really need this at the moment, but it may be useful for post processing and/or statistics
		error = Cam[i].SetEmbeddedImageInfo(&embedImgInfo);
		if (error != PGRERROR_OK)
		{
			PrintError(error);
			return -1;
		}
		//=================================================================
		//Start capturing image
		//=================================================================
		error = Cam[i].StartCapture();
		if (error != PGRERROR_OK)
		{
			PrintError(error);
			return -1;
		}
	}
	
	
	
	
	//=============================================================================
    // Grab images::
	//=============================================================================
	Image rawIm;
	Image convIm;
	int numImages = 10;        // small number used here for debugging. I plan to acquire images continuously (when operational)
	CameraInfo camInfo1, camInfo2;
	string FileDir;
	Cam[0].GetCameraInfo(&camInfo1);  //retrieve the serial information for each camera   (I use this to identify images as either left or right camera
	Cam[1].GetCameraInfo(&camInfo2);  //retrieve the serial information for each camera    in my stereo layout)
	ostringstream fileName[2];
	//PNGOption pngOpt;
	//pngOpt.compressionLevel = 10;             
	//pngOpt.interlaced = false;
	//TIFFOption tiffOpt;
	//tiffOpt.NONE;
	JPEGOption jpgOpt;
	jpgOpt.quality = JpegQuality;

	//=========================
	// times to acquire images:
	//=========================
	double startstop[2];
	int DOY;
	double DOYfrac;
	dayOfYear(systime, &DOY, &DOYfrac);   //find day of year from timestamp. DOY is passed by pointer here.
	sunRiseSet(DOY, startstop);  //retrieve sunrise and sunset times.

	//Pause the system if it is still dark morning time:
	while (DOYfrac<startstop[0])
	{
		Sleep((double)(startstop[0]- DOYfrac)*86400000);  //get the program to pause for 
	}

	
	while (DOYfrac >= startstop[0] && DOYfrac < startstop[1])
	{
		//daylight hours, acquire images!
		for (int imageCnt = 0; imageCnt < numImages; imageCnt++)   //loop to acquire multiple images.
		{
			//=============================================================================
			// prepare the file path for each image    (I use system time to determine a suitable
			//											data path, and save it in a ostringstream.)
			//prepare the filename for each image		This is overwritten each time and should 
			//											change directory path every hour.)
			//=============================================================================
			//calculate common timestamp using system time:
			sysTimeStamp(systime);
			dte = systime;
			hh = dte.substr(8, 2);
			mmdd = dte.substr(4, 4);
			//directory:
			FileDir = csDestinationDirectory + mmdd + "\\" + hh;
			//filenames:
			fileName[0] << FileDir << "\\" << systime << "-" << camInfo1.serialNumber << ".jpg";  //FC API ref. pp287: PGM, PPM, BMP, JPEG, JPEG2000, TIFF, PNG, RAW
			fileName[1] << FileDir << "\\" << systime << "-" << camInfo2.serialNumber << ".jpg";
			fileName[0].seekp(0);  // fileName dumps it's content whenever fileName is redefined. This means that future 
			fileName[1].seekp(0);  //writing to 'fileName' is overwritten and not appended to the existing text.
			for (unsigned int i = 0; i < numCam; i++)
			{
				// Retrieve an image
				error = Cam[i].RetrieveBuffer(&rawIm);
				if (error != PGRERROR_OK)
				{
					cout << "Cam" << i << "  Img" << imageCnt << ":   " << dte << ":  Grabbed:: No" << endl;
				}
				else { cout << "Cam" << i << "  Img" << imageCnt << ":   " << dte << ":  Grabbed:: Yes" << endl; }
				/*// Convert raw image 1
				error = rawIm.Convert(PIXEL_FORMAT_MONO8, &convIm);
				if (error != PGRERROR_OK)
				{
					cout << "Cam" << i << ":   " << dte << ":  Converted:: No" << imageCnt << endl;
				}
				else { cout << "Cam" << i << ":   " << dte << ":  Converted:: Yes" << imageCnt << endl; }
				*///save image:
				error = rawIm.Save(fileName[i].str().c_str(), &jpgOpt);
				if (error != PGRERROR_OK)
				{
					cout << "Cam" << i << "  Img" << imageCnt << ":   " << dte << ":  Saved:: No" << endl;
				}
				else { cout << "Cam" << i << "  Img" << imageCnt << ":   " << dte << ":  Saved:: Yes" << endl; }
				TimeStamp timestamp = rawIm.GetTimeStamp();
				//cout << "Camera " << i << " - TimeStamp ["
					//<< timestamp.seconds << " " << int(timestamp.microSeconds/1000) << "..." << fileName[i].str() << endl;
			}

		}

	}

	//=============================================================================







	//======================================================
	//  Stop triggering and cpaturing, and disconnect cameras
	//======================================================
	for (unsigned int i = 0; i < numCam; i++)
	{
		// Turn trigger mode off.
		TriggerMode triggerMode;
		error = Cam[i].GetTriggerMode(&triggerMode);
		if (error != PGRERROR_OK)
		{
			PrintError(error);
			return -1;
		}
		triggerMode.onOff = false;
		error = Cam[i].SetTriggerMode(&triggerMode);
		if (error != PGRERROR_OK)
		{
			PrintError(error);
			return -1;
		}
		cout << endl;


		// Stop capturing images
		error = Cam[i].StopCapture();
		if (error != PGRERROR_OK)
		{
			PrintError(error);
			return -1;
		}

		// Disconnect the camera
		error = Cam[i].Disconnect();
		if (error != PGRERROR_OK)
		{
			PrintError(error);
			return -1;
		}
	}
	//======================================================
	cout << "Finished grabbing images" << endl;
	//cout << "Done! Press Enter to exit..." << endl;
	//cin.ignore();
	return 0;
	//======================================================
	return 0;
}


