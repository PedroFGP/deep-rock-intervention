# Deep Rock Intervention

## Summary

**Deep Rock Intervention** is a project based on the Deep Rock Galactic game made using Unreal Engine 4.27.

> Deep Rock Galactic is a cooperative first-person shooter video game developed by Danish studio Ghost Ship Games and published by Coffee Stain Publishing.

## Objectives

The main objectives of this project are defined below:

- Discover the main characteristics from the Unreal Engine 4
- Practice reverse engineering code
- Have fun making hacks

## Progress

So far the following has been achieved:

- [x] Basic engine structures [Commit](https://github.com/PedroFGP/deep-rock-intervention/commit/dd1af482fafb5150ddd93b379887575b15c61cde)
- [x] Math library: World2Screen, LineOfSight, Rotation delta Calculation [Commit](https://github.com/PedroFGP/deep-rock-intervention/commit/79190a88d19128ff4b948dc0140004bf437c090b)
- [x] Finished drawing methods [Commit](https://github.com/PedroFGP/deep-rock-intervention/commit/04181f3b852d3609ec25a88d34d0668753b01bc4)
- [x] Added support for correct actor (pawns iteration) [Commit](https://github.com/PedroFGP/deep-rock-intervention/commit/0c587938eff1aced485f466889478c4c5f8b7105)
- [x] Added support for correct entity naming and bone matrix access (pawns) [Commit](https://github.com/PedroFGP/deep-rock-intervention/commit/89950ade70e2846a9cf104ad88ec3c5c95d6c803)
- [x] Add support for aimbot [Commit](https://github.com/PedroFGP/deep-rock-intervention/commit/82b02ff167d24466dd07b03bdbaf963794fcc318)
- [x] Add support for no recoil [Commit](https://github.com/PedroFGP/deep-rock-intervention/commit/82b02ff167d24466dd07b03bdbaf963794fcc318)
- [x] Fix visibility check [Commit](https://github.com/PedroFGP/deep-rock-intervention/commit/82b02ff167d24466dd07b03bdbaf963794fcc318)

**TODO:**

- [ ] Add support for correct 2D box ESP (Extra Sensorial Perception)
- [ ] Refactor code (split classes into individual files)
- [ ] Add basic menu for on/off features

## Build & Use

In order to build and use this project you need vcredists installed and C++ 17 support at least. To use this, set compilation for 64 bits and use a valid injector to inject into the game's process.

## Reference Links

- [Sea of Thieves Reversal, Structs and Offsets](https://www.unknowncheats.me/forum/sea-of-thieves/278391-sea-thieves-reversal-structs-offsets.html)
- [Draw 3DBox Player and Vehicles](https://www.unknowncheats.me/forum/pubg-mobile/469873-draw-3dbox-player-vehicles.html)
- [Finding Offset In UE4 - Any Game](https://www.unknowncheats.me/forum/unreal-engine-4-a/285628-finding-offset-ue4-game.html)
- [UnrealDumper-4.25](https://www.unknowncheats.me/forum/unreal-engine-4-a/428680-unrealdumper-4-25-a.html)
