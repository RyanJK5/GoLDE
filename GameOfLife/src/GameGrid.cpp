#include "GameGrid.h"
#include "Logging.h"

#include <iostream>
#include <unordered_set>

gol::GameGrid::GameGrid(const std::vector<unsigned char>& seedBuffer, int32_t width, int32_t height)
	: m_Grid(width * height), m_Width(width), m_Height(height)
{
	ParseSeed(seedBuffer);
}

gol::GameGrid::GameGrid(int32_t width, int32_t height)
	: m_Grid(width * height), m_Width(width), m_Height(height)
{ }

gol::GameGrid::GameGrid(Size2 size)
	: GameGrid(size.Width, size.Height)
{ }

bool gol::GameGrid::Dead() const
{
	return std::find_if(m_Grid.begin(), m_Grid.end(), [](bool b) { return b; }) == m_Grid.end();
}

void gol::GameGrid::Update()
{
	std::vector<uint32_t> updateIndices;
	for (int32_t x = 0; x < m_Width; x++)
	{
		for (int32_t y = 0; y < m_Height; y++)
		{
			int8_t neighbors = CountNeighbors(x, y);
			int32_t index = y * m_Width + x;

			if (!(m_Grid[index] && neighbors < 2) &&
				!(m_Grid[index] && neighbors > 3) &&
				!(!m_Grid[index] && neighbors == 3))
				continue;

			updateIndices.push_back(static_cast<uint32_t>(index));
		}
	}

	for (uint32_t index : updateIndices)
	{
		m_Grid[index] = !m_Grid[index];
		m_Population += m_Grid[index] ? 1 : -1;
	}
	m_Generation++;
}

bool gol::GameGrid::Toggle(int32_t x, int32_t y)
{
	if (x >= m_Width || x < 0 || y >= m_Height || y < 0)
		return false;

	return Set(x, y, !m_Grid[y * m_Width + x]);
}

bool gol::GameGrid::Set(int32_t x, int32_t y, bool active)
{
	if (x >= m_Width || x < 0 || y >= m_Height || y < 0)
		return false;
	
	uint32_t index = static_cast<uint32_t>(y * m_Width + x);
	if (active != m_Grid[index])
		m_Population += active ? 1 : -1;
	m_Grid[index] = active;
	return true;
}

std::optional<bool> gol::GameGrid::Get(int32_t x, int32_t y) const
{
	if (x >= m_Width || x < 0 || y >= m_Height || y < 0)
		return {};
	return m_Grid[static_cast<uint32_t>(y * m_Width + x)];
}

void gol::GameGrid::ParseSeed(const std::vector<unsigned char>& seedBuffer)
{
	uint8_t charBits = sizeof(char) * 8;
	uint32_t area = m_Width * m_Height;
	for (uint32_t i = 0; i < (area / charBits); i++)
	{
		if (i >= seedBuffer.size())
			break;

		for (uint32_t bit = 0; bit < charBits; bit++)
		{
			unsigned char digit = 0b1 << (charBits - bit - 1);
			m_Grid[i * charBits + bit] = (digit & seedBuffer[i]) == digit;
		}
	}
}

int8_t gol::GameGrid::CountNeighbors(int32_t x, int32_t y)
{
	if (x < 0 || x >= m_Width || y < 0 || y >= m_Height)
		return 0;

	int8_t count = 0;

	for (int8_t i = -1; i <= 1; i ++)
	{
		if ((i < 0 && y == 0) || (i > 0 && y == m_Height - 1))
			continue;

		for (int8_t j = -1; j <= 1; j++)
		{
			if ((j < 0 && x == 0) || (j > 0 && x == m_Width - 1) || (i == 0 && j == 0))
				continue;
			if (m_Grid[(y + i) * m_Width + (x + j)])
				count++;
		}
	}

	return count;
}