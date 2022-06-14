
enum Axis
{
	AXIS_X,
	AXIS_Y,
	AXIS_Z,
	AXIS_RX,
	AXIS_RY,
	AXIS_RZ,
	AXIS_S0,

	AXIS_COUNT
};

enum AxisActionMode
{
	AAM_NOP,
	AAM_MOVE,
	AAM_REMAP,
	AAM_SWAP,

	AxisActionModeCount
};

enum { ACTION_LIMIT = 10 };

struct AxisAction
{
	AxisActionMode mode;
	Axis a;
	Axis b;
	bool invert;
};

struct ConfigFile
{
	int action_count;
	AxisAction actions[ ACTION_LIMIT ];
	bool invert[ AXIS_COUNT ];

	// Default
	ConfigFile();

	// Read
	void Read( const char* p_file );

	// Write
	void Write( const char* p_file );
};
