#ifndef Topology_hpp_
#define Topology_hpp_

#include "BigInt.hpp"
#include "Graphics2D.hpp"
#include "LifeDataStructure.hpp"

#include <cstdint>
#include <optional>
#include <string_view>

namespace gol {
class Topology {
  public:
    Topology(Rect bounds = {});

    std::optional<Rect> GetBounds() const;

    virtual ~Topology() = default;

    virtual std::string_view GetIdentifier() const = 0;

    virtual BigInt MaxIncrement() const = 0;

    virtual void PrepareBorderCells(LifeDataStructure& data) = 0;

    virtual void CleanupBorderCells(LifeDataStructure& data) = 0;

  private:
    Rect m_Bounds;
};
} // namespace gol

#endif