#pragma once

#include "../../Types.hpp"

struct NoTilingIndexProvider
{
    NoTilingIndexProvider(u32 cellsPerRow) :
        multiplier(cellsPerRow * 2),
        mask(multiplier - 1)
    {}

    inline u32 operator ()(i32 i, i32 j) const
    {
        return static_cast<u32>(i) + static_cast<u32>(j) * multiplier;
    }

private:
    u32 multiplier;
    u32 mask;
};

struct SimpleTilingIndexProvider
{
    SimpleTilingIndexProvider(u32 cellsPerRow) :
        multiplier(cellsPerRow),
        mask(multiplier - 1)
    {
    }

    inline u32 operator ()(i32 i, i32 j) const
    {
        return (static_cast<u32>(i) & mask) + (static_cast<u32>(j) & mask) * multiplier;
    }

private:
    u32 multiplier;
    u32 mask;
};

struct WangTilingIndexProvider
{
    WangTilingIndexProvider(u32 cellsPerRow) :
        multiplier(cellsPerRow),
        borderValue((cellsPerRow >> 2) - borderWidth),
        cellsPerTileRow(cellsPerRow >> 2)
    {
    }

    inline u32 operator ()(i32 i, i32 j) const
    {
        u32 x = static_cast<u32>(i);
        u32 y = static_cast<u32>(j);

        u32 localX = x % multiplier;
        u32 localY = y % multiplier;
        u32 tileX = localX / cellsPerTileRow;
        u32 tileY = localY / cellsPerTileRow;
        u32 xInTile = localX - tileX * cellsPerTileRow;
        u32 yInTile = localY - tileY * cellsPerTileRow;

        const bool isLeftBorder = (xInTile < borderWidth);
        const bool isRightBorder = (xInTile >= borderValue);
        const bool isBorderX = isLeftBorder || isRightBorder;
        const bool isTopBorder = (yInTile < borderWidth);
        const bool isBottomBorder = (yInTile >= borderValue);
        const bool isBorderY = isTopBorder || isBottomBorder;
        const bool isBorder = isBorderX || isBorderY;
        const bool isCorner = isBorderX && isBorderY;

        u32 left = tileX / 2;
        u32 bottom = 1 - tileY / 2;
        u32 right = left != (tileX & 1);
        u32 top = bottom == (tileY & 1);

        u32 tileOffsetX = (isCorner || !isBorderX) ? 0 : (isLeftBorder ? left : right);
        u32 tileOffsetY = (isCorner || !isBorderY) ? 0 : (isBottomBorder ? bottom : top);

        x = isBorder ? tileOffsetX * cellsPerTileRow + xInTile : x;
        y = isBorder ? tileOffsetY * cellsPerTileRow + yInTile : y;

        return x + y * multiplier;
    }

private:
    u32 multiplier;
    u32 borderValue;
    u32 cellsPerTileRow;

    static const u32 borderWidth = 1;
};
