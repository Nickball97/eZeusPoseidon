# Adding a building

Written on 13 Nov 2026, when the table of building kinds was introduced.
It says what one place now covers, and lists honestly the places that still
have to be touched by hand.

## The table of building kinds

`buildings/ebuildingregistry.cpp` has one line per kind of building that is
made the plain way: from the map and the city it belongs to, on a fixed
footprint. 71 kinds are in it. A line reads:

    {eBT::dairy, 2, 2, &make<eDairy>, false},

that is: the type, the footprint in tiles (width, height), how one is made,
and whether its walkers follow the routes a computer city's plan lays down
(true only for the eight service buildings whose routes can be planned).

Three parts of the game work from this table, so a new plain building needs
nothing in them:

- **loading a save** (`fileIO/ebuildingreader.cpp`);
- **writing a save** (`fileIO/ebuildingwriter.cpp`);
- **the computer player placing it from a city plan**
  (`engine/ai/eaidistrict.cpp`).

Buildings that need more to exist keep their own cases in those files: an
orientation (agora, fishery, pier, gatehouse, stadium, palace), a god or a
hero (sanctuaries, hero halls, monuments), a partner building (stalls,
horse ranch, palace tiles), or a number (commemorative, water park).

**The check.** `tools/check/check.ps1` starts by making one building of
every kind in the table and comparing it with its line (type and footprint).
A wrong line shows as `TABLE WRONG`. By hand:
`eZeus-check.exe --headless-check "The Founding of Athens" 0 --building-table`.

## What a new plain building still needs, by hand

One type name still appears in about nineteen files. For a plain building
(a new workshop, a cistern) the steps are:

1. **The class**: `buildings/e<name>.h/.cpp`, added to `CMakeLists.txt`,
   `eZeus.pro` and `buildings/allbuildings.h`.
2. **The type**: a new value in `eBuildingType`. Add it at the end: the
   number is written into save files.
3. **The table**: its line in `buildings/ebuildingregistry.cpp`.
4. **Name and info text**: `eBuilding::sNameForBuilding` and the info-window
   text choice, both in `buildings/ebuilding.cpp`.
5. **Cost and risks**: the cost, fire-risk and damage-risk lists in
   `engine/edifficulty.cpp`.
6. **Workers**: which sector it belongs to, in
   `engine/eemploymentdistributor.cpp`.
7. **Whether an episode allows it**: `buildings/eavailablebuildings.cpp`,
   and the editor's list in `widgets/eeditorsettingsmenu.cpp`.
8. **The build menu**: a build mode in `widgets/ebuildingmode.h/.cpp`, its
   button in `widgets/egamemenu.cpp`, placing it in
   `widgets/egamewidgetbuild.cpp` and its preview under the pointer in
   `widgets/egamewidgetpaint.cpp`.
9. **How it shows**: its pictures (`textures/ebuildingtextures.*`), its
   colour on the small map (`widgets/eminimap.cpp`), the overlay it belongs
   to (`widgets/eviewmode.cpp`), its sound (`audio/esounds.cpp`).
10. **Its surroundings**: how it changes the appeal of the land around it
    (`buildings/eheatgetters.cpp`).

Depending on what it does there may be more (a building that stores goods
is also listed in `engine/thread/ethreadbuilding.cpp`, for the path
searches).

## Where this should go next

The same idea, one step at a time, each checked against the safety net or
by the table's own check:

- **Build menu and preview.** `egamewidgetbuild.cpp` and
  `egamewidgetpaint.cpp` each have about seventy near-identical cases that
  make a plain building. With the footprint already in the table, most of
  them can become one shared path. This is interface code the safety net
  cannot see, so it needs a play test of the build menu.
- **Name, cost, risks, sector, small-map colour, sound** as further columns
  of the table, replacing the separate lists in steps 4 to 9. After that a
  plain building is: its class, its type, one line.
- **The special buildings** (those with an orientation or a partner) could
  get their own, smaller tables once the plain ones are done.
