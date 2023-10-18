#pragma once
namespace MyNetwork
{
	enum sector
	{
		X_DEFAULT = 30000,
		Y_DEFAULT = 30000
	};

	struct SectorPos
	{
		short _xPos;
		short _yPos;

		SectorPos() : _xPos(X_DEFAULT), _yPos(Y_DEFAULT)
		{}

		SectorPos(short x, short y) : _xPos(X_DEFAULT), _yPos(Y_DEFAULT)
		{
			_xPos = x;
			_yPos = y;
		}

		~SectorPos() {}
	};

	struct SectorAround
	{
		char _count = 0;
		SectorPos _around[9];

	};

}
