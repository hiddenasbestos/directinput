
//
// Dependencies
//

#include "wrapper.h"

typedef std::list< DIDEVICEINSTANCEA > tDeviceInstanceList;
typedef std::vector< DIDEVICEOBJECTINSTANCEA > tDeviceObjectInstanceList;


//
// Globals
//

ConfigFile							DLL::Config;

#ifdef WRAPPER8
DIRECTINPUT8CREATEPROC				DLL::fnDirectInput8Create;
#else // WRAPPER8
DIRECTINPUTCREATEAPROC				DLL::fnDirectInputCreateA;
#endif // WRAPPER8

HMODULE								DLL::hDLL;
tDirectInputList					DLL::ms_direct_input_list;
HANDLE								DLL::hModule;

GUID GUID_AXIS[ AXIS_COUNT ] =
{
	{ 0xA36D02E0, 0xC9F3, 0x11CF, 0xBF, 0xC7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00 }, // "X Axis"
	{ 0xA36D02E1, 0xC9F3, 0x11CF, 0xBF, 0xC7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00 }, // "Y Axis"
	{ 0xA36D02E2, 0xC9F3, 0x11CF, 0xBF, 0xC7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00 }, // "Z Axis"
	{ 0xA36D02F4, 0xC9F3, 0x11CF, 0xBF, 0xC7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00 }, // "X Rotation"
	{ 0xA36D02F5, 0xC9F3, 0x11CF, 0xBF, 0xC7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00 }, // "Y Rotation"
	{ 0xA36D02E3, 0xC9F3, 0x11CF, 0xBF, 0xC7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00 }, // "Z Rotation"
	{ 0xA36D02E4, 0xC9F3, 0x11CF, 0xBF, 0xC7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00 } // Slider 0
};

WORD AxisUsage[ AXIS_COUNT ] =
{
	0x30,		// HID_USAGE_GENERIC_X	
	0x31,		// HID_USAGE_GENERIC_Y	
	0x32,		// HID_USAGE_GENERIC_Z	
	0x33,		// HID_USAGE_GENERIC_RX
	0x34,		// HID_USAGE_GENERIC_RY
	0x35,		// HID_USAGE_GENERIC_RZ
	0x36,		// HID_USAGE_GENERIC_SLIDER
};

//
// DLL exports
//

extern "C"
{
	BOOL WINAPI DllMain( HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved )
	{
		switch ( ul_reason_for_call )
		{

		case DLL_PROCESS_ATTACH:
			if( !DLL::Init( hModule ) ) {
				return FALSE;
			}
			break;

		case DLL_PROCESS_DETACH:
			DLL::Shutdown( );
			break;
		
		}

		return TRUE;
	}

#ifdef WRAPPER8
	
	HRESULT WINAPI DirectInput8Create( HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, cDirectInput** object, LPUNKNOWN punkOuter )
	{
		*object = DLL::CreateDirectInput();
		return DLL::fnDirectInput8Create( hinst, dwVersion, riidltf, ( LPVOID * )( *object )->GetBasePtr(), punkOuter );
	};

#else // WRAPPER8

	HRESULT WINAPI DirectInputCreateA( HINSTANCE inst_handle, DWORD version, cDirectInput** object, LPUNKNOWN p_unk )
	{
		*object = DLL::CreateDirectInput();
		return DLL::fnDirectInputCreateA( inst_handle, version, ( LPDIRECTINPUTA * )( *object )->GetBasePtr(), p_unk );
	};

#endif // WRAPPER8

}


//
// DLL
//

bool DLL::Init( HANDLE _hModule )
{
	// Do something to require USER32.DLL (fixes X3:Reunion)
	IsCharAlphaA( 'A' );

	// read config
	Config.Read( "dinput.cfg" );

	// Make path to actual DirectInput DLL
	char dll_name[ MAX_PATH ];
	GetSystemDirectoryA( dll_name, MAX_PATH );
#ifdef WRAPPER8	
	strcat( dll_name, "\\dinput8.dll" );
#else // WRAPPER8
	strcat( dll_name, "\\dinput.dll" );
#endif // WRAPPER8

	// Load DLL
	hDLL = LoadLibraryA( dll_name );
	if ( hDLL > ( HMODULE )31 )
	{
		// Acquire DirectInput factory
#ifdef WRAPPER8
		fnDirectInput8Create = ( DIRECTINPUT8CREATEPROC )GetProcAddress( hDLL, "DirectInput8Create" );
		if ( fnDirectInput8Create )
#else // WRAPPER8
		fnDirectInputCreateA = ( DIRECTINPUTCREATEAPROC )GetProcAddress( hDLL, "DirectInputCreateA" );
		if ( fnDirectInputCreateA )
#endif // WRAPPER8
		{
			hModule = _hModule;
			return true;
		}
		else
		{
			Shutdown( );
			return false;
		}
	}

	return false;
}

void DLL::Shutdown()
{
	// tidy up
	for ( tDirectInputList::iterator wrpItr = ms_direct_input_list.begin( ); wrpItr != ms_direct_input_list.end( ); wrpItr++ )
	{
		delete *wrpItr;
	}

	FreeLibrary( hDLL );
}

cDirectInput* DLL::CreateDirectInput()
{
	cDirectInput* p = new cDirectInput( );
	ms_direct_input_list.push_back( p );
	return p;
}




//
// cDirectInput
//

cDirectInput::cDirectInput() :	m_ref_count( 0 ),
								m_p_base( NULL )
{
	//
}

cDirectInput::~cDirectInput()
{
	// Tidy up
	for( tDirectInputDeviceList::iterator devItr = m_devices.begin( ); devItr != m_devices.end( ); devItr++ )
	{
		delete* devItr;
	}

	m_p_base->Release( );
}

BOOL cDirectInput::EnumDevicesCallback( LPCDIDEVICEINSTANCEA dev_inst, LPVOID cb_userdata )
{
	tDeviceInstanceList* devices = (tDeviceInstanceList*)cb_userdata;
	devices->push_back( *dev_inst );
	return TRUE;
}

HRESULT cDirectInput::QueryInterface( const IID& r_iid, LPVOID* p_obj )
{
	*p_obj = this;
	return S_OK;
}

ULONG cDirectInput::AddRef()
{
	return m_ref_count++;
}

ULONG cDirectInput::Release()
{
	return m_ref_count--;
}

HRESULT cDirectInput::CreateDevice( const GUID& r_guid, cDirectInputDevice** pp_device, LPUNKNOWN p_unk )
{
	*pp_device = new cDirectInputDevice();
	
	m_devices.push_back( *pp_device );

	HRESULT res;
	res = m_p_base->CreateDevice( r_guid, (LPDEVICE_CLASS*)(*pp_device)->GetBasePtr(), p_unk );

	return res;
}

HRESULT cDirectInput::EnumDevices( DWORD dev_type, LPDIENUMDEVICESCALLBACKA callback, LPVOID cb_userdata, DWORD flags )
{
	tDeviceInstanceList devices;
	m_p_base->EnumDevices( dev_type, EnumDevicesCallback, &devices, flags );

	for ( tDeviceInstanceList::iterator devItr = devices.begin(); devItr != devices.end( ); devItr++ )
	{
		callback( (LPCDIDEVICEINSTANCEA)&(*devItr), cb_userdata );
	}

	return S_OK;
}

HRESULT cDirectInput::GetDeviceStatus( const GUID& r_guid )
{
	return S_OK;
}

HRESULT cDirectInput::RunControlPanel( HWND win_handle, DWORD flags )
{
	return S_OK;
}

HRESULT cDirectInput::Initialize( HINSTANCE inst_handle, DWORD version )
{
	return S_OK;
}

HRESULT cDirectInput::FindDevice( const GUID & r_guid, LPCSTR dev_name, LPGUID dev_guid )
{
	return S_OK;
}

#ifdef WRAPPER8

HRESULT cDirectInput::EnumDevicesBySemantics( LPCSTR user_name, LPDIACTIONFORMATA action_format, LPDIENUMDEVICESBYSEMANTICSCBA callback, LPVOID cb_userdata, DWORD flags )
{
	return S_OK;
}

HRESULT cDirectInput::ConfigureDevices( LPDICONFIGUREDEVICESCALLBACK callback, LPDICONFIGUREDEVICESPARAMSA device_config, DWORD flags, LPVOID ref_data )
{
	return S_OK;
}

#endif // WRAPPER8


//
// cDirectInputDevice
//

cDirectInputDevice::cDirectInputDevice() :

	m_ref_count( 0 ),
	m_p_base( NULL )
{
	//

	for ( int i = 0; i < AXIS_COUNT; ++i )
	{
		m_range[ i ].min = -65535;
		m_range[ i ].max = 65535;
	}

	m_ranges_default = true;

}

cDirectInputDevice::~cDirectInputDevice()
{
	m_p_base->Release();
}

HRESULT cDirectInputDevice::QueryInterface( const IID & r_iid, LPVOID * p_obj )
{
	*p_obj = this;
	return S_OK;
}

ULONG cDirectInputDevice::AddRef()
{
	return m_ref_count++;
}

ULONG cDirectInputDevice::Release()
{
	return m_ref_count--;
}

HRESULT cDirectInputDevice::GetCapabilities( LPDIDEVCAPS dev_caps )
{
	return m_p_base->GetCapabilities( dev_caps );
}

static BOOL PASCAL custom_object_callback( LPCDIDEVICEOBJECTINSTANCEA p_instance, LPVOID p_userdata )
{
	DIDEVICEOBJECTINSTANCEA object;
	object = *p_instance;

	// Invent missing axis?
	for ( int iaction = 0; iaction < DLL::Config.action_count; ++iaction )
	{
		const AxisAction& action = DLL::Config.actions[ iaction ];

		if ( action.mode == AAM_REMAP )
		{
			if ( object.guidType == GUID_AXIS[ action.a ] )
			{
				// Reconfigure to match the target axis
				object.guidType = GUID_AXIS[ action.b ];
				object.dwOfs = action.b * sizeof( LONG );
				object.wUsage = AxisUsage[ action.b ];

				break;
			}
			// We have the target axis as well? We're not creating it?
			else if ( object.guidType == GUID_AXIS[ action.b ] )
			{
				// ignore it ...
				return true;
			}
		}
	}

	((tDeviceObjectInstanceList*)p_userdata)->push_back( object );
	return true;
}

HRESULT cDirectInputDevice::EnumObjects( LPDIENUMDEVICEOBJECTSCALLBACKA callback, LPVOID cb_userdata, DWORD flags )
{
	HRESULT hres;
	
	// Acquire the objects for ourselves
	tDeviceObjectInstanceList objects;
	hres = m_p_base->EnumObjects( custom_object_callback, &objects, flags );

	if ( hres == S_OK )
	{
		// Sort into ascending dwOfs.
		for ( size_t i = 0; i < objects.size() - 1; ++i )
		for ( size_t j = i + 1; j < objects.size(); ++j )
		{
			if ( objects[ i ].dwOfs > objects[ j ].dwOfs )
			{
				DIDEVICEOBJECTINSTANCEA t;
				t = objects[ i ];
				objects[ i ] = objects[ j ];
				objects[ j ] = t;
			}
		}

		// Now pass the modified list to the game.
		for ( tDeviceObjectInstanceList::iterator it = objects.begin(); it != objects.end(); it++ )
		{
			(LPDIENUMDEVICEOBJECTSCALLBACKA)(*callback)( &( *it ), cb_userdata );
		}
	}

	return hres;
}

HRESULT cDirectInputDevice::GetProperty( const IID & r_iid, LPDIPROPHEADER dip )
{
	HRESULT hres;
	hres = m_p_base->GetProperty( r_iid, dip );

	return hres;
}

HRESULT cDirectInputDevice::SetProperty( const IID & r_iid, LPCDIPROPHEADER dip )
{
	HRESULT hres;
	hres = m_p_base->SetProperty( r_iid, dip );

	// Configure the range?
	if ( dip->dwSize == sizeof( DIPROPRANGE ) && dip->dwHow == 1 )
	{
		// store range update.
		DIPROPRANGE rng;
		rng = *(LPDIPROPRANGE)( dip );

		// Axis
		const int axis = dip->dwObj / sizeof( LONG );
		if ( axis < AXIS_COUNT )
		{
			m_range[ axis ].min = rng.lMin;
			m_range[ axis ].max = rng.lMax;
		}
	}

	return hres;
}

HRESULT cDirectInputDevice::Acquire()
{
	if ( m_ranges_default )
	{
		// for each axis
		for ( int iaxis = 0; iaxis < AXIS_COUNT; ++iaxis )
		{
			DIPROPRANGE rng;
			rng.diph.dwHeaderSize = sizeof( DIPROPHEADER );
			rng.diph.dwHow = 1;
			rng.diph.dwObj = iaxis * sizeof( LONG );
			rng.diph.dwSize = sizeof( DIPROPRANGE );
		
			HRESULT hres;
			hres = m_p_base->GetProperty( DIPROP_RANGE, &rng.diph );
			if ( hres == S_OK )
			{
				m_range[ iaxis ].min = rng.lMin;
				m_range[ iaxis ].max = rng.lMax;
			}
		}

		m_ranges_default = false;
	}
	
	return m_p_base->Acquire();
}

HRESULT cDirectInputDevice::Unacquire()
{
	return m_p_base->Unacquire();
}

static void swap_long( LONG* map, int a, int b )
{
	LONG t;
	t = map[ a ];
	map[ a ] = map[ b ];
	map[ b ] = t;
}

static LONG remap_range( LONG value, const sLongPair& src, const sLongPair& dst )
{
	const __int64 src_d = value - src.min;
	const __int64 src_r = src.max - src.min;
	const __int64 dst_r = dst.max - dst.min;
	const __int64 new_d = ( src_d * dst_r ) / src_r;
	const LONG new_v = (LONG)( new_d + dst.min );

	return new_v;
}

static void centre_axis( LONG* map, int src, const sLongPair& src_rng )
{
	// Centre axis
	map[ src ] = src_rng.min + ( src_rng.max - src_rng.min ) / 2;
}

static void remap_axis( LONG* map, 
						int src, 
						const sLongPair& src_rng, 
						int dst,
						const sLongPair& dst_rng,
						bool invert )
{
	// Convert value to target axis range
	LONG src_fix = remap_range( map[ src ], src_rng, dst_rng );

	if ( invert )
	{
		// Invert -> Copy
		map[ dst ] = dst_rng.max - ( src_fix - dst_rng.min );
	}
	else
	{
		// Copy
		map[ dst ] = src_fix;
	}

	// Destroy original axis reading.
	centre_axis( map, src, src_rng );
}

static void swap_axis( LONG* map, 
					   int a, 
					   const sLongPair& a_rng, 
					   int b,
					   const sLongPair& b_rng )
{
	// Convert values to target axis range
	LONG a_fix = remap_range( map[ a ], a_rng, b_rng );
	LONG b_fix = remap_range( map[ b ], b_rng, a_rng );

	// Swap on copy
	map[ a ] = b_fix;
	map[ b ] = a_fix;
}

HRESULT cDirectInputDevice::GetDeviceState( DWORD buf_size, LPVOID buf_data )
{
	HRESULT res;
	res = m_p_base->GetDeviceState( buf_size, buf_data );

	if ( buf_size == sizeof( DIJOYSTATE ) )
	{
		// Alias as an array of LONG's.
		LONG* remap = (LONG*)buf_data;

		// Run Actions
		for ( int iaction = 0; iaction < DLL::Config.action_count; ++iaction )
		{
			const AxisAction& action = DLL::Config.actions[ iaction ];
			
			switch ( action.mode )
			{

			case AAM_MOVE:
				remap_axis( remap, action.a, m_range[ action.a ], action.b, m_range[ action.b ], action.invert );
				break;

			case AAM_SWAP:
				swap_axis( remap, action.a, m_range[ action.a ], action.b, m_range[ action.b ] );
				break;

			}
		}

	}

	return res;
}

HRESULT cDirectInputDevice::GetDeviceData( DWORD buf_size, LPDIDEVICEOBJECTDATA buf_data, LPDWORD out_size, DWORD flags )
{
	return m_p_base->GetDeviceData( buf_size, buf_data, out_size, flags );
}

HRESULT cDirectInputDevice::SetDataFormat( LPCDIDATAFORMAT data_format )
{
	return m_p_base->SetDataFormat( data_format );
}

HRESULT cDirectInputDevice::SetEventNotification( HANDLE h_event )
{
	return m_p_base->SetEventNotification( h_event );
}

HRESULT cDirectInputDevice::SetCooperativeLevel( HWND h_wnd, DWORD level )
{
	return m_p_base->SetCooperativeLevel( h_wnd, level );
}

HRESULT cDirectInputDevice::GetObjectInfo( LPDIDEVICEOBJECTINSTANCEA obj_inst, DWORD obj, DWORD how )
{
	return m_p_base->GetObjectInfo( obj_inst, obj, how );
}

HRESULT cDirectInputDevice::GetDeviceInfo( LPDIDEVICEINSTANCEA info )
{
	return m_p_base->GetDeviceInfo( info );
}

HRESULT cDirectInputDevice::RunControlPanel( HWND h_wnd, DWORD flags )
{
	return m_p_base->RunControlPanel( h_wnd, flags );
}

HRESULT cDirectInputDevice::Initialize( HINSTANCE inst, DWORD version, const IID & r_iid )
{
	return m_p_base->Initialize( inst, version, r_iid );
}

HRESULT cDirectInputDevice::CreateEffect( const IID & r_iid, LPCDIEFFECT fx, LPDIRECTINPUTEFFECT * di_fx, LPUNKNOWN p_unk )
{
	return m_p_base->CreateEffect( r_iid, fx, di_fx, p_unk );
}

HRESULT cDirectInputDevice::EnumEffects( LPDIENUMEFFECTSCALLBACKA callback, LPVOID cb_userdata, DWORD fx_type )
{
	return m_p_base->EnumEffects( callback, cb_userdata, fx_type );
}

HRESULT cDirectInputDevice::GetEffectInfo( LPDIEFFECTINFOA fx, const IID & r_iid )
{
	return m_p_base->GetEffectInfo( fx, r_iid );
}

HRESULT cDirectInputDevice::GetForceFeedbackState( LPDWORD out )
{
	return m_p_base->GetForceFeedbackState( out );
}

HRESULT cDirectInputDevice::SendForceFeedbackCommand( DWORD flags )
{
	return m_p_base->SendForceFeedbackCommand( flags );
}

HRESULT cDirectInputDevice::EnumCreatedEffectObjects( LPDIENUMCREATEDEFFECTOBJECTSCALLBACK callback, LPVOID cb_userdata, DWORD flags )
{
	return m_p_base->EnumCreatedEffectObjects( callback, cb_userdata, flags );
}

HRESULT cDirectInputDevice::Escape( LPDIEFFESCAPE esc )
{
	return m_p_base->Escape( esc );
}

HRESULT cDirectInputDevice::Poll()
{
	return m_p_base->Poll();
}

HRESULT cDirectInputDevice::SendDeviceData( DWORD buf_size, LPCDIDEVICEOBJECTDATA buf_data, LPDWORD out_size, DWORD flags )
{
	return m_p_base->SendDeviceData( buf_size, buf_data, out_size, flags );
}

HRESULT cDirectInputDevice::EnumEffectsInFile( LPCSTR file_name, LPDIENUMEFFECTSINFILECALLBACK callback, LPVOID cb_userdata, DWORD flags )
{
	return m_p_base->EnumEffectsInFile( file_name, callback, cb_userdata, flags );
}

HRESULT cDirectInputDevice::WriteEffectToFile( LPCSTR file_name, DWORD count, LPDIFILEEFFECT data, DWORD flags )
{
	return m_p_base->WriteEffectToFile( file_name, count, data, flags );
}

#ifdef WRAPPER8

HRESULT cDirectInputDevice::BuildActionMap( LPDIACTIONFORMATA action_info, LPCSTR user_name, DWORD flags )
{
	return m_p_base->BuildActionMap( action_info, user_name, flags );
}

HRESULT cDirectInputDevice::SetActionMap( LPDIACTIONFORMATA action_info, LPCSTR user_name, DWORD flags )
{
	return m_p_base->SetActionMap( action_info, user_name, flags );
}

HRESULT cDirectInputDevice::GetImageInfo( LPDIDEVICEIMAGEINFOHEADERA image_info )
{
	return m_p_base->GetImageInfo( image_info );
}

#endif // WRAPPER8

