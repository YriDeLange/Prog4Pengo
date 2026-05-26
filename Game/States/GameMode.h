#pragma once

// The three required play modes. Selected in MenuState, consumed by PlayingState
// to decide how many Pengos / Sno-Bees to spawn from a level's P/S tiles.
enum class GameMode
{
    SinglePlayer, // one Pengo (keyboard OR gamepad)
    Coop,         // two Pengos (keyboard + gamepad)
    Versus        // one Pengo + one player-controlled Sno-Bee
};