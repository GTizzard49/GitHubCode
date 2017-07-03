#include <windows.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <dxerr.h>
#define _XM_NO_INTRINSICS_
#define XM_NO_ALIGNMENT
#include <xnamath.h>
#include <dinput.h>


class Input
{
private:
	HINSTANCE g_hInst;
	HWND g_hWnd;

	// pointer to the directinput interface
	IDirectInput8* g_direct_input;
	// pointer to the keyboard
	IDirectInputDevice8* g_keyboard_device;
	// stores the state of the keyboard
	unsigned char g_keyboard_keys_state[256]; 



public:
	Input(HINSTANCE Inst, HWND Wnd);
	HRESULT InitInput();
	void ReadInputStates();
	bool IsKeyPressed(unsigned char DI_keycode);

	~Input();
};

