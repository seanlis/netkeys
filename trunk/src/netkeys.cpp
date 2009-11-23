/*
 * Copyright (c) 2009 Matthew Iselin
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#if defined(__linux__) || defined(linux) /// \todo This isn't exhaustive
#define _LINUX

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

/// Value returned in the case of an error when obtaining a socket
#define SOCKET_ERROR_VALUE (-1)

/// Type of a socket
#define SOCKET int

/// The type of ports for use with connect/bind
/// \todo Should be determined by autoconf
#define PORT_TYPE uint16_t

#define SD_BOTH SHUT_RDWR
#elif defined(WIN32)

// netkeys targets a minimum platform of Windows 2000
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500 // Minimum platform is Win 2000
#endif

#include <winsock2.h>
#include <windows.h>
#include <conio.h>

#define HAVE_STRCPY_S

/// Value returned in the case of an error when obtaining a socket
/// \note SOCKET_ERROR is already taken by Windows, *sigh*
#define SOCKET_ERROR_VALUE (INVALID_SOCKET)

/// The type of ports for use with connect/bind
#define PORT_TYPE u_short
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <string>
#include <iostream>
#include <string>
#include <map>

using namespace std;

#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif

#ifdef _LINUX
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/extensions/XTest.h>
#endif

#ifndef HAVE_STRCPY_S
#define strncpy_s(d, n, s, m) strncpy(d, s, m)
#endif

#if !defined(HAVE_STRICMP) && !defined(WIN32)
int stricmp(const char *a, const char *b)
{
    while(*a && *b)
    {
        char ac = tolower(*a), bc = tolower(*b);
        if(ac < bc)
            return -1;
        else if(ac > bc)
            return 1;
        a++; b++;
    }
    return 0;
}
#define _stricmp stricmp
#endif

#ifndef WIN32
typedef uint32_t DWORD;
typedef uint16_t WORD;
#endif

#ifdef WIN32
#define stricmp _stricmp
#endif

/** Major version identifier */
#define VER_MAJOR       0

/** Minor version identifier */
#define VER_MINOR       1

/**
 * MESSAGE_MAGIC serves two purposes: one, ensuring the correct structures
 * are in use, and two, ensuring that these structures are not malformed.
 */
#define MESSAGE_MAGIC   0xdeadbeef

/**
 * These two definitions are merely codes for each message between the two
 * hosts.
 */
#define MESSAGE_KEYUP   0x00000001
#define MESSAGE_KEYDOWN 0x00000002

/** Default port for all communication */
#define SYSTEM_PORT     23000

/**
 * Packet structure for the key press/release message transfer.
 */
struct keyMessage
{
    /** Magic number, to verify version and integrity */
    DWORD magic;

    /** Action code */
    DWORD code;

    /** Name of key that was pressed (allows Linux to talk to Windows) */
    char keyName[32];
};

/**
 * Translation from string to a keycode. These will be converted into a
 * std::map and used to determine the set of keys to transmit.
 */
struct keyTranslation
{
    string keyName;
    DWORD vkCode;
    bool shouldRetransmit;
} myTranslations[] = {
#ifdef WIN32
#include "win_keytrans.h"
#elif defined(_LINUX)
#include "x_keytrans.h"
#endif
};

#ifdef WIN32
/**
 * Low-level keyboard hook. Global so it can be deleted via an atexit setup
 * rather than requiring a full message pump with special WM_CLOSE handling.
 */
HHOOK myHook = NULL;
#elif defined(_LINUX)
Display *myDisplay = NULL;
Window rootWindow;
#endif

/**
 * Global network socket.
 *
 * Probably not the greatest way in the world to do things. I'd like to C++
 * this up a bit.
 */
SOCKET mySocket = SOCKET_ERROR_VALUE;

/**
 * Port on which to communicate. Defaults to myPort unless a port is
 * specified on the command line.
 */
PORT_TYPE myPort = SYSTEM_PORT;

/**
 * Remote IP for all networking.
 */
string remoteIp = "";

/** Map of keys to transmit */
map<DWORD, bool> keysToTransmit;

/** Map of strings -> virtual key codes */
map<string, DWORD> keyTranslations;

/** Map of virtual key codes -> strings */
map<DWORD, string> vkTranslations;

/** Key states (to determine whether or not to send another message */
map<DWORD, bool> keyState;

/** Whether or not to avoid retransmitting KEYDOWN events */
map<DWORD, bool> keyRetransmit;

#ifdef WIN32
/** KeyboardProc: Low-level keyboard hook handler */
LRESULT __declspec(dllexport)__stdcall  CALLBACK KeyboardProc(int nCode,
                                                              WPARAM wParam,
                                                              LPARAM lParam);

LRESULT __declspec(dllexport)__stdcall CALLBACK RegKeyboardProc(int code,
                                                                WPARAM wParam,
                                                                LPARAM lParam);

/**
 * initWinsock
 *
 * Initialises Windows sockets
 * \todo Move into a class!
 */
/// 
int initWinsock();

/** destroyWinsock: Cleans up Windows Sockets */
int destroyWinsock();

#endif

/** death: atexit routine */
void death();

/** getSocket: Set & return mySocket if not set, return it if so. */
SOCKET getSocket();

/** startListen: Listen on the given port */
int startListen(PORT_TYPE port);

/**
 * connectRemote
 *
 * "Connect" to the given host on the given port. Because this is all done over
 * UDP this is more or less defining the IP with which we'll be performing send
 * and recv operations.
 */
int connectRemote(const char *host, PORT_TYPE port);

/** sendMessage: Sends the given message. */
int sendMessage(const char *msg, int len);

/** recvMessage: Blocks and receives a message */
int recvMessage(char *buff, int msglen);

/** returnSocket: Cleans up mySocket */
int returnSocket();

/**
 * hookAndSend
 *
 * This function is the "server" side of things. It creates a low-level
 * keyboard hook that is used to pick up every key the user presses.
 * Basically, it dumps a little bit of information to the console and then
 * takes over the Windows message pump for the console.
 */
void hookAndSend()
{
    connectRemote(remoteIp.c_str(), myPort);

#ifdef WIN32
    // Grab our window handle (to take over the message pump
    HWND hWnd = GetConsoleWindow();
    HINSTANCE hInst = (HINSTANCE) GetModuleHandle(NULL);

    // Spruce up the window
    SetConsoleTitle("netkeys");

    // Install the low-level hook
    myHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, hInst, 0);
    if(!myHook)
    {
        // If that failed, try a normal keyboard hook
        myHook = SetWindowsHookEx(WH_KEYBOARD, RegKeyboardProc, hInst, 0);
        if(!myHook)
        {
            cerr << "Obtaining the keyboard hook failed [" << GetLastError() << "]." << endl;
            cerr << "Press any key to continue." << endl;
            _getch();
            return;
        }
    }
#elif defined(_LINUX)
    rootWindow = DefaultRootWindow(myDisplay);
    for(map<DWORD, bool>::iterator it = keysToTransmit.begin();
        it != keysToTransmit.end();
        ++it)
    {
        /// \todo For some reason, this eats all keypresses, which is far from
        ///       ideal.
        KeyCode key = XKeysymToKeycode(myDisplay, (*it).first);
        XGrabKey(myDisplay, key, AnyModifier, rootWindow, True, 
                 GrabModeAsync, GrabModeAsync);
    }
#endif

    // Tell the user what's happening
    cout << "netkeys is now running." << endl;
    cout << "Just close this window when you're done." << endl;
    cout << "I'll automatically shut everything down." << endl;

#ifdef WIN32
    // Main message pump
    MSG msg;
    while(GetMessage(&msg, hWnd, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
#elif defined(_LINUX)
    for(;;)
    {
        XEvent ev;
    	XNextEvent(myDisplay, &ev);
    	KeySym code = XKeycodeToKeysym(myDisplay, ev.xkey.keycode, 0);
        if(keysToTransmit[code])
        {
    	    if(ev.type == KeyPress)
    	    {
                if(keyRetransmit[code] || !keyState[code])
                {
                    keyState[code] = true;
                    
                    struct keyMessage s;
                    s.code = MESSAGE_KEYDOWN;
                    s.magic = MESSAGE_MAGIC;
                    strncpy_s(s.keyName, 32, vkTranslations[code].c_str(), 32);
                    sendMessage((const char*) &s, sizeof(s));
                }
    	    }
    	    else if(ev.type == KeyRelease)
    	    {
                struct keyMessage s;
                s.code = MESSAGE_KEYUP;
                s.magic = MESSAGE_MAGIC;
                strncpy_s(s.keyName, 32, vkTranslations[code].c_str(), 32);
                sendMessage((const char*) &s, sizeof(s));
                
                keyState[code] = false;
    	    }
    	}
    }
#endif

    // Shouldn't really get here
	return;
}

/**
 * listenForKeys
 *
 * This function is the "client" side of things. It merely listens for any
 * incoming key messages, ensures that they're valid, and then injects them
 * into the host key press queue.
 *
 * \todo Allow the user to type something like "quit" or whatever to exit,
 *       perhaps via a small command-line monitor to also see some statistics
 * \todo It's real easy to kill things right now: just exit the client while
 *       holding down a key on the host to find out why (the key remains held
 *       down, as the keyup event is never received).
 */
void listenForKeys()
{
    startListen(myPort);

    struct keyMessage s;
    while(recvMessage((char*) &s, sizeof(s)) != -1)
    {
        if(s.magic != MESSAGE_MAGIC)
        {
            /// \todo Better error-handling, more consistent, etc...
            cerr << "netkeys: non-fatal: Got a bad packet." << endl;
            cerr << "packet magic: " << hex << s.magic << dec << "]" << endl;
            continue;
        }

        DWORD keyCode = keyTranslations[string(s.keyName)];
#if WIN32
        INPUT wrapper;
        wrapper.type = INPUT_KEYBOARD;

        KEYBDINPUT myInput;
        myInput.dwExtraInfo = 0;
        myInput.dwFlags = 0;
        myInput.time = 0;
        myInput.wVk = (WORD) keyCode;
        myInput.wScan = (WORD) MapVirtualKey(keyCode, MAPVK_VK_TO_VSC);

        if(s.code == MESSAGE_KEYUP)
            myInput.dwFlags |= KEYEVENTF_KEYUP;

        wrapper.ki = myInput;
        SendInput(1, &wrapper, sizeof(wrapper));
#elif defined(_LINUX)
	    unsigned int kc = XKeysymToKeycode(myDisplay, keyCode);
	    
	    // Avoid sending the character if it's to come up now
	    if(s.code != MESSAGE_KEYUP)
    	    XTestFakeKeyEvent(myDisplay, kc, True, 0);
        if(s.code == MESSAGE_KEYUP)
	        XTestFakeKeyEvent(myDisplay, kc, False, 0);
	    XFlush(myDisplay);
#endif
    }
}

#ifdef WIN32
LRESULT __declspec(dllexport)__stdcall  CALLBACK KeyboardProc(int nCode,
                                                              WPARAM wParam,
                                                              LPARAM lParam)
{
    if(nCode == HC_ACTION)
    {
        KBDLLHOOKSTRUCT *p = (KBDLLHOOKSTRUCT*) lParam;
        if(keysToTransmit[p->vkCode])
        {
            if(wParam == WM_KEYDOWN)
            {
                if(keyRetransmit[p->vkCode] || !keyState[p->vkCode])
                {
                    keyState[p->vkCode] = true;
                    struct keyMessage s;
                    s.code = MESSAGE_KEYDOWN;
                    s.magic = MESSAGE_MAGIC;
                    strncpy_s(s.keyName, 32, vkTranslations[p->vkCode].c_str(), 32);
                    sendMessage((const char*) &s, sizeof(s));
                }
            }
            if(wParam == WM_KEYUP)
            {
                struct keyMessage s;
                s.code = MESSAGE_KEYUP;
                s.magic = MESSAGE_MAGIC;
                strncpy_s(s.keyName, 32, vkTranslations[p->vkCode].c_str(), 32);
                sendMessage((const char*) &s, sizeof(s));

                keyState[p->vkCode] = false;
            }
        }
    }
    return CallNextHookEx(myHook, nCode, wParam, lParam);
}

LRESULT __declspec(dllexport)__stdcall CALLBACK RegKeyboardProc(int nCode,
                                                                WPARAM wParam,
                                                                LPARAM lParam)
{
    if(nCode == HC_ACTION)
    {
        WORD vkCode = LOWORD(wParam);
        if(keysToTransmit[vkCode])
        {
            // Bit 31: Transition state. 0 if being pressed, 1 if being released.
            if(!(lParam & (1 << 31)))
            {
                if(keyRetransmit[vkCode] || !keyState[vkCode])
                {
                    keyState[vkCode] = true;
                    struct keyMessage s;
                    s.code = MESSAGE_KEYDOWN;
                    s.magic = MESSAGE_MAGIC;
                    strncpy_s(s.keyName, 32, vkTranslations[vkCode].c_str(), 32);
                    sendMessage((const char*) &s, sizeof(s));
                }
            }
            if(lParam & (1 << 31))
            {
                struct keyMessage s;
                s.code = MESSAGE_KEYUP;
                s.magic = MESSAGE_MAGIC;
                strncpy_s(s.keyName, 32, vkTranslations[vkCode].c_str(), 32);
                sendMessage((const char*) &s, sizeof(s));

                keyState[vkCode] = false;
            }
        }
    }
    return CallNextHookEx(myHook, nCode, wParam, lParam);
}
#endif

void death()
{
    returnSocket();
#ifdef WIN32
    if(myHook)
        UnhookWindowsHookEx(myHook);
    
    destroyWinsock();
#elif defined(_LINUX)
    XCloseDisplay(myDisplay);
#endif
}

#ifdef WIN32
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
        cerr << "WSAStartup failed with error: " << err << endl;
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
        cerr << "Could not find a usable version of Winsock.dll" << endl;
        WSACleanup();
        return 1;
    }
    else
        return 0;
}
#endif

SOCKET getSocket()
{
    if(mySocket == SOCKET_ERROR_VALUE)
        mySocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    return mySocket;
}

int startListen(PORT_TYPE port)
{
    if(mySocket == SOCKET_ERROR_VALUE)
        return -1;

    sockaddr_in service;
    service.sin_family = AF_INET;
    service.sin_addr.s_addr = INADDR_ANY;
    service.sin_port = htons(port);
    if(bind(mySocket, (sockaddr*) &service, sizeof(service)) == SOCKET_ERROR)
        return -1;

    return 0;
}

int connectRemote(const char *host, PORT_TYPE port)
{
    if(mySocket == SOCKET_ERROR_VALUE)
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

    return connect(mySocket, (sockaddr*) &service, sizeof(service));
}

int sendMessage(const char *msg, int len)
{
   if(mySocket == SOCKET_ERROR_VALUE)
       return -1;

   return send(mySocket, msg, len, 0);
}

int recvMessage(char *buff, int msglen)
{
   if(mySocket == SOCKET_ERROR_VALUE)
       return -1;

   return recv(mySocket, buff, msglen, 0);
}

int returnSocket()
{
    if(mySocket >= 0)
    {
        shutdown(mySocket, SD_BOTH);
#ifdef WIN32
        closesocket(mySocket);
#elif defined(_LINUX)
        close(mySocket);
#endif
    }
    return 0;
}

#ifdef WIN32
int destroyWinsock()
{
    return WSACleanup();
}
#endif

int main(int argc, char* argv[])
{
#ifdef WIN32
    // Initialise Winsock and grab a socket
    if(initWinsock())
    {
        cerr << "Couldn't initialise the socket layer." << endl;
        cerr << "Press any key to exit." << endl;
        _getch();
        return 1;
    }
#endif
    getSocket();

#ifdef _LINUX
    myDisplay = XOpenDisplay(0);
    if(myDisplay == NULL)
    {
        cout << "Couldn't access the X server (display 0)." << endl;
        return 1;
    }
#endif

    // Install cleanup to run when we die
    atexit(death);

    // Setup the translation map
    int i;
    for(i = 0; i < (sizeof(myTranslations) / sizeof(myTranslations[0])); i++)
    {
        keyTranslations[myTranslations[i].keyName] = myTranslations[i].vkCode;
        vkTranslations[myTranslations[i].vkCode] = myTranslations[i].keyName;
        keyRetransmit[myTranslations[i].vkCode] = myTranslations[i].shouldRetransmit;
    }

    // Check for and handle input parameters
    /// \todo C++ify
    bool bClient = false;
    bool bTest = false;
    for(i = 1; i < argc; i++)
    {
        if(stricmp(argv[i], "--client") == 0)
        {
            bClient = true;
        }
        else if(stricmp(argv[i], "--ip") == 0)
        {
            if((i + 1) < argc)
            {
                remoteIp = argv[i + 1];
                i++;
            }
            else
            {
                cout << "The 'ip' argument requires an IP." << endl;
                return 1;
            }
        }
        else if(stricmp(argv[i], "--port") == 0)
        {
            bool bValid = false;
            if((i + 1) < argc)
            {
                myPort = (PORT_TYPE) atoi(argv[i + 1]);
                i++;

                if(myPort > 0 && myPort < 0xFFFF)
                    bValid = true;
            }

            if(!bValid)
            {
                cout << "The 'port' argument requires a valid port." << endl;
                return 1;
            }
        }
        else if(stricmp(argv[i], "--config") == 0)
        {
            /// \todo Read the config file and add to the transmit map.
            cout << "Config files are not supported yet." << endl;
            i++;
        }
        else if(stricmp(argv[i], "--help") == 0)
        {
            // Dump some help to the screen
            /// \todo When C++ifying arguments, this'll be far simpler
            cout << "netkeys " << VER_MAJOR << "." << VER_MINOR << endl;
            cout << "Copyright 2009 Matthew Iselin" << endl;
            cout << endl;
            cout << "Usage: netkeys [--client] [--ip addr] [--port port]" << endl;
            cout << "               [--config file] [keys to transmit]:" << endl;
            cout << endl;
            cout << "--ip <location>:            Specifies a hostname or IP to connect to." << endl;
            cout << "--port <port>:   (optional) Specifies a port to use when connecting." << endl;
            cout << "--client:                   If specified, receives keys from the network" << endl;
            cout << "                            and emulates them being pressed." << endl;
            cout << "--config:        (optional) Specifies a configuration file to use." << endl;
            cout << "                            Not implemented." << endl;
            cout << endl;
            cout << "When running without the --client option, add keys to the command line that" << endl;
            cout << "you wish to transmit over the network. For example:" << endl;
            cout << endl;
            cout << "Primary computer (10.0.0.1):    `netkeys --ip 10.0.0.2 RCtrl`" << endl;
            cout << "Secondary computer (10.0.0.2):  `netkeys --client`" << endl;
            cout << endl;
            cout << "The secondary computer will now receive and emulate presses of the Right" << endl;
            cout << "Control key." << endl;
            cout << endl;
            return 0;
        }
        else if(stricmp(argv[i], "--test") == 0)
        {
            // The --test option allows the hook acquire to be tested without
            // requiring a full test environment (with multiple instances).
            bTest = true;
        }
        else
        {
            // If it's in the translation map, add it to the list of keys we
            // are to transmit
            DWORD vkCode = keyTranslations[string(argv[i])];
            if(vkCode)
                keysToTransmit[vkCode] = true;

            cout << "Key translation for '" << string(argv[i]) << "': ";
            cout << keyTranslations[string(argv[i])] << endl;
        }
    }

    if(!bTest && (remoteIp == "" && !bClient))
    {
        cout << "No IP specified, quitting." << endl;
        return 1;
    }

    if(bClient)
    {
        // We are the client, listen for key messages
        listenForKeys();
        returnSocket();

#ifdef WIN32
        destroyWinsock();
#endif
    }
    else
    {
        // No, we're not the client. Hook and transmit keys.
        hookAndSend();
    }
    return 0;
}
