#include <windows.h>
#include <iostream>
#include "D:\ARKA\Computer\Programming\Img_processing_opencv\trialmaps\gmaps.cpp"

using namespace std;

HANDLE hPort = CreateFile("COM2", GENERIC_WRITE|GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
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
    return Val;
}


int main()
{
    char data;
    int x;
    while(1)
    {
        cin>>x;data = x;
        if (writebyte(&data)) cout<<"Data Sent";
        cout<<"\n Data Received::"<<readByte()<<endl;
    }

    CloseHandle(hPort);
}
