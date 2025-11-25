#include <cstdint>
#include <map>
#include <optional>
#include <set>
#include <utility>
#include <vector>

#include "GameGrid.h"
#include "Graphics2D.h"
#include <functional>

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

struct Vec2Hash
{
	size_t operator()(gol::Vec2 vec) const
	{
		return std::hash<int32_t>{}(vec.X) ^ std::hash<int32_t>{}(vec.Y);
	}
};

void gol::GameGrid::Update()
{
	std::unordered_map<Vec2, int8_t, Vec2Hash> neighborCount;
	for (auto&& pos : m_Data)
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

void gol::GameGrid::TranslateRegion(const Rect& region, Vec2 translation)
{
	std::vector<Vec2> newCells;
	auto it = m_Data.begin();
	while (it != m_Data.end())
	{
		if (region.InBounds(*it))
		{
			newCells.push_back(*it + translation);
			m_Data.erase(it++);
			continue;
		}
		++it;
	}
	m_Data.insert_range(newCells);
}

gol::GameGrid gol::GameGrid::SubRegion(const Rect& region) const
{
	auto result = GameGrid { region.Width, region.Height };
	for (auto&& pos : m_Data)
	{
		if (region.InBounds(pos))
		{
			result.m_Population++;
			result.m_Data.insert(pos - region.Pos());
		}
	}
	return result;
}

std::set<gol::Vec2> gol::GameGrid::ReadRegion(const Rect& region) const
{
	std::set<Vec2> result;
	for (auto&& pos : m_Data)
	{
		if (region.InBounds(pos))
			result.insert(pos);
	}
	return result;
}


void gol::GameGrid::ClearRegion(const Rect& region)
{
	m_Population -= std::erase_if(m_Data, [region](const Vec2& pos) { return region.InBounds(pos); });
}

void gol::GameGrid::ClearData(const std::vector<Vec2>& data, Vec2 offset)
{
	for (auto& vec : data)
	{
		m_Population -= m_Data.erase({ vec.X + offset.X, vec.Y + offset.Y });
	}
}

std::set<gol::Vec2> gol::GameGrid::InsertGrid(const GameGrid& region, Vec2 pos)
{
	std::set<Vec2> result {};
	for (auto&& cell : region.m_Data)
	{
		Vec2 offsetPos = { pos.X + cell.X, pos.Y + cell.Y };
		if (m_Data.find(offsetPos) != m_Data.end())
			continue;
		m_Data.insert(offsetPos);
		result.insert(offsetPos);
		m_Population++;
	}
	return result;
}

void gol::GameGrid::RotateGrid(bool clockwise)
{
	auto center = Vec2F { static_cast<float>(m_Width / 2.f - 0.5f), static_cast<float>(m_Height / 2.f - 0.5f) };
	std::set<Vec2> newSet;
	for (auto&& cellPos : m_Data)
	{
		auto offset = Vec2F { static_cast<float>(cellPos.X), static_cast<float>(cellPos.Y) } - center;
		auto rotated = clockwise
			? Vec2F { -offset.Y,  offset.X }
			: Vec2F { offset.Y, -offset.X };
		auto result = rotated + Vec2F { center.Y, center.X };
		newSet.insert(Vec2 { static_cast<int32_t>(result.X), static_cast<int32_t>(result.Y) });
	}
	std::swap(m_Width, m_Height);
	m_Data = newSet;
}

std::optional<bool> gol::GameGrid::Get(int32_t x, int32_t y) const
{
	return Get({ x, y });
}

std::optional<bool> gol::GameGrid::Get(Vec2 pos) const
{
	if (!InBounds(pos))
		return std::nullopt;
	return m_Data.find(pos) != m_Data.end();
}