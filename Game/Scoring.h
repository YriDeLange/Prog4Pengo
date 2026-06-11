#pragma once

// Official scoring values from the Sega Pengo preliminary owner's manual
namespace Scoring
{
    // Sno-Bee kills
    inline constexpr int StunnedSnoBeeCaught = 100;   // run over a wall-stunned Sno-Bee
    inline constexpr int SnoBeeCrushed = 400;   // crush one with an ice block
    inline constexpr int TwoSnoBeesCrushed = 1'600; // single push crushes 2 at once
    inline constexpr int ThreeSnoBeesCrushed = 3'200; // single push crushes 3 at once

    // Blocks
    inline constexpr int EggBlockDestroyed = 500;   // disintegrate a block containing an egg

    // Diamond alignment bonus
    inline constexpr int DiamondsAlignedOnWall = 5'000;  // line touches a wall (easier)
    inline constexpr int DiamondsAlignedFree = 10'000; // line in the open field

    // Time bonus: all Sno-Bees destroyed within the round time.
    // Returns the bonus for clearing the board at `secondsTaken`.
    inline constexpr int TimeBonus(float secondsTaken)
    {
        if (secondsTaken < 20.f) return 5'000;
        if (secondsTaken < 30.f) return 2'000;
        if (secondsTaken < 40.f) return 1'000;
        if (secondsTaken < 50.f) return 500;
        if (secondsTaken < 60.f) return 10;
        return 0;
    }
}