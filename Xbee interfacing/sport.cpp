#include<windows.h>
#include<string.h>
#include<conio.h>
#include<iostream>

using namespace std;
// Initialize the serial port variable and parameters

HANDLE hPort = CreateFile("COM10",GENERIC_WRITE|GENERIC_READ,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
DCB dcb; // Create a DCB struct variable

/*********************Function for Sending Data***********************/
bool writebyte(char* data)
{
    DWORD byteswritten;
    if (!GetCommState(hPort,&dcb))
    {
        cout<<"\nSerial port cant b opened\n";
        return false;
    }
    dcb.BaudRate = CBR_9600; //9600 Baud
    dcb.ByteSize = 8; //8 data bits
    dcb.Parity = NOPARITY; //no parity
    dcb.StopBits = ONESTOPBIT; //1 stop
    if (!SetCommState(hPort,&dcb)) //If Com port cannot be configured accordingly
    return false;
    bool retVal = WriteFile(hPort,data,1,&byteswritten,NULL); //Write the data to be sent to Serial port
    return retVal; // return true if the data is written
}

int ReadByte()
{
    int Val;
    BYTE Byte;
    DWORD dwBytesTransferred;
    DWORD dwCommModemStatus;
    if (!GetCommState(hPort,&dcb))
    return 0;


    SetCommMask (hPort, EV_RXCHAR | EV_ERR); //receive character event
    WaitCommEvent (hPort, &dwCommModemStatus, 0); //wait for character
    if (dwCommModemStatus & EV_RXCHAR)
    ReadFile (hPort, &Byte, 1, &dwBytesTransferred, 0);
    Val = Byte;
    return Val;
}


int main()
{
    cout<<"\n\n /**************** SERIAL PORT PROGRAM ****************/ \n\n";
    cout<<"This is a loopback test..The Tx & Rx pins of serial port are shorted here...\n\n";
    char data=2; // Enter any data to b sent
    if(writebyte(&data)) // if the function returns non-zero value
    cout<<" Data Sent.. "<<(int)data<<"\n";

    cout<<" Data Received.. "<<ReadByte();
    CloseHandle(hPort); // Close the Serial port handle
    getch();
    return 0;
}

