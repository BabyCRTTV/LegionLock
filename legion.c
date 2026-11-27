#define UNICODE
#include <windows.h>
#include <xinput.h>
#include <wchar.h>
static HWND mainwin,cover,binding,status;
static DWORD shortcut=0x300,prev[4],learned;
static int learning,slot=-1,blank;
static const WCHAR key[]=L"Software\\8BitTrade\\LegionLock";
static void describe(void){
 const WCHAR *names[]={L"Up",L"Down",L"Left",L"Right",L"Start",L"Back",L"LS",L"RS",L"LB",L"RB",L"Guide",L"",L"A",L"B",L"X",L"Y",L"LT",L"RT"};
 WCHAR buf[240]=L"Controller shortcut:  ";int first=1;
 for(int i=0;i<18;i++)if(shortcut&(1u<<i)){if(!first)lstrcatW(buf,L" + ");lstrcatW(buf,names[i]);first=0;}
 SetWindowTextW(binding,buf);
}
static void bounds(void){SetWindowPos(cover,HWND_TOPMOST,GetSystemMetrics(76),GetSystemMetrics(77),GetSystemMetrics(78),GetSystemMetrics(79),SWP_NOACTIVATE|SWP_SHOWWINDOW);}
static void dark(void){learning=0;if(!SetThreadExecutionState(ES_CONTINUOUS|ES_SYSTEM_REQUIRED|ES_DISPLAY_REQUIRED)){SetWindowTextW(status,L"Windows could not keep the system awake.");return;}blank=1;bounds();POINT p;GetCursorPos(&p);if(WindowFromPoint(p)==cover)SetCursor(NULL);}
static void wake(void){blank=0;ShowWindow(cover,SW_HIDE);SetThreadExecutionState(ES_CONTINUOUS);SetCursor(LoadCursor(NULL,IDC_ARROW));SetWindowTextW(status,L"Ready.");}
static void poll(void){int activate=0,pressed=0;
 for(int i=0;i<4;i++){XINPUT_STATE s={0};DWORD b=0;if(!XInputGetState(i,&s)){b=s.Gamepad.wButtons;if(s.Gamepad.bLeftTrigger>30)b|=65536;if(s.Gamepad.bRightTrigger>30)b|=131072;}
 pressed|=(b&~prev[i])!=0;activate|=(b&shortcut)==shortcut&&(prev[i]&shortcut)!=shortcut;prev[i]=b;
 if(learning&&slot<0&&b)slot=i;
 if(learning&&slot==i){learned|=b;if(!b&&learned){shortcut=learned;learning=0;RegSetKeyValueW(HKEY_CURRENT_USER,key,L"Shortcut",REG_DWORD,&shortcut,4);describe();SetWindowTextW(status,L"Shortcut saved.");return;}}}
 if(learning)return;if(blank){if(pressed)wake();}else if(activate)dark();}
static LRESULT CALLBACK curtain(HWND w,UINT m,WPARAM a,LPARAM b){switch(m){case WM_MOUSEACTIVATE:return MA_NOACTIVATE;case WM_SETCURSOR:SetCursor(NULL);return TRUE;case WM_LBUTTONUP:case WM_POINTERUP:if(blank)wake();return 0;case WM_DISPLAYCHANGE:if(blank)bounds();return 0;}return DefWindowProcW(w,m,a,b);}
static LRESULT CALLBACK settings(HWND w,UINT m,WPARAM a,LPARAM b){switch(m){case WM_TIMER:poll();return 0;case WM_COMMAND:switch(LOWORD(a)){case 1:learning=1;slot=-1;learned=0;SetWindowTextW(status,L"Hold your new shortcut, then release all buttons.");break;case 2:ShowWindow(w,SW_MINIMIZE);break;case 3:dark();break;}return 0;case WM_DESTROY:if(blank)wake();PostQuitMessage(0);return 0;}return DefWindowProcW(w,m,a,b);}
static HWND control(WCHAR *type,WCHAR *label,int id,int x,int y,int width){HWND h=CreateWindowW(type,label,WS_CHILD|WS_VISIBLE,x,y,width,35,mainwin,(HMENU)(INT_PTR)id,NULL,NULL);return h;}
int WINAPI WinMain(HINSTANCE instance,HINSTANCE old,LPSTR args,int show){
 CreateMutexW(NULL,FALSE,L"Local\\8BitTrade.LegionLock");if(GetLastError()==ERROR_ALREADY_EXISTS)return 0;
 SetProcessDPIAware();WNDCLASSW c={0};c.hInstance=instance;c.lpfnWndProc=curtain;c.hbrBackground=GetStockObject(BLACK_BRUSH);c.lpszClassName=L"LegionLockCurtain";RegisterClassW(&c);
 cover=CreateWindowExW(WS_EX_TOPMOST|WS_EX_NOACTIVATE|WS_EX_TOOLWINDOW,c.lpszClassName,L"",WS_POPUP,0,0,0,0,NULL,NULL,instance,NULL);
 c.lpfnWndProc=settings;c.hbrBackground=(HBRUSH)(COLOR_BTNFACE+1);c.hCursor=LoadCursor(NULL,IDC_ARROW);c.lpszClassName=L"LegionLockSettings";RegisterClassW(&c);
 mainwin=CreateWindowW(c.lpszClassName,L"LegionLock  |  8BitTrade",WS_OVERLAPPED|WS_SYSMENU|WS_MINIMIZEBOX|WS_CAPTION,CW_USEDEFAULT,CW_USEDEFAULT,555,215,NULL,NULL,instance,NULL);
 if(!mainwin||!cover)return 1;
 DWORD size=4,value;if(!RegGetValueW(HKEY_CURRENT_USER,key,L"Shortcut",RRF_RT_REG_DWORD,NULL,&value,&size)&&value&&value<=0x3ffff)shortcut=value;
 binding=control(L"STATIC",L"",0,18,18,510);describe();status=control(L"STATIC",L"Ready. Tap the screen or press a controller button to wake.",0,18,56,510);
 control(L"BUTTON",L"Change shortcut",1,18,110,158);control(L"BUTTON",L"Minimize",2,185,110,140);control(L"BUTTON",L"Blank screen",3,335,110,170);
 HFONT font=CreateFontW(-17,0,0,0,FW_NORMAL,0,0,0,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,0,L"Segoe UI");for(HWND h=GetWindow(mainwin,GW_CHILD);h;h=GetWindow(h,GW_HWNDNEXT))SendMessageW(h,WM_SETFONT,(WPARAM)font,TRUE);
 ShowWindow(mainwin,show);SetTimer(mainwin,1,25,NULL);MSG msg;while(GetMessageW(&msg,NULL,0,0)>0){TranslateMessage(&msg);DispatchMessageW(&msg);}return 0;}
