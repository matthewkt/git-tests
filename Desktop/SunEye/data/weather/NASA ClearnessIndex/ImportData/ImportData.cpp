// ImportData.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <sstream>
#include <iomanip>

typedef unsigned char byte;

using namespace std;

void Split(string& inStr, vector<string>& splits);
bool ParseFile(const char* pFileName);
int Float2Int(float f) { return (int)(f + ((f>0) ? .5 : -.5)); }

struct LocationPoint
{
    short lat;
    short lon;

    LocationPoint(const LocationPoint& pt) : lat(pt.lat), lon(pt.lon)
    {}

    LocationPoint(int latIn, int lonIn) : lat(latIn), lon(lonIn)
    {}

    bool operator == (const LocationPoint& pt) const
    { return (pt.lat==lat) && (pt.lon==lon); }

    bool operator != (const LocationPoint& pt) const
    {
        return (pt.lat != lat || pt.lon != lon); 
    }

    bool operator > (const LocationPoint& a) const
    { 
        unsigned int key = (lat << 16) | (lon &0x0000FFFF);
        unsigned int aKey = (a.lat << 16) | (a.lon &0x0000FFFF);
        return ( key > aKey );
    }  

    bool operator < (const LocationPoint& a) const
    { 
        unsigned int key = (lat << 16) | (lon &0x0000FFFF);
        unsigned int aKey = (a.lat << 16) | (a.lon &0x0000FFFF);
        return ( key < aKey );
    }  
};

typedef map<LocationPoint, vector<byte>> LocationMapType;
LocationMapType LocationMap;


int _tmain(int argc, _TCHAR* argv[])
{
    bool ok = ParseFile("..\\NASA Clearness Northeast Quadrant.txt");
    if (! ok)
    {
        cout << "Error northeast" << endl;
        return 0;
    }

    ok = ParseFile("..\\NASA Clearness Northwest Quadrant.txt");
    if (! ok)
    {
        cout << "Error northwest" << endl;
        return 0;
    }

    ok = ParseFile("..\\NASA Clearness Southeast Quadrant.txt");
    if (! ok)
    {
        cout << "Error southeast" << endl;
        return 0;
    }

    ok = ParseFile("..\\NASA Clearness Southwest Quadrant.txt");
    if (! ok)
    {
        cout << "Error southwest" << endl;
        return 0;
    }

    ofstream outFile("kt.bin", ios_base::binary);
    if (! outFile.is_open())
    {
        cout << "Error opening out file" << endl;
        return 0;
    }

    for (int lat=0; lat<180; lat++)
    {
        for (int lon=0; lon<360; lon++)
        {
            int trueLat = lat - 90;
            int trueLon = (lon >= 180) ? (360-lon) * -1 : lon;

            LocationPoint pt(trueLat,trueLon);
            LocationMapType::iterator iter = LocationMap.find(pt);
            if (iter == LocationMap.end())
            {
                cout << "Error:  truelat: " << trueLat << "  truelon: " << trueLon << "NOT IN TABLE" << endl;
                return 0;
            }

            vector<byte>& indexes = iter->second;

            if (indexes.size() != 12)
            {
                cout << "Error, 12 indexes for this location don't exist" << endl;
                return 0;
            }

            for (int i=0; i<12; i++)
            {
                byte c = indexes[i];
                outFile.write((char*)&c, sizeof(byte));
            }
        }
    }

	return 0;
}

bool ParseFile(const char* pFileName)
{
    ifstream fileStrm(pFileName);

    if (fileStrm.bad())
    {
        cout << "Error opening " << pFileName << endl;
        return false;
    }

    bool beginImport = false;

    while (! fileStrm.eof())
    {
        char buffer[1024];
        fileStrm.getline(buffer, 1024);
        string s(buffer);
        if (s.empty() || (s.size() == 0) )
        {
            continue;
        }

        if (! beginImport)
        {
            if (s.find("Lat Lon Jan") == 0)
            {
                beginImport = true;
            }
            continue;
        }

        vector<string> splits;
        Split(s, splits);
        if (splits.size() != 14)
        {
            cout << "Error splitting line in file " << pFileName;
            return false;
        }

        // get latitude
        istringstream latStrm(splits[0]); 
        int lat;
        latStrm >> lat;

        // get longitude
        istringstream lonStrm(splits[1]); 
        int lon;
        lonStrm >> lon;

        LocationPoint pt(lat, lon);

         pair<LocationMapType::iterator,bool> result =
             LocationMap.insert(make_pair(pt, vector<byte>()));
         vector<byte>& indexes = result.first->second;
 
         for (int i=0; i<12; i++)
         {
             string& monthIndexStr = splits[i+2];
             float clearness = 0.f;
             if (monthIndexStr.find("na") == string::npos)
             {
                 istringstream clearnessStrm(monthIndexStr);
                 clearnessStrm >> clearness;
             }
             // convert float to a byte * 100
             clearness *= 100.f;
             byte byteClearness = (byte)Float2Int(clearness);
             indexes.push_back(byteClearness);
         }
    }

    return true;
}

void Split(string& inStr, vector<string>& splits)
{
    string::size_type index1 = 0;
    string::size_type index2 = 0;

    while ( (index2 = inStr.find(" ", index1)) != string::npos )
    {
        string s = inStr.substr(index1, index2-index1);
        splits.push_back(s);
        index1 = index2+1;
    }

    string s = inStr.substr(index1, index2-index1);
    splits.push_back(s);
}


