/*****************************************************************
 * GMaps_XBee.cpp
 *
 * Created: 07-04-2015 19:44:17
 *  Author: ARKA, SHALAKA
 *****************************************************************/ 


#include <windows.h>
#include <iostream>
#include <string.h>
#include <vector>
#include <math.h>
#include <stdlib.h>

using namespace std;

struct LatLng
{
    double lat, lng;
};


vector<LatLng> polyline;

/**Function to decode the polyline into latitude and longitude of multiple points */

void decodePolyline(string encoded)
{
    polyline.clear();

    int index = 0, len = encoded.length();
    int lat = 0, lng = 0;

    while (index < len)
	{
        int b, shift = 0, result = 0;
        do
		{
            b = encoded[index++] - 63;
            result |= (b & 0x1f) << shift;
            shift += 5;
        }
		while (b >= 0x20);
        int dlat = ((result & 1) != 0 ? ~(result >> 1) : (result >> 1));
        lat += dlat;

        shift = 0;
        result = 0;
        do
		{
            b = encoded[index++] - 63;
            result |= (b & 0x1f) << shift;
            shift += 5;
        }
		while (b >= 0x20);
        int dlng = ((result & 1) != 0 ? ~(result >> 1) : (result >> 1));
        lng += dlng;

        LatLng p;

        p.lat= (double) lat / 100000; p.lng= (double) lng / 100000;

        polyline.push_back(p);
    }
}

/** XBee code */

HANDLE hPort = CreateFile("COM14", GENERIC_WRITE|GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
DCB dcb;

bool writebyte (char* data)
{
    DWORD byteswritten;
    if (!GetCommState(hPort, &dcb))
    {
        cout<<"\n Serial port can't be opened";
        return false;
    }

    dcb.BaudRate = CBR_9600;
    dcb.ByteSize = 8;
    dcb.Parity = NOPARITY;
    dcb.StopBits = ONESTOPBIT;

    if (!SetCommState(hPort, &dcb))
    return false;

    bool retVal = WriteFile(hPort, data, 1, &byteswritten, NULL);
    return retVal;
}

int readByte()
{
    int Val;
    BYTE byte;
    DWORD dwBytesTransferred;
    DWORD dwCommModemStatus;
    if(!GetCommState(hPort, &dcb))
        return 0;

    SetCommMask(hPort, EV_RXCHAR|EV_ERR);
    WaitCommEvent(hPort, &dwCommModemStatus, 0);
    if (dwCommModemStatus& EV_RXCHAR)
        ReadFile(hPort, &byte, 1, &dwBytesTransferred, 0);
    Val = byte;
    cout<<"\n Some Data Received::"<<int(Val)<<endl;
    return Val;
}


int main()
{

    double radius = 6371000; //radius of the earth in metres

    string poly ;			 //Test case : poly = "ohxsBya`|LsEaARfB";
	
	
    decodePolyline(poly);

    int x_ground, y_ground, x_prev, y_prev;

    x_ground = x_prev = (int)(radius*cos(polyline[0].lat)*cos(polyline[0].lng));
    y_ground = y_prev = (int)(radius*cos(polyline[0].lat)*sin(polyline[0].lng));

    while(1)
    {

		for(int i = 1; i<polyline.size() && (readByte()==127 || i == 1); i++)
		{
			//cout<<polyline[i].lat<<' '<<polyline[i].lng<<endl;

			//Converting to x - y

			x_prev = x_ground;
			y_prev = y_ground;

			x_ground = (int)(radius*cos(polyline[i].lat)*cos(polyline[i].lng));
			y_ground = (int)(radius*cos(polyline[i].lat)*sin(polyline[i].lng));



			char x = (char)(x_ground-x_prev);
			char y = (char)(y_ground-y_prev);

			if(writebyte(&x) && writebyte(&y))
			{
				cout<<"Sent current destination no. "<<i<<" : "x<<" "<<y<<endl;
			}


		}

    }

    CloseHandle(hPort);
}
