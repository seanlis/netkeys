// NetworkKeys.cpp : Defines the entry point for the console application.
//

#include <winsock2.h>

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500 // Minimum platform is Win 2000
#endif

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <time.h>
#include <string>
#include <iostream>
#include <string>

using namespace std;

#define MESSAGE_MAGIC   0xdeadbeef
#define MESSAGE_KEYUP   0x00000001
#define MESSAGE_KEYDOWN 0x00000002

#define SYSTEM_PORT     23000

struct keyMessage
{
    DWORD magic;
    DWORD code;
};

HHOOK myHook = NULL;
int mySocket = -1;
string destIp = "";
volatile bool isDown = false;

// Function prototypes
LRESULT __declspec(dllexport)__stdcall  CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
void death();
int initWinsock();
int getSocket();
int startListen(int port);
int connectRemote(const char *host, int port);
int sendMessage(const char *msg, int len);
int recvMessage(char *buff, int msglen);
int returnSocket();
int destroyWinsock();

// Normal run: hook and send PTT presses
void hookAndSend()
{
    connectRemote(destIp.c_str(), SYSTEM_PORT);

    // Grab our window handle
    HWND hWnd = GetConsoleWindow();
    HINSTANCE hInst = (HINSTANCE) GetWindowLong(hWnd, GWL_HINSTANCE);

    // Spruce up the window
    SetConsoleTitle("PTT Forwarder");

    // Install the hook
    myHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, hInst, 0);
    if(!myHook)
    {
        cout << "Obtaining the keyboard hook failed. Press any key to continue." << endl;
        _getch();
        return;
    }

    // Tell the user what's happening
    cout << "PTT Forwarder is now running. Any press of your PTT will now be sent to your secondary computer." << endl;
    cout << "Just close this window when you're done, it'll automatically shut everything down." << endl;

    // Main message pump
    MSG msg;
    while(GetMessage(&msg, hWnd, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Shouldn't really get here
	return;
}

// Client run: just listen out for the down/up messages and handle them
void listenForPtt()
{
    startListen(SYSTEM_PORT);

    struct keyMessage s;
    while(recvMessage((char*) &s, sizeof(s)) != -1)
    {
        if(s.magic != MESSAGE_MAGIC)
        {
            cerr << "Got a bad packet [magic=" << cerr.hex << s.magic << cerr.dec << "]" << endl;
            continue;
        }

        INPUT wrapper;
        wrapper.type = INPUT_KEYBOARD;

        KEYBDINPUT myInput;
        myInput.dwExtraInfo = 0;
        myInput.dwFlags = KEYEVENTF_EXTENDEDKEY;
        myInput.time = 0;
        myInput.wVk = VK_RCONTROL;
        myInput.wScan = 0x1D;

        if(s.code == MESSAGE_KEYUP)
            myInput.dwFlags |= KEYEVENTF_KEYUP;

        wrapper.ki = myInput;
        SendInput(1, &wrapper, sizeof(wrapper));
    }
}

int main(int argc, char* argv[])
{
    // Initialise Winsock and grab a socket
    if(initWinsock())
    {
        cout << "Couldn't initialise the socket layer. Press any key to exit." << endl;
        _getch();
        return 1;
    }
    getSocket();

    // Install the unhooker to run when we die
    atexit(death);

    // Check for and handle input parameters
    bool bClient = false;
    for(int i = 1; i < argc; i++)
    {
        if(_stricmp(argv[i], "--client") == 0)
        {
            bClient = true;
        }
        else if(_stricmp(argv[i], "--ip") == 0)
        {
            if((i + 1) < argc)
            {
                destIp = argv[i + 1];
                i++;
            }
            else
            {
                cout << "The 'ip' argument requires an IP." << endl;
                return 1;
            }
        }
    }

    if(destIp == "")
    {
        cout << "No IP specified, quitting." << endl;
        return 1;
    }

    // Client?
    if(bClient)
    {
        listenForPtt();
        returnSocket();
        destroyWinsock();
    }
    // No, we're not the client. Hook and serve.
    else
        hookAndSend();
    return 0;
}

LRESULT __declspec(dllexport)__stdcall  CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if(nCode == HC_ACTION)
    {
        KBDLLHOOKSTRUCT *p = (KBDLLHOOKSTRUCT*) lParam;
        if(p->vkCode == VK_RCONTROL)
        {
            if(wParam == WM_KEYDOWN)
            {
                if(!isDown)
                {
                    isDown = true;
                    struct keyMessage s;
                    s.code = MESSAGE_KEYDOWN;
                    s.magic = MESSAGE_MAGIC;
                    sendMessage((const char*) &s, sizeof(s));
                }
            }
            if(wParam == WM_KEYUP)
            {
                struct keyMessage s;
                s.code = MESSAGE_KEYUP;
                s.magic = MESSAGE_MAGIC;
                sendMessage((const char*) &s, sizeof(s));

                isDown = false;
            }
        }
    }
    return CallNextHookEx(myHook, nCode, wParam, lParam);
}

void death()
{
    if(myHook)
        UnhookWindowsHookEx(myHook);

    returnSocket();
    destroyWinsock();
}

int initWinsock()
{
    WORD wVersionRequested;
    WSADATA wsaData;
    int err;

    /* Use the MAKEWORD(lowbyte, highbyte) macro declared in Windef.h */
    wVersionRequested = MAKEWORD(2, 2);

    err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0) {
        /* Tell the user that we could not find a usable */
        /* Winsock DLL.                                  */
        cout << "WSAStartup failed with error: " << err << endl;
        return 1;
    }

    /* Confirm that the WinSock DLL supports 2.2.*/
    /* Note that if the DLL supports versions greater    */
    /* than 2.2 in addition to 2.2, it will still return */
    /* 2.2 in wVersion since that is the version we      */
    /* requested.                                        */

    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
        /* Tell the user that we could not find a usable */
        /* WinSock DLL.                                  */
        cout << "Could not find a usable version of Winsock.dll" << endl;
        WSACleanup();
        return 1;
    }
    else
        return 0;
}

int getSocket()
{
    if(mySocket == -1)
        mySocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    return mySocket;
}

int startListen(int port)
{
    if(mySocket < 0)
        return -1;

    sockaddr_in service;
    service.sin_family = AF_INET;
    service.sin_addr.s_addr = INADDR_ANY;
    service.sin_port = htons(port);
    if(bind(mySocket, (SOCKADDR*) &service, sizeof(service)) == SOCKET_ERROR)
        return -1;

    return 0;
}

int connectRemote(const char *host, int port)
{
    if(mySocket < 0)
        return -1;

    sockaddr_in service;
    service.sin_family = AF_INET;

    struct hostent *ent;
    if((service.sin_addr.s_addr = inet_addr(host)) == INADDR_NONE)
    {
        if((ent = gethostbyname(host)) != NULL)
        {
            service.sin_addr.s_addr = inet_addr(ent->h_addr_list[0]);
        }
        else
            return -1;
    }

    service.sin_port = htons(port);

    return connect(mySocket, (SOCKADDR*) &service, sizeof(service));
}

int sendMessage(const char *msg, int len)
{
   if(mySocket < 0)
       return -1;

   return send(mySocket, msg, len, 0);
}

int recvMessage(char *buff, int msglen)
{
   if(mySocket < 0)
       return -1;

   return recv(mySocket, buff, msglen, 0);
}

int returnSocket()
{
    if(mySocket >= 0)
    {
        shutdown(mySocket, SD_BOTH);
        closesocket(mySocket);
    }
    return 0;
}

int destroyWinsock()
{
    return WSACleanup();
}
