#include "stdafx.h"
#include <iostream>
#include <string>
#include <afx.h>
#include <atlconv.h>
#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui/highgui.hpp>


using namespace std;
using namespace cv;

//=====================================================================
//  system timestamp (yyyymmddHHMMSSFFF), using windows.h library
//=====================================================================
void sysTimeStamp(char* mmddHH)
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
	sprintf(mmddHH, "%02d%02d\\%02d\\",stime.wMonth, stime.wDay, stime.wHour);
}//================================



//=====================================================================
// Subfunction to search for the latest RAW Image file!
//=====================================================================
string processName(string TargetDir)
{
	FILETIME bestDate = { 0, 0 };
	FILETIME curDate;
	string name;
	string out;
	CFileFind finder;
	//Here we use a macro to convert the string to a LPCTSTR
	CA2CT dir(TargetDir.c_str());
	
	finder.FindFile(dir);
	while (finder.FindNextFile())
	{
		finder.GetCreationTime(&curDate);

		if (CompareFileTime(&curDate, &bestDate) > 0)
		{
			bestDate = curDate;
			name = finder.GetFileName().GetString();
		}
	}
	return name;
}
//=====================================================================





//=====================================================================
//
//=====================================================================
int main(int argc, char *argv[]) // Don't forget first integral argument 'argc'
{
	//stringstream ss;
	string ParentDir = string(argv[1]) + ":\\EKOFISK_Images\\";
	char mmddHH_char[15];
	string TargetDir;
	string fName, fNameAlt;
	string camLeftID = "16048358";
	string camRightID = "16048400";
	string Ext = "*.raw";
	string FnL;
	string FnR;
	FILE * RawfileL;
	FILE * RawfileR;
	cout << "The first is : " << ParentDir << endl;


	while (TRUE)
	{
		Sleep(1000);
		sysTimeStamp(mmddHH_char);
		TargetDir = ParentDir + string(mmddHH_char);
		cout << "Next is : " << TargetDir << endl;
		//Some translating for str to 

		fName = processName(TargetDir + Ext);
		cout << "latest RAW file is : " << fName << endl;
		//Make sure the names are correct:
		fName.replace(23, 8, camLeftID);
		fNameAlt = fName;
		fNameAlt.replace(23, 8, camRightID);
		FnL = TargetDir + fName;
		FnR = TargetDir + fNameAlt;
		cout << "Left Cam: " << fName << " Right Cam: " << fNameAlt << endl;
		cout << "Left Cam: " << FnL << " Right Cam: " << FnR << endl;
		//Display the image using opencv. Now these are RAW image format, so IMREAD() will not work.
		//Instead, I will have to prepare my own import & conversion solution.
		RawfileL = fopen(FnL.c_str(), "rb");
		RawfileR = fopen(FnR.c_str(), "rb");
		//https://stackoverflow.com/questions/21865700/read-a-raw-file-and-convert-it-to-8-bit

		//Mat imLeft, imRight, imTile; // a,b loaded
		//image = imread()

//		hconcat(imLeft, imRight, imTile);

	}

	// Now find the Day, Month and hour UTC:
	//ParentDir.append(mmddHH);
	return 0;
}
//=====================================================================

/*
string LatestFile(string DIRR)
{
	WIN32_FIND_DATAW ffd;
	wstring widestr = wstring(DIRR.begin(), DIRR.end()); //added this
	//wchar_t const* directory = L"D:\\My_GRB_Files\\";
	wchar_t const* directory = widestr.c_str();
	wchar_t currentFile[MAX_PATH], lastModifiedFilename[MAX_PATH];
	FILETIME currentModifiedTime, lastModified;
	HANDLE hFile;
	bool first_file = true;

	//HANDLE hFind = FindFirstFileW(L"D:\\My_GRB_Files\\*.grb2", &ffd);
	HANDLE hFind = FindFirstFileW(directory + L"", &ffd);

	if (INVALID_HANDLE_VALUE == hFind)
	{
		return " ";
	}

	do
	{
		if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			wcscpy(currentFile, directory);
			wcscat(currentFile, ffd.cFileName);
			// open file to read it's last modified time
			hFile = CreateFileW(currentFile, GENERIC_READ, FILE_SHARE_READ, NULL,
				OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if (INVALID_HANDLE_VALUE != hFile)
			{
				// get it's last write time
				if (GetFileTime(hFile, NULL, NULL, &currentModifiedTime) != 0)
				{
					if (first_file)
					{
						lastModified = currentModifiedTime;
						wcscpy(lastModifiedFilename, ffd.cFileName);
						first_file = false;
					}
					else
					{
						// First file time is earlier than second file time.
						if (CompareFileTime(&lastModified, &currentModifiedTime) == -1)
						{
							lastModified = currentModifiedTime;
							wcscpy(lastModifiedFilename, ffd.cFileName);
						}
					}
				}
				CloseHandle(hFile);
			}
		}
	} while (FindNextFileW(hFind, &ffd) != 0);

	FindClose(hFind);
	wprintf(L"Last modified file is: %s%s", directory, lastModifiedFilename);
	wstring ws(lastModifiedFilename);
	string fName(ws.begin(), ws.end());
	return fName;
}*/





/*
string latestFile(string path)
{
	FILETIME bestDate = { 0, 0 };
	FILETIME curDate;
	string name;
	CFileFind finder;

	finder.FindFile(path.c_str());
	while (finder.FindNextFile())
	{
		finder.GetCreationTime(&curDate);

		if (CompareFileTime(&curDate, &bestDate) > 0)
		{
			bestDate = curDate;
			name = finder.GetFileName().GetString();
		}
	}
	return name;
}*/


/*#include "stdafx.h"

using namespace System;
using namespace System::IO;
using namespace System::Collections;

string latestFile(string path)
{
	//get all file names of path 
	array<string> fileEntries = Directory::GetFiles(path);
	int contants = fileEntries->Length;
	//sort file names by crteated date
	for (int i = 0; i<contants - 1; i++)
	{
		for (int j = i + 1; j<contants; j++)
		{
			if (Directory::GetCreationTime(fileEntries[i])<Directory::GetCreationTime(fileEntries[j]))
			{
				String^ t = fileEntries[i];
				fileEntries[i] = fileEntries[j];
				fileEntries[j] = t;
			}
		}
	}
	//select the latest one
	return string(fileEntries[0]);
}*/









/*
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <iostream>
#include <string>
#include <windows.h>
#include <afx.h>
namespace fs = boost::filesystem;
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
int parse_filename(fs::path const& p)
{
	return std::stoi(p.filename().string());
}

void sort_numeric_1(fs::path const& p)
{
	typedef std::pair<fs::path, int> file_entry;
	typedef std::vector<file_entry> vec;
	vec v;

	for (fs::directory_iterator it(p); it != fs::directory_iterator(); ++it) {
		v.emplace_back(*it, parse_filename(*it));
	}

	std::sort(v.begin(), v.end()
		, [](file_entry const& a, file_entry const& b) {
		return a.second < b.second;
	});

	for (vec::const_iterator it(v.begin()), it_end(v.end()); it != it_end; ++it) {
		std::cout << "   " << it->first << '\n';
	}
}

void sort_numeric_2(fs::path const& p)
{
	typedef std::vector<fs::path> vec;
	vec v;

	std::copy(fs::directory_iterator(p), fs::directory_iterator(), back_inserter(v));

	std::sort(v.begin(), v.end()
		, [](fs::path const& a, fs::path const& b) {
		return std::stoi(a.filename().string()) < std::stoi(b.filename().string());
	});

	for (vec::const_iterator it(v.begin()), it_end(v.end()); it != it_end; ++it) {
		std::cout << "   " << *it << '\n';
	}
}

int main()
{
	sort_numeric_1("test");
	std::cout << "\n";
	sort_numeric_2("test");
}





















#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <filesystem.h>
#include <string>
#include <windows.h>
#include <afx.h>
#include <boost.h>

using namespace cv;
using namespace std;


string processName()
{
	FILETIME bestDate = { 0, 0 };
	FILETIME curDate;
	string name;
	CFileFind finder;

	finder.FindFile("*.png");
	while (finder.FindNextFile())
	{
		finder.GetCreationTime(&curDate);

		if (CompareFileTime(&curDate, &bestDate) > 0)
		{
			bestDate = curDate;
			name = finder.GetFileName().GetString();
		}
	}
	return name;
}




int main(int argc, char** argv)
{

	//find the latest directory:


	String imageName("rreet.jpg"); // by default
	cout << "image name is " << imageName << endl;
	if (argc > 1)
	{
		imageName = argv[1];
	}
	Mat image;
	image = imread(imageName, IMREAD_COLOR); // Read the file
	if (image.empty())                      // Check for invalid input
	{
		cout << "Could not open or find the image" << std::endl;
		return -1;
	}
	namedWindow("Display window", WINDOW_AUTOSIZE); // Create a window for display.
	imshow("Display window", image);                // Show our image inside it.
	waitKey(0); // Wait for a keystroke in the window
	return 0;
}



bool find_file(const filesystem::path::path & dir_path,         // in this directory,
	const std::string & file_name, // search for this name,
	path & path_found)            // placing path here if found
{
	if (!exists(dir_path)) return false;
	directory_iterator end_itr; // default construction yields past-the-end
	for (directory_iterator itr(dir_path);
		itr != end_itr;
		++itr)
	{
		if (is_directory(itr->status()))
		{
			if (find_file(itr->path(), file_name, path_found)) return true;
		}
		else if (itr->leaf() == file_name) // see below
		{
			path_found = itr->path();
			return true;
		}
	}
	return false;
}
*/