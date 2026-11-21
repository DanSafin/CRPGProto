#pragma once

#include <vector>
#include <iostream>

enum class TileType
{
	GROUND,
	OBSTACLE
};

struct Tile
{
	int x = 0;
	int y = 0;
	TileType type = TileType::GROUND;
	bool isOccupied = false;
};

class GridMap
{
private:
	int _width;
	int _height;
	std::vector<Tile> _tiles;

public:
	GridMap(int width, int height) 
		: _width(width), _height(height)
	{
		_tiles.resize(_width * _height);
		for (int y = 0; y < _height; ++y)
		{
			for (int x = 0; x < _width; ++x)
			{
				int index = y * _width + x;
				_tiles[index] = Tile{ x, y, TileType::GROUND, false };
			}
		}
	}

	int getWidth() const { return _width; }
	int getHeight() const { return _height; }

	Tile* getTile(int x, int y)
	{
		if (x < 0 || x >= _width || y < 0 || y >= _height)
			return nullptr;
		return &_tiles[y * _width + x];
	}

	void setObstacle(int x, int y, TileType type)
	{
		Tile* tile = getTile(x, y);
		if (tile)
			tile->type = type;
	}
};
