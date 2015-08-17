#include <windows.h>
#include <string>
#include <vector>
#include <stack>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <algorithm>

using namespace std;

/**
	Store a list of files from a given path into the files vector.
*/
bool ListFiles(wstring path, wstring mask, vector<wstring>& files) {
    HANDLE hFind = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA ffd;
    wstring spec;
    stack<wstring> directories;

    directories.push(path);
    files.clear();

    while (!directories.empty()) {
        path = directories.top();
        spec = path + L"\\" + mask;
        directories.pop();

        hFind = FindFirstFile(spec.c_str(), &ffd);
        if (hFind == INVALID_HANDLE_VALUE)  {
            return false;
        } 

        do {
            if (wcscmp(ffd.cFileName, L".") != 0 && 
                wcscmp(ffd.cFileName, L"..") != 0) {
                if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                    directories.push(path + L"\\" + ffd.cFileName);
                }
                else {
                    files.push_back(path + L"\\" + ffd.cFileName);
                }
            }
        } while (FindNextFile(hFind, &ffd) != 0);

        if (GetLastError() != ERROR_NO_MORE_FILES) {
            FindClose(hFind);
            return false;
        }

        FindClose(hFind);
        hFind = INVALID_HANDLE_VALUE;
    }

    return true;
}

/*
	This function takes a full path and strips it to a c++ compliant macro/#define name.
*/
std::string GetMacroName( std::wstring wideFullPath )
{
	// Get our full path out of wide character mode-
	char buf[512];
	wcstombs( buf , wideFullPath.c_str() , sizeof buf );
	string fullPath = buf;

	// Get just the file name
	size_t pos = fullPath.find_last_of("/\\");
	string tempName = fullPath.substr(pos+1);

	// Erase the extension
	tempName.erase( tempName.find(".") , tempName.length() ); 

	// Make the name uppercase
	std::transform(tempName.begin(), tempName.end(), tempName.begin(), toupper);

	string finalName = "H_" + tempName;
	cout << finalName << endl;
	return finalName;
}

bool WriteGuard( std::wstring fullPath )
{
	ifstream file;
	vector<string> contents;
	file.open( fullPath.c_str() );

	// Open file as read only and store as vector of strings

	string macroName = GetMacroName(fullPath);
	contents.push_back("#ifndef " + macroName + "\n#define " + macroName + "\n");

	if(file.is_open())
	{
		std::string currentLine;
		while(! file.eof() )
		{
			getline( file , currentLine );
			contents.push_back(currentLine);
		}

		contents.push_back("\n#endif");

	}

	file.close();

	ofstream newFile;
	newFile.open( fullPath.c_str() );
	
    for (vector<string>::iterator it = contents.begin(); it != contents.end(); ++it) 
	{
		newFile << it->c_str() << endl;
	}

	newFile.close();

	return true;
}

int main(int argc, char* argv[])
{
    vector<wstring> files;
	std::wstring dir = L"C:\\TestDir";

    if (ListFiles(dir, L"*.h", files)) {
        for (vector<wstring>::iterator it = files.begin(); 
             it != files.end(); 
             ++it) {
			WriteGuard( it->c_str() );
        }
    }
    return 0;
}
