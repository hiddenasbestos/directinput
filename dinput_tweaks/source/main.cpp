
#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <windowsx.h>
#include <stdio.h>

#include "resource.h"
#include "ConfigFile.h"

// TODO: Rewrite this to use the full capabilities of ConfigFile, rather than this sub-set.

struct DialogConfig
{
	ConfigFile dll_config;

	bool remap_s0_to_z;
	bool remap_zr_to_z;
	bool invert_z;
	bool invert_rz;
	bool invert_s0;

	void Read( const char* p_file )
	{
		// Read
		dll_config.Read( p_file );

		// Unpack
		invert_z = dll_config.invert[ AXIS_Z ];
		invert_rz = dll_config.invert[ AXIS_RZ ];
		invert_s0 = dll_config.invert[ AXIS_S0 ];

		remap_s0_to_z = false;
		remap_zr_to_z = false;

		for ( int i = 0; i < dll_config.action_count; ++i )
		{
			const AxisAction& action = dll_config.actions[ i ];
			
			// remap_s0_to_z ?
			if ( action.mode == AAM_REMAP && action.a == AXIS_S0 && action.b == AXIS_Z )
			{
				remap_s0_to_z = true;
			}

			// remap_zr_to_z ?
			if ( action.mode == AAM_REMAP && action.a == AXIS_RZ && action.b == AXIS_Z )
			{
				remap_zr_to_z = true;
			}
		}
	}

	void Write( const char* p_file )
	{
		// Pack
		dll_config.invert[ AXIS_Z ] = invert_z;
		dll_config.invert[ AXIS_RZ ] = invert_rz;
		dll_config.invert[ AXIS_S0 ] = invert_s0;

		dll_config.action_count = 0;

		if ( remap_s0_to_z )
		{
			AxisAction& action = dll_config.actions[ dll_config.action_count ];
			action.a = AXIS_S0;
			action.b = AXIS_Z;
			action.invert = false;
			action.mode = AAM_REMAP;
			++dll_config.action_count;
		}

		if ( remap_zr_to_z )
		{
			AxisAction& action = dll_config.actions[ dll_config.action_count ];
			action.a = AXIS_RZ;
			action.b = AXIS_Z;
			action.invert = false;
			action.mode = AAM_REMAP;
			++dll_config.action_count;
		}

		// Write
		dll_config.Write( p_file );
	}

};

static DialogConfig g_config;


static int CALLBACK dlgproc( HWND window_handle, UINT message, WPARAM wparam, LPARAM lparam )
{
	switch ( message )
	{

	case WM_INITDIALOG:

		// apply
		Button_SetCheck( GetDlgItem( window_handle, IDC_REMAP_S0_AS_Z ), g_config.remap_s0_to_z );
		Button_SetCheck( GetDlgItem( window_handle, IDC_REMAP_ZR_AS_Z ), g_config.remap_zr_to_z );
		Button_SetCheck( GetDlgItem( window_handle, IDC_INVERT_Z ), g_config.invert_z );
		Button_SetCheck( GetDlgItem( window_handle, IDC_INVERT_RZ ), g_config.invert_rz );
		Button_SetCheck( GetDlgItem( window_handle, IDC_INVERT_S0 ), g_config.invert_s0 );

		return 1;

	case WM_COMMAND:

		switch ( LOWORD( wparam ) )
		{

		case IDC_REMAP_S0_AS_Z:
			{
				int val = Button_GetCheck( GetDlgItem( window_handle, IDC_REMAP_S0_AS_Z ) );
				g_config.remap_s0_to_z = val ? true : false;
			}
			break;

		case IDC_REMAP_ZR_AS_Z:
			{
				int val = Button_GetCheck( GetDlgItem( window_handle, IDC_REMAP_ZR_AS_Z ) );
				g_config.remap_zr_to_z = val ? true : false;
			}
			break;

		case IDC_INVERT_Z:
			{
				int val = Button_GetCheck( GetDlgItem( window_handle, IDC_INVERT_Z ) );
				g_config.invert_z = val ? true : false;
			}
			break;

		case IDC_INVERT_RZ:
			{
				int val = Button_GetCheck( GetDlgItem( window_handle, IDC_INVERT_RZ ) );
				g_config.invert_rz = val ? true : false;
			}
			break;

		case IDC_INVERT_S0:
			{
				int val = Button_GetCheck( GetDlgItem( window_handle, IDC_INVERT_S0 ) );
				g_config.invert_s0 = val ? true : false;
			}
			break;

		case IDOK:
			EndDialog( window_handle, 1 );
			break;

		}

		return 1;

	default:
		break;

	}; // switch ( message )

	return 0;
}

int WINAPI WinMain( HINSTANCE, HINSTANCE, LPSTR, int )
{
	// read config
	g_config.Read( "dinput.cfg" );

	int dlg_result;
	dlg_result = DialogBox( GetModuleHandle( NULL ), MAKEINTRESOURCE( IDD_MAIN ), NULL, dlgproc );

	// write config
	g_config.Write( "dinput.cfg" );

	return 0;
}