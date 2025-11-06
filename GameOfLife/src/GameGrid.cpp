#include "GameGrid.h"
#include "Logging.h"

#include <iostream>
#include <map>

gol::GameGrid::GameGrid(int32_t width, int32_t height)
	: m_Width(width), m_Height(height)
{ }

gol::GameGrid::GameGrid(Size2 size)
	: GameGrid(size.Width, size.Height)
{ }

gol::GameGrid::GameGrid(const GameGrid& other, Size2 size)
	: GameGrid(size)
{
	m_Population = other.m_Population;
	for (const auto& pos : other.Data())
	{
		if (InBounds(pos))
			m_Data.insert(pos);
	}
}

bool gol::GameGrid::Dead() const
{
	return m_Data.size() == 0;
}

void gol::GameGrid::Update()
{
	std::map<Vec2, int8_t> neighborCount;
	for (const auto& pos : m_Data)
	{
		for (int32_t x = pos.X - 1; x <= pos.X + 1; x++)
		{
			for (int32_t y = pos.Y - 1; y <= pos.Y + 1; y++)
			{
				if (!InBounds(x, y))
					continue;
				if (x != pos.X || y != pos.Y)
					neighborCount[{x, y}]++;
			}
		}
	}

	std::set<Vec2> newSet;
	for (auto&& [pos, neighbors] : neighborCount)
	{
		if (neighbors == 3 || (neighbors == 2 && m_Data.find(pos) != m_Data.end()))
			newSet.insert(pos);
	}
	m_Data = newSet;
	m_Population = newSet.size();
	m_Generation++;
}

bool gol::GameGrid::Toggle(int32_t x, int32_t y)
{
	if (!InBounds(x, y))
		return false;

	return Set(x, y, m_Data.find({x, y}) != m_Data.end());
}

bool gol::GameGrid::Set(int32_t x, int32_t y, bool active)
{
	if (!InBounds(x, y))
		return false;

	auto itr = m_Data.find({x, y});
	if (itr == m_Data.end() && active)
	{
		m_Population++;
		m_Data.insert({ x, y });
	}
	else if (itr != m_Data.end() && !active)
	{
		m_Population--;
		m_Data.erase(itr);
	}

	m_Generation = 0;
	return true;
}

std::optional<bool> gol::GameGrid::Get(int32_t x, int32_t y) const
{
	if (!InBounds(x, y))
		return std::nullopt;
	auto itr = m_Data.find({ x, y });
	return itr != m_Data.end();
}