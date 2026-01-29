#include <algorithm>
#include <cstdint>
#include <limits>
#include <memory>
#include <optional>
#include <set>
#include <utility>
#include <vector>

#include "GameGrid.h"
#include "Graphics2D.h"
#include "LifeHashSet.h"
#include "LifeAlgorithm.h"

gol::GameGrid::GameGrid(int32_t width, int32_t height)
	: m_Width(width), m_Height(height)
	, m_Algorithm(LifeAlgorithm::HashLife)
	, m_Data(LifeHashSet{})
{ }

gol::GameGrid::GameGrid(Size2 size)
	: GameGrid(size.Width, size.Height)
{ }

gol::GameGrid::GameGrid(const GameGrid& other, Size2 size)
	: GameGrid(size)
{
	m_Population = other.m_Population;
	m_Data = other.Data() 
		| std::views::filter([this](const auto& pos) { return InBounds(pos); }) 
		| std::ranges::to<LifeHashSet>();
}

bool gol::GameGrid::Dead() const
{

	return m_Data.size() == 0;
}

gol::Rect gol::GameGrid::BoundingBox() const
{
	if (Bounded())
		return { 0, 0, m_Width, m_Height };

	auto least = Vec2 { std::numeric_limits<int32_t>::max(), std::numeric_limits<int32_t>::max() };
	auto most = Vec2  { std::numeric_limits<int32_t>::min(), std::numeric_limits<int32_t>::min() };
	for (const auto value : m_Data)
	{
		least.X = std::min(least.X, value.X);
		least.Y = std::min(least.Y, value.Y);

		most.X = std::max(most.X, value.X);
		most.Y = std::max(most.Y, value.Y);
	}

	return Rect 
	{ 
		least.X, 
		least.Y, 
		most.X - least.X + 1, 
		most.Y - least.Y + 1 
	};
}

const std::set<gol::Vec2>& gol::GameGrid::SortedData() const
{
	if (m_CacheInvalidated)
		ValidateCache(true);
	return m_SortedData;
}

const gol::LifeHashSet& gol::GameGrid::Data() const
{
	if (m_CacheInvalidated && m_HashLifeData)
		ValidateCache(false);
	return m_Data;
}

std::variant<std::reference_wrapper<const gol::LifeHashSet>, std::reference_wrapper<const gol::HashQuadtree>> gol::GameGrid::IterableData() const
{
	if (m_Algorithm == LifeAlgorithm::HashLife && m_HashLifeData)
		return std::ref(*m_HashLifeData);
	else
		return std::ref(m_Data);
}

void gol::GameGrid::Update(int64_t numSteps)
{
	switch (m_Algorithm) {
	case LifeAlgorithm::SparseLife:
		for (auto i = 0; i < numSteps; i++)
		{
			m_Data = SparseLife(m_Data, {0, 0, m_Width, m_Height});
			m_Generation++;
		}
		break;
	case LifeAlgorithm::HashLife:
		if (!m_HashLifeData)
			m_HashLifeData = HashQuadtree{ m_Data, {0, 0} };
		auto updateInfo = HashLife(*m_HashLifeData, { 0, 0, m_Width, m_Height }, numSteps);
		
		if (std::numeric_limits<int64_t>::max() - updateInfo.Generations < m_Generation)
			m_Generation = std::numeric_limits<int64_t>::max();
		else
			m_Generation += updateInfo.Generations;
	
		m_HashLifeData = std::move(updateInfo.Data);
		break;
	}

	m_CacheInvalidated = true;
	m_Population = m_Data.size();
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
		m_SortedData.insert({ x, y });
	}
	else if (itr != m_Data.end() && !active)
	{
		m_Population--;
		m_Data.erase(itr);
		m_SortedData.erase({x, y});
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
	for (auto& pos : newCells)
		m_Data.insert(pos);
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

gol::LifeHashSet gol::GameGrid::ReadRegion(const Rect& region) const
{
	LifeHashSet result;
	for (auto&& pos : m_Data)
	{
		if (region.InBounds(pos))
			result.insert(pos);
	}
	return result;
}


void gol::GameGrid::ClearRegion(const Rect& region)
{
	m_Population -= std::erase_if(m_Data, [region](Vec2 pos) { return region.InBounds(pos); });
	m_CacheInvalidated = true;
}

void gol::GameGrid::ClearData(const std::vector<Vec2>& data, Vec2 offset)
{
	for (auto& vec : data)
	{
		m_Population -= m_Data.erase({ vec.X + offset.X, vec.Y + offset.Y });
	}
	m_CacheInvalidated = true;
}

gol::LifeHashSet gol::GameGrid::InsertGrid(const GameGrid& region, Vec2 pos)
{
	gol::LifeHashSet result {};
	for (auto&& cell : region.m_Data)
	{
		Vec2 offsetPos = { pos.X + cell.X, pos.Y + cell.Y };
		if (m_Data.find(offsetPos) != m_Data.end())
			continue;
		m_Data.insert(offsetPos);
		m_SortedData.insert(offsetPos);
		result.insert(offsetPos);
		m_Population++;
	}
	return result;
}

void gol::GameGrid::RotateGrid(bool clockwise)
{
	auto center = Vec2F { static_cast<float>(m_Width / 2.f - 0.5f), static_cast<float>(m_Height / 2.f - 0.5f) };
	gol::LifeHashSet newSet {};
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
	m_CacheInvalidated = true;
	m_Data = std::move(newSet);
}

void gol::GameGrid::FlipGrid(bool vertical)
{
	LifeHashSet newData;
	if (!Bounded())
	{
		for (const auto pos : m_Data)
		{
			if (vertical)
				newData.insert({ pos.X, -pos.Y });
			else
				newData.insert({ -pos.X, pos.Y });
		}
	}
	else
	{
		for (const auto pos : m_Data)
		{
			if (vertical)
				newData.insert({ pos.X, m_Height - 1 - pos.Y });
			else
				newData.insert({m_Width - 1 - pos.X, pos.Y});
		}
	}
	m_CacheInvalidated = true;
	m_Data = std::move(newData);
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

void gol::GameGrid::ValidateCache(bool validateSorted) const
{
	if (m_HashLifeData)
		m_Data = *m_HashLifeData | std::ranges::to<LifeHashSet>();
	if (validateSorted)
		m_SortedData = m_Data | std::ranges::to<std::set<Vec2>>();
}
