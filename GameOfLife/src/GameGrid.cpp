#include "GameGrid.h"
#include "Logging.h"

#include <iostream>
#include <unordered_set>

gol::GameGrid::GameGrid(const std::vector<unsigned char>&  seedBuffer, size_t width, size_t height)
	: m_grid(width * height), m_width(width), m_height(height)
{
	ParseSeed(seedBuffer);
}

gol::GameGrid::GameGrid(const std::vector<bool>& seed, size_t width, size_t height)
	: m_grid(seed), m_width(width), m_height(height)
{ }

bool gol::GameGrid::Dead() const
{
	return std::find_if(m_grid.begin(), m_grid.end(), [](bool b) { return b; }) == m_grid.end();
}

void gol::GameGrid::Update()
{
	std::unordered_set<size_t> updateIndices;
	for (size_t x = 0; x < m_width; x++)
	{
		for (size_t y = 0; y < m_height; y++)
		{
			uint8_t neighbors = CountNeighbors(x, y);
			size_t index = y * m_width + x;

			if (!(m_grid[index] && neighbors < 2) &&
				!(m_grid[index] && neighbors > 3) &&
				!(!m_grid[index] && neighbors == 3))
				continue;

			updateIndices.insert(index);
		}
	}

	for (auto& index : updateIndices)
		m_grid[index] = !m_grid[index];
}

bool gol::GameGrid::Toggle(size_t x, size_t y)
{
	if (x >= m_width || x < 0 || y >= m_height || y < 0)
		return false;

	return Set(x, y, !m_grid[y * m_width + x]);
}

bool gol::GameGrid::Set(size_t x, size_t y, bool active)
{
	if (x >= m_width || x < 0 || y >= m_height || y < 0)
		return false;

	m_grid[y * m_width + x] = active;
	return true;
}

std::pair<float, float> gol::GameGrid::GLCoords(size_t x, size_t y) const
{
	return std::make_pair((x - m_width / 2.0f) / (m_width / 2.0f),
		(m_height / 2.0f - y) / (m_height / 2.0f));
}

std::pair<float, float> gol::GameGrid::GLCellDimensions() const
{
	return std::make_pair(2.0f / m_width, 2.0f / m_height);
}

void gol::GameGrid::ParseSeed(const std::vector<unsigned char>& seedBuffer)
{
	uint8_t charBits = sizeof(char) * 8;
	unsigned int area = m_width * m_height;
	for (size_t i = 0; i < (area / charBits); i++)
	{
		if (i >= seedBuffer.size())
			break;

		for (size_t bit = 0; bit < charBits; bit++)
		{
			unsigned char digit = 0b1 << (charBits - bit - 1);
			m_grid[i * charBits + bit] = (digit & seedBuffer[i]) == digit;
		}
	}
}

uint8_t gol::GameGrid::CountNeighbors(size_t x, size_t y)
{
	if (x < 0 || x >= m_width || y < 0 || y >= m_height)
		return 0;

	uint8_t count = 0;

	for (int8_t i = -1; i <= 1; i ++)
	{
		if ((i < 0 && y == 0) || (i > 0 && y == m_height - 1))
			continue;

		for (int8_t j = -1; j <= 1; j++)
		{
			if ((j < 0 && x == 0) || (j > 0 && x == m_width - 1) || (i == 0 && j == 0))
				continue;
			if (m_grid[(y + i) * m_width + (x + j)])
				count++;
		}
	}

	return count;
}

std::vector<float_t> gol::GameGrid::GenerateGLBuffer() const
{
	std::vector<float_t> result;

	result.reserve(m_width * m_height * 4);

	auto [glW, glH] = GLCellDimensions();
	for (size_t x = 0; x < m_width; x++)
	{
		for (size_t y = 0; y < m_height; y++)
		{
			if (!m_grid[y * m_width + x])
				continue;

			auto [glX, glY] = GLCoords(x, y);
			
			result.emplace_back(glX);
			result.emplace_back(glY);
			
			result.emplace_back(glX);
			result.emplace_back(glY - glH);

			result.emplace_back(glX + glW);
			result.emplace_back(glY - glH);

			result.emplace_back(glX + glW);
			result.emplace_back(glY);
		}
	}

	return result;
}