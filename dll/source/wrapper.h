
#ifndef __WRAPPER_HDR__
#define __WRAPPER_HDR__

#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <assert.h>
#include <list>
#include <vector>

#include "../../dinput_tweaks/source/ConfigFile.h"

struct sLongPair
{
	LONG min;
	LONG max;
};

#ifdef WRAPPER8

// API 8

#define DIRECTINPUT_VERSION 0x0800
#include "dinput.h"
#define DEVICE_BASE_CLASS IDirectInputDevice8A
#define LPDEVICE_CLASS LPDIRECTINPUTDEVICE8A
#define OBJECT_BASE_CLASS IDirectInput8A
typedef HRESULT ( WINAPI * DIRECTINPUT8CREATEPROC )( HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID *ppvOut, LPUNKNOWN punkOuter );

#else // WRAPPER8

// API 7

#define DIRECTINPUT_VERSION 0x0700
#include "dinput.h"
#define DEVICE_BASE_CLASS IDirectInputDevice7A
#define LPDEVICE_CLASS LPDIRECTINPUTDEVICEA
#define OBJECT_BASE_CLASS IDirectInput7A
typedef HRESULT ( WINAPI * DIRECTINPUTCREATEAPROC )( HINSTANCE hinst, DWORD dwVersion, LPDIRECTINPUTA * ppDI, LPUNKNOWN punkOuter );

#endif // WRAPPER8





// Device Wrapper

class cDirectInputDevice : public DEVICE_BASE_CLASS
{

public:

	cDirectInputDevice();

	~cDirectInputDevice();

	DEVICE_BASE_CLASS** GetBasePtr()
	{
		return &m_p_base;
	}

	// IUnknown
	virtual HRESULT WINAPI QueryInterface( const IID & r_iid, LPVOID * p_obj );
	virtual ULONG WINAPI AddRef();
	virtual ULONG WINAPI Release();

	// v2
	virtual HRESULT WINAPI GetCapabilities( LPDIDEVCAPS dev_caps );
	virtual HRESULT WINAPI EnumObjects( LPDIENUMDEVICEOBJECTSCALLBACKA callback, LPVOID cb_userdata, DWORD flags );
	virtual HRESULT WINAPI GetProperty( const IID & r_iid, LPDIPROPHEADER dip );
	virtual HRESULT WINAPI SetProperty( const IID & r_iid, LPCDIPROPHEADER dip );
	virtual HRESULT WINAPI Acquire( );
	virtual HRESULT WINAPI Unacquire( );
	virtual HRESULT WINAPI GetDeviceState( DWORD buf_size, LPVOID buf_data );
	virtual HRESULT WINAPI GetDeviceData( DWORD buf_size, LPDIDEVICEOBJECTDATA buf_data, LPDWORD out_size, DWORD flags );
	virtual HRESULT WINAPI SetDataFormat( LPCDIDATAFORMAT data_format );
	virtual HRESULT WINAPI SetEventNotification( HANDLE h_event );
	virtual HRESULT WINAPI SetCooperativeLevel( HWND h_wnd, DWORD level );
	virtual HRESULT WINAPI GetObjectInfo( LPDIDEVICEOBJECTINSTANCEA obj_inst, DWORD obj, DWORD how );
	virtual HRESULT WINAPI GetDeviceInfo( LPDIDEVICEINSTANCEA info );
	virtual HRESULT WINAPI RunControlPanel( HWND h_wnd, DWORD flags );
	virtual HRESULT WINAPI Initialize( HINSTANCE inst, DWORD version, const IID & r_iid );
	virtual HRESULT WINAPI CreateEffect( const IID & r_iid, LPCDIEFFECT fx, LPDIRECTINPUTEFFECT * di_fx, LPUNKNOWN p_unk );
	virtual HRESULT WINAPI EnumEffects( LPDIENUMEFFECTSCALLBACKA callback, LPVOID cb_userdata, DWORD fx_type );
	virtual HRESULT WINAPI GetEffectInfo( LPDIEFFECTINFOA fx, const IID & r_iid );
	virtual HRESULT WINAPI GetForceFeedbackState( LPDWORD out );
	virtual HRESULT WINAPI SendForceFeedbackCommand( DWORD flags );
	virtual HRESULT WINAPI EnumCreatedEffectObjects( LPDIENUMCREATEDEFFECTOBJECTSCALLBACK callback, LPVOID cb_userdata, DWORD flags );
	virtual HRESULT WINAPI Escape( LPDIEFFESCAPE esc );
	virtual HRESULT WINAPI Poll( );
	virtual HRESULT WINAPI SendDeviceData( DWORD buf_size, LPCDIDEVICEOBJECTDATA buf_data, LPDWORD out_size, DWORD flags );

	// API 7
	virtual HRESULT WINAPI EnumEffectsInFile( LPCSTR file_name, LPDIENUMEFFECTSINFILECALLBACK callback, LPVOID cb_userdata, DWORD flags );
	virtual HRESULT WINAPI WriteEffectToFile( LPCSTR file_name, DWORD count, LPDIFILEEFFECT data, DWORD flags );

#ifdef WRAPPER8
	// API 8
	virtual HRESULT WINAPI BuildActionMap( LPDIACTIONFORMATA action_info, LPCSTR user_name, DWORD flags );
	virtual HRESULT WINAPI SetActionMap( LPDIACTIONFORMATA action_info, LPCSTR user_name, DWORD flags );
	virtual HRESULT WINAPI GetImageInfo( LPDIDEVICEIMAGEINFOHEADERA image_info );
#endif // WRAPPER8


private:

	ULONG m_ref_count;
	
	DEVICE_BASE_CLASS* m_p_base;

	sLongPair m_range[ AXIS_COUNT ];
	bool m_ranges_default;

};

typedef std::list< cDirectInputDevice* > tDirectInputDeviceList;


// Direct Input Wrapper

class cDirectInput
{

public:

	cDirectInput();
	
	~cDirectInput();

	OBJECT_BASE_CLASS** GetBasePtr()
	{
		return &m_p_base;
	}

	// IUnknown
	virtual HRESULT WINAPI QueryInterface( const IID & r_iid, LPVOID * p_obj );
	virtual ULONG WINAPI AddRef();
	virtual ULONG WINAPI Release();

	// API 7
	virtual HRESULT WINAPI CreateDevice( const GUID& r_guid, cDirectInputDevice** di_device, LPUNKNOWN p_unk );
	static BOOL	CALLBACK EnumDevicesCallback( LPCDIDEVICEINSTANCEA dev_inst, LPVOID cb_userdata ); // helper
	virtual HRESULT WINAPI EnumDevices( DWORD dev_type, LPDIENUMDEVICESCALLBACKA callback, LPVOID cb_userdata, DWORD flags );
	virtual HRESULT WINAPI GetDeviceStatus( const GUID& r_guid );
	virtual HRESULT WINAPI RunControlPanel( HWND win_handle, DWORD flags );
	virtual HRESULT WINAPI Initialize( HINSTANCE inst_handle, DWORD version );
	virtual HRESULT WINAPI FindDevice( const GUID& r_guid, LPCSTR dev_name, LPGUID dev_guid );

#ifdef WRAPPER8
	// API 8
	virtual HRESULT WINAPI EnumDevicesBySemantics( LPCSTR user_name, LPDIACTIONFORMATA action_format, LPDIENUMDEVICESBYSEMANTICSCBA callback, LPVOID cb_userdata, DWORD flags );
	virtual HRESULT WINAPI ConfigureDevices( LPDICONFIGUREDEVICESCALLBACK callback, LPDICONFIGUREDEVICESPARAMSA device_config, DWORD flags, LPVOID ref_data );
#endif // WRAPPER8

private:

	ULONG m_ref_count;
	
	OBJECT_BASE_CLASS* m_p_base;
	
	tDirectInputDeviceList m_devices;

};

typedef std::list< cDirectInput* > tDirectInputList;




// DLL base

class DLL
{

public:

#ifdef WRAPPER8
	static DIRECTINPUT8CREATEPROC fnDirectInput8Create;
#else // WRAPPER8
	static DIRECTINPUTCREATEAPROC fnDirectInputCreateA;
#endif // WRAPPER8

	static bool Init( HANDLE mod_hnd );
	
	static void Shutdown( );

	static cDirectInput * CreateDirectInput();

	static ConfigFile Config;

private:

	static LRESULT CALLBACK WindowHookFuncGetMessage( int nCode, WPARAM wParam, LPARAM lParam );
	static LRESULT CALLBACK WindowHookFuncCallWnd( int nCode, WPARAM wParam, LPARAM lParam );

	static HANDLE hModule;

	static HMODULE hDLL;
	
	static tDirectInputList ms_direct_input_list;

};

#endif
