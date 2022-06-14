
#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <stdio.h>
#include "ConfigFile.h"

// C'tor
ConfigFile::ConfigFile() : action_count( 0 )
{
	for ( int i = 0; i < AXIS_COUNT; ++i )
	{
		invert[ i ] = false;
	}

	for ( int iaction = 0; iaction < ACTION_LIMIT; ++iaction )
	{
		actions[ iaction ].mode = AAM_NOP;
		actions[ iaction ].a = AXIS_COUNT;
		actions[ iaction ].b = AXIS_COUNT;
		actions[ iaction ].invert = false;
	}
}

static void write_axis( FILE* fp, const Axis axis )
{
	if ( axis == AXIS_X ) {
		fprintf( fp, "X" );
	} else if ( axis == AXIS_Y ) {
		fprintf( fp, "Y" );
	} else if ( axis == AXIS_Z ) {
		fprintf( fp, "Z" );
	} else if ( axis == AXIS_RX ) {
		fprintf( fp, "RX" );
	} else if ( axis == AXIS_RY ) {
		fprintf( fp, "RY" );
	} else if ( axis == AXIS_RZ ) {
		fprintf( fp, "RZ" );
	} else if ( axis == AXIS_S0 ) {
		fprintf( fp, "S0" );
	}
}

static Axis scan_axis( const char* buf )
{
	if ( buf[ 0 ] == 'X' )
	{
		return AXIS_X;
	}
	else if ( buf[ 0 ] == 'Y' )
	{
		return AXIS_Y;
	}
	else if ( buf[ 0 ] == 'Z' )
	{
		return AXIS_Z;
	}
	else if ( buf[ 0 ] == 'R' )
	{
		if ( buf[ 1 ] == 'X' )
		{
			return AXIS_RX;
		}
		else if ( buf[ 1 ] == 'Y' )
		{
			return AXIS_RY;
		} 
		else if ( buf[ 1 ] == 'Z' )
		{
			return AXIS_RZ;
		}
	}
	else if ( buf[ 0 ] == 'S' )
	{
		if ( buf[ 1 ] == '0' ) {
			return AXIS_S0;
		}
	}
	
	// Failed.
	return AXIS_COUNT;
}

static const char* skip_underscore( const char* buf )
{
	while (*buf)
	{
		const char c = *buf++;
		if ( c == '_' ) {
			return buf;
		}
	}
	return 0;
}

static const char* skip_to( const char* buf )
{
	const char* p = strstr( buf, "_TO_" );

	if ( p ) {
		return p + 4;
	} else {
		return 0;
	}
}

static const char* skip_as( const char* buf )
{
	const char* p = strstr( buf, "_AS_" );

	if ( p ) {
		return p + 4;
	} else {
		return 0;
	}
}

static const char* skip_and( const char* buf )
{
	const char* p = strstr( buf, "_AND_" );

	if ( p ) {
		return p + 5;
	} else {
		return 0;
	}
}

static bool has_inv( const char* buf )
{
	const char* p = strstr( buf, "_INV" );

	if ( p ) {
		return true;
	} else {
		return false;
	}
}

// Read
void ConfigFile::Read( const char* p_file )
{
	for ( int i = 0; i < AXIS_COUNT; ++i )
	{
		invert[ i ] = false;
	}
	
	action_count = 0;

	FILE* fp = fopen( p_file, "rb" );
	if ( fp )
	{
		int cnt = 0;
		char buf[ 1024 ];


		while( action_count < ACTION_LIMIT )
		{
			char c;
			c = fgetc( fp );

			if ( c == '\r' || c == '\n' || c == '\t' )
			{
				if ( cnt == 0 ) {
					continue;
				} else {
					break;
				}
			}
			else if ( c == EOF || c < 32 || cnt >= 512 )
			{
				break;
			}
			else if ( c == ';' )
			{
				// command
				buf[ cnt ] = 0;

				if ( strncmp( buf, "INV_", 4 ) == 0 )
				{
					// decode
					const char* b = buf + 4;
					Axis a = scan_axis( b );
					if ( a != AXIS_COUNT ) {
						invert[ a ] = true;
					}
				}
				if ( strncmp( buf, "MOVE_", 5 ) == 0 )
				{
					// decode
					AxisAction& action = actions[ action_count ];
					const char* b = buf + 5;
					action.a = scan_axis( b );
					if ( action.a != AXIS_COUNT ) {
						b = skip_to( b );
						if ( b ) {
							action.b = scan_axis( b );
							if ( ( action.b != AXIS_COUNT ) && ( action.b != action.a ) )
							{
								action.mode = AAM_MOVE;
								action.invert = has_inv( b );

								++action_count;
							}
						}
					}
				}
				else if ( strncmp( buf, "REMAP_", 6 ) == 0 )
				{
					// decode
					AxisAction& action = actions[ action_count ];
					const char* b = buf + 6;
					action.a = scan_axis( b );
					if ( action.a != AXIS_COUNT ) {
						b = skip_to( b );
						if ( b ) {
							action.b = scan_axis( b );
							if ( ( action.b != AXIS_COUNT ) && ( action.b != action.a ) )
							{
								action.mode = AAM_REMAP;
								action.invert = has_inv( b );

								++action_count;
							}
						}
					}
				}
				else if ( strncmp( buf, "SWAP_", 5 ) == 0 )
				{
					// decode
					AxisAction& action = actions[ action_count ];
					const char* b = buf + 5;
					action.a = scan_axis( b );
					if ( action.a != AXIS_COUNT ) {
						b = skip_and( b );
						if ( b ) {
							action.b = scan_axis( b );
							if ( ( action.b != AXIS_COUNT ) && ( action.b != action.a ) )
							{
								action.mode = AAM_SWAP;
								action.invert = false;

								++action_count;
							}
						}
					}
				}

				// reset
				cnt = 0;
			}
			else
			{
				buf[ cnt++ ] = c;
			}
		}

		fclose( fp );
	}
}

// Write
void ConfigFile::Write( const char* p_file )
{
	FILE* fp = fopen( p_file, "wb" );
	if ( fp )
	{
		fprintf( fp, "BEGIN;\n" );

		for ( int i = 0; i < AXIS_COUNT; ++i )
		{
			if ( invert[ i ] )
			{
				fprintf( fp, "INV_" );
				write_axis( fp, (Axis)i );
				fprintf( fp, ";\n" );
			}
		}

		for ( int i = 0; i < action_count; ++i )
		{
			const AxisAction& action = actions[ i ];
			if ( action.mode == AAM_MOVE && action.a < AXIS_COUNT && action.b < AXIS_COUNT )
			{
				fprintf( fp, "MOVE_" );

				write_axis( fp, action.a );

				fprintf( fp, "_TO_" );

				write_axis( fp, action.b );

				if ( action.invert ) {
					fprintf( fp, "_INV;\n" );
				} else {
					fprintf( fp, ";\n" );
				}
			}
			else if ( action.mode == AAM_SWAP && action.a < AXIS_COUNT && action.b < AXIS_COUNT )
			{
				fprintf( fp, "SWAP_" );

				write_axis( fp, action.a );

				fprintf( fp, "_AND_" );

				write_axis( fp, action.b );

				fprintf( fp, ";\n" );
			}
			else if ( action.mode == AAM_REMAP && action.a < AXIS_COUNT && action.b < AXIS_COUNT )
			{
				fprintf( fp, "REMAP_" );

				write_axis( fp, action.a );

				fprintf( fp, "_TO_" );

				write_axis( fp, action.b );

				fprintf( fp, ";\n" );
			}
		}

		fprintf( fp, "END;\n" );
		fclose( fp );
	}
}

