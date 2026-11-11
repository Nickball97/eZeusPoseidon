# Findings from the original Zeus.exe

Read with Ghidra 12.2 from Nicolás's Steam `zeus.exe` (same build as GOG: compiled 2002-01-04 11:03, same size; checksum differs in a few bytes). Function names and the address map come from zeus-poseidon-arm64 (`analysis/symbols.tsv`). Tools live in `C:\Users\LENOVO\source\repos\re-tools` (see `gh.ps1`; decompiled output in `work\out`). "Cycle" = one of the 16 simulation steps per game month.

## Crime (verified in code)
- Each common house (types 2–8) has a crime level, 0–100. Elite houses have none.
- **When a house is built**, its crime level starts at the house table's "crime base" (Model column p).
- **Twice a month** (cycles 0 and 8; `0x005671A0` → `0x00566F20`), once the city has 350 or more people: crime += crime increment for the house level (column o) + (40 − city mood) / 2 (integer division). Then it is floored at the crime base and capped at 100.
- **Watchmen** (figure 61, `0x004DFB70` → `0x004A5F70`): every house within 2 tiles of the watchman loses the guardpost's "Risk Reducer" (Model `BUILD_GUARDPOST` column j = 50 on every difficulty), floored at the crime base. There is no persuasion: the watchman just lowers crime around him.
- **Outlaws** (`0x005BFDE0`, every cycle): for each threshold a house's crime level reaches, a 1-in-200 draw spawns that walker, if the house has none out already: 70 → disgruntled citizen (figure 22), 80 → mugger (27), 90 → thief (23), 100 → vandal (29). Table at `0x0065E898`.
- **Gods' sanctuaries** (`0x004E9440` with a god record, see "Sanctuary benefits"): Dionysus sets the crime increment to 0 and makes crime fall by 20 each update; Aphrodite lowers house appeal needs by 10.

## City mood (verified in code, `0x005671A0`, twice a month)
- Starts at 60, range 0–100. Each update adds the sum of six factors:
  - **taxes:** the tax-sentiment table per difficulty (already in eZeus);
  - **wages:** none −10, very low −5, low −2, normal 0, high +2, very high +4 (table `0x00641F20`);
  - **unemployment:** under 5% +1; 5–10% 0; 11–17% −1; 18–25% −2; 26% or more −3;
  - **food:** the average over houses of +2 for a fed house, −1, −2, −3 for a house unfed 1, 2, 3+ updates in a row;
  - **debt:** −2 after the first full year in debt, −5 after two years, −9 after three, and −2 more while the treasury is negative (the month counter adds a year every 12, `0x00483B20`);
  - **a sixth factor** (`0x00566E70`) from counts kept by `0x004DDD70` (figures of types 11–13), −1 to −5; not identified yet.
- Below 350 people, negative factors count as 0.
- Drift toward the middle: mood under 11 +4, 11–20 +3, 21–30 +2, 31–40 +1, 41–60 0, 61–70 −1, 71–80 −2, 81–90 −3, above 90 −4. Below 41 the drift only stops a fall, it never pushes the mood up past it; above 60, likewise for a rise.

## Houses
- A common house devolves only after failing its needs on 3 checks in a row (types 3–8, verified for 3, 4, 5 and 8); elite residence to estate after 10 (`0x004FE9C0` … `0x004FEC30`); the devolved elite type counts to 12 (`0x004FE900`). A passing check resets the count, and a house moves one level per check.

## Appeal (from zeus-poseidon-arm64 docs/engine/terrain.md §7, applied 9 Nov 2026)
- Spread ring by ring from the footprint, at most 6 rings; the footprint itself gets nothing; each tile clamped to −100..100.
- A building's appeal is the highest over its footprint, +10 when a tile sharing an edge with the footprint is water (tile flag 4). The building creator `0x00493E70` stores this as +0xD0 from `0x004A4500`, which reads the ring of edge tiles in table `0x005F1680` (24 offsets per size); `0x004842A0` adds it each cycle. Placing an elite home (`0x004FF430`) uses the same value over the 4x4 spot against the Model elite level-1 appeal (Aphrodite included). Applied 10 Nov 2026 to eBuilding::appeal and the elite placement check (which used the average appeal).

## Outlaws' behaviour (verified in code)
- Action table per figure type at `0x0063969C` (40 bytes each): disgruntled `0x004B5A20`, thief `0x004B5B70`, mugger `0x004B68B0`, vandal `0x004B6A00`.
- **Thief** heads for an elite home (`0x005C0630`). There he takes, from each good, a quarter of the stock or half the level's Model storage number if larger (food: column q, horses: g, armor: n), shows message 19:167 and leaves (`0x005C00C0`).
- **Vandal** heads for an elite home, stands there for a countdown of 51 steps, then destroys it and shows 19:168 (`0x005C0360`).
- A thief or vandal who finds no elite home leaves, and his house sends a **mugger** instead (`0x005BFF90` with figure 27).
- **Mugger:** when his attack countdown ends, the victim is removed (dies); no chance roll found.
- **Disgruntled citizen:** only roams.

## Sixth mood factor: military service (applied 9 Nov 2026)
`0x00566E70` counts the city's rabble, hoplite and horse companies (figure types 11–13) carrying a flag (probably out of their forts or away), once the city has had 2+ months of peace: 0 → 0, 1–2 → −1, 3–5 → −2, 6–8 → −3, 9–12 → −4, 13+ → −5.

## Plague
The plague walker (figure 66) calls an empty function on each tile, so in the original it spreads nothing as it walks.

## Plague outbreaks (verified in code, `0x00447FF0`, `0x004481B0`, `0x00448490`, `0x00448220`)
- City health starts at 80. Each cycle a 4-digit random draw (0–9999) against a chance from the health band (table `0x005F84A8`): health under 10 → 300 (3%), 10–19 → 250, 20–29 → 200, 30–44 → 100, 45–59 → 50, 60–74 → 25, 75–99 → 0.
- Outbreaks are capped: at most ceil(chance × 384 / 10000) counted over a period (and half that by one counter), and none while one is running.
- An outbreak (`0x00448490`) keeps the 40% of lived-in houses with the lowest health-care score, which is the healer visit counter (+0x78, 96 after a visit, falling 1 a cycle) + 20 when flag +0x7f is set (the same flag that adds 50 to the health target; eZeus takes it to be water, but what sets it wasn't found). From the lowest up, each is infected on a coin toss until half are taken: it loses part of its people and sends out 3 plague walkers (timer 600). eZeus already does this (9 Nov 2026).

## Hero hall requirements (verified in code, `0x004FBC20`)
Applied 9 Nov 2026: popularity (city mood) above 89; health above 79; gymnasium/observatory coverage above 89%; tax coverage above 88%; the hall's appeal above 60; a palace tile within 5 tiles of the hall (`0x004FBB20`); walls: no path from the hall to the map's entry points.

## Tax coverage
The tax mood factor uses the "no taxes" row when tax coverage is below 11% (`0x00567120`).

## City health and plague details (verified in code; applied 9 Nov 2026)
- Health moves 1 point per cycle toward a target (`0x00566B90`).
- Target, monthly (`0x00566C00`): population-weighted average over houses of 50 for a recent healer visit + 50 for a second flag (taken to be water), plus a small-city bonus (table `0x00641ED0`): under 100 people +80, 200 +80, 300 +70, 400 +60, 500 +50, 600 +40, 700 +30, 800 +20, 900 +10, 1000+ 0.
- A healer (figure 32) sets the visit value to 96 for houses within 2 tiles (`0x004A66B0`); how fast it wears off was not traced (eZeus uses its hygiene value).
- An infected house loses a share of its people by difficulty (doubles at `0x005F8480`): 25%, 33%, 50%, 67%, 100%; it stays sick for 32 cycles (house timer +0x100).
- An outbreak infects a fifth of lived-in houses (candidates are the 40% with the least health care; each taken on a coin toss until half are chosen).

## Sanctuary benefits (verified in code)
`0x004E9440` checks one god's sanctuary through a record in an array at `0x00EF5DD8`, 0x34 bytes per god, in the Model's sanctuary row order: Zeus, Poseidon, Demeter, Apollo, Artemis, Ares, Aphrodite, Hermes, Athena, Hephaestus, Dionysus, Hades, Hera, Atlas. Effects found:
- **Aphrodite:** every house's appeal needs (evolve and devolve) are 10 lower (`0x00433D80`).
- **Dionysus:** crime increment 0, and crime falls by 20 each update (`0x00433D80`, `0x00566F20`).
- **Hephaestus:** building fire risk 0 (`0x00433D50`).
- **Athena:** the player's rabble, hoplites and horsemen get +4 armor against missiles (`0x004E1B20`).
- **Hermes:** cart pushers, storehouse workers and deliverymen get speed code ×1.33 (6 → 7 = 1.25×); merchants, donkeys and trade ships ×1.5 (`0x004E1B20`).
- **Apollo:** +20 to the city's Pan-Hellenic games score (`0x00565D40`).
- **Hera:** +3% to the yearly births (`0x00566D40`).
- **Artemis:** the wolf's action (`0x004BD4A0`) calls `0x0056AF80`. When a wolf attacks, or its victim has just died, and her sanctuary is finished (record 0xEF5EA8), Artemis (god 4) is found, or spawned at her sanctuary, and sent to attack the wolf (state 0x4A). Replaced by Nicolás's mod rule (9 Nov 2026): once her sanctuary is finished, Artemis, while out on her rounds, hunts down every wolf in her city with her arrows (eArtemisHuntAction), and that city's wolf dens stop producing wolves (eWolfSpawner). This is not the original rule.
- **Hermes, trade visits** (`0x005B9EE0`, every cycle, record 0xEF5F44): a route's next trader waits 4 cycles (caravan) or 30 cycles (trade ship) while it has a free trader slot. Hermes' sanctuary: x0.66. A route allows 1-3 traders at once: each traded good (city +0x60+i) counts 1 (under 13), 2 (13-24) or 3 (25+), averaged and rounded up. Applied in eTradePost. Before, eZeus sent one trader every 30000 ticks with no cap.
- Ares (`0x0046DAA0`): that check is in the request handler (sending Ares on troop requests), not a city bonus. Hades (`0x00546E80`): sanctuary layout geometry, not a bonus. Zeus: the only direct uses of his record (`0x00482B10` god events, `0x004EE329` and `0x0052FBA0` Zeus figure behaviour, `0x004FC5F0` god figure search) are not city bonuses. No "opinion abroad" effect was found.

## Births and deaths (verified in code, `0x00566D40`, once a year)
Population changes by a share set by city health: 91+ +6%, 81–90 +4%, 71–80 +3%, 61–70 +2%, 51–60 +1%, 41–50 0; 40 or less: −1% while the health target is 31+, otherwise −2% (21–40), −3% (11–20), −5% (1–10). Hera +3%.

## Immigration and emigration (verified in code, `0x00567450`, every cycle)
- Rate by city mood: 71+ 100, 61–70 75, 50–60 50, 36–49 0, 26–35 −8, 16–25 −17, under 16 −25.
- Positive: ceil(rate × 12 / 100) newcomers per cycle (12 / 9 / 6), arriving in groups of 4. Negative, with 101+ people: ceil(|rate| × 12 / 100) leave per cycle (1 / 3 / 3), in groups of 4, from the lowest houses first, at most 4 from one house (`0x00486AB0`). Two-cycle pause after switching between the two.
- Aphrodite's sanctuary stops emigration (her record is checked in `0x00486AB0`).
- There is no separate "unhappy house leaves" rule.

## Fishing and urchins (verified in code)
- Fishing boat (`0x004B5F10`): catching takes 408 walker ticks at full staff (half a month at 816 ticks a month), 816 / 1224 / 1632 / 2040 at 75 / 50 / 25 / under 25% staffing; Poseidon's sanctuary: ×0.66. Unloading 204 ticks at full staff (204 more per lower band). Each catch is 1 load.
- Urchin gatherer (`0x004302D0`, `0x00430400`): 408 ticks per trip, +100 units (1 load) per trip, quay holds 500 units.

## Trireme wharf (verified in code, `0x0048CFB0`)
Progress per cycle by staffing (10 / 8 / 6 / 4 / 2); a ship is launched past 319, so 32 cycles (2 months) at full staff. Needs 200+ units of a material in stock to progress.

## Roads, ground and the appeal view (verified in code)
- Roads on slopes use the paved image when the tile's appeal is above 4 (`0x004A0480`, called from the slope road picker `0x0045E110`). Not checked whether eZeus has roads on slopes.
- Flat roads (`0x004A0190`, called for every tile with road flag 0x40 by `0x004A0090`) pick one of four looks by the tile's appeal (`0x004A0450`): below 0 SPR_RUTTED_ROAD (Zeus_Terrain group 43, Land3 images 390–438), 0–15 SPR_ROADS (group 33, Land3 235–252), 16–29 SPR_PAVED_ROADS (group 39, Land3 253–301), 30+ SPR_PLAZA_1X1 (Zeus_General group 47, 7 stones, chosen by a per-tile random byte mod 7, no road shapes). The tiles around (top-right, right, bottom-right, bottom, bottom-left, left, top-left, top) are matched against the tables at `0x00637098` (17 pieces, plain roads, diagonals ignored) and `0x006371F0` (48 pieces, paved and rutted, with pieces for wide road areas); first fit wins, no variants. Roads on the beach (tile flag 0x10000) use the beach road images instead. Earlier notes called this "plain ground"; it is roads. Applied 10 Nov 2026 (`eRoad::getTexture`); straight plain roads now always use images 0/1 as in the original (eZeus used to swap in 2/3 at random).
- Appeal view colour (`0x0044BCF0`): −11 or less → 0; −10 … 69 → appeal/10 + 2; 70+ → 9; a tile at 0 with no building is not coloured.

## Avenue and Boulevard (verified in code and the Zeus manual p. 156; applied 10 Nov 2026)
- Two buildings: Avenue (type 118) and Boulevard (type 126), each with its own name, help text (Zeus_Text group 135 / 136), cost and scenario switch. Manual: a Boulevard is placed directly over an existing road with both sides clear, lined on both sides; an Avenue is built the same way but lined on one side only.
- Tools: Avenue vtable `0x005D85E4`, Boulevard `0x005D85C0`. Build loop: Avenue `0x00419F90`, Boulevard `0x0041B250`. One piece: `0x00419C90` / `0x0041AFB0`. Start tile: `0x0041AEA0` / `0x0041BBA0`. Cursor preview: `0x0041A820` / `0x00419760`. Delete: `0x0041A6D0`→`0x0041A490` / `0x0041BA20`→`0x0041B6E0`.
- A piece is one building over 2 tiles (Avenue: the road tile and one tree tile) or 3 (Boulevard: trees, road, trees). The road tile keeps its road flag, so walkers use it as a road.
- Start tile, Boulevard: must be a road. If the road runs along x (x ± 1 both road), the trees go at y ± 1; if it runs along y they go at x ± 1.
- Start tile, Avenue: on a road, the same rule puts the trees on the x − 1 or y − 1 side. Next to a road, they go on the clicked tile: road at x − 1 or y − 1 is checked first, then x + 1, then y + 1.
- Dragging: the line runs along the drag's longer direction from the start. If the drag turns across the road, the Avenue's side keeps to the same edge (lower or higher coordinate). A click without a drag builds nothing.
- Each piece needs a road tile, and tree tiles with no building and none of the blocking tile flags. Road isn't one of those flags, so the original lets trees go over a second road.
- After building (`0x004195D0`), any piece with no Avenue or Boulevard tile next to it along the line is removed and not charged, so the shortest is 2 pieces. The cost is per piece (Model BUILD_AVENUE 10/16/20/25/30, BUILD_BOULEVARD 15/24/30/36/45).
- Appeal from every tile (`0x00493C80`). Avenue: 3, falling 1 per tile, range 3. Boulevard: 3, falling 2 every 2 tiles, range 4.
- Art (Zeus_General): tree tiles use SPR_BOUL_SIDE_SOUTH/NORTH/EAST/WEST (9 images each) and the corner groups (images 88–131 in one block). Road tiles use SPR_BOUL_ROAD (images 39–87).
- How the art is picked (`0x004193A0`, `0x00419190`, `0x00418A20`, `0x00418CF0`): each tile's 8 neighbours are marked if they are road or Avenue/Boulevard, then matched against the 47-piece table at `0x005E92B8`. A tree tile maps the matched row's image number for the current view to an art index: 31 → 0, 23 → 9, 27 → 18, 19 → 27, 11–14 → 36–39, 35/37/39/41 → 40–43. A tree tile with no art index, or with road on both sides along a line, becomes road.
- eZeus (applied): the Boulevard is a new building type; the build tool follows the rules above. eZeus keeps road tiles as separate roads and puts only the tree tiles in the Avenue/Boulevard. Kept from eZeus: trees can't go over another road; a drag that doesn't start on or next to a road builds nothing. The original instead picks a side from the drag direction (the decompiled comparison is unclear).
- Table rows (`0x005E92B8`, 20 bytes each; the mask at +0x10 is built from the 8 bytes by `0x004A6BF0`): per neighbour 1 = road or Avenue/Boulevard, 0 = neither, 2 = either; then one picture per view. Rows 8 and 9 have the same pattern, so a tile with only the left and right corners filled falls to the last row (picture 15). Picture 15 adds a random 0-3 (`0x004193A0`); other rows add 0.
- Straight sides (`0x00418CF0`, tree pictures 0, 9, 18, 27): the tile before (x − 1 or y − 1, along the line) and after decide the variant. 0, 1, 2, 8 stand alone (tile random & 3); 5 starts a stretch, 4, 6, 7 continue it (random & 7: 0 → 4, 1 or 7 → 6, 2 → 7, else end) and 3 ends it. After a stretch ends, a new one starts only if the tile random % 9 is 3–7. In the other views a byte table (`0x005E9664`) swaps 5 and 3, and `0x00418980` / `0x005E9674` remap the pictures for views 2 and 6.
- Applied 10 Nov 2026: eZeus now uses this table, the tree map, the road-between-roads rule and the straight-side variants for Avenue and Boulevard tree tiles, and the same table for the road tiles beside them (this also fixes Boulevard roads, which used plain road pictures). eZeus works out the pictures from what is on screen as if in the default view, so the original's extra remaps for the turned views aren't copied. A tree tile shown as road stays a tree tile for walkers and building (the original turns it into real road). eZeus keeps road tiles as separate roads, so any road tile touching an Avenue/Boulevard tile gets these pictures, not only the piece's own road.
- Not applied: the scenario on/off switch (eZeus has none for either).

## Pan-Hellenic games (verified in code and eventmsg, applied 10 Nov 2026)
- Which game: (year, or the next year after month 8) & 3: 0 Olympian, 1 Isthmian, 2 Nemean, 3 Pythian (the begin messages 0x4B0, 0x488, 0x474, 0x49C follow eventmsg's order Nemean, Isthmian, Pythian, Olympian). eZeus has the same order in its own year count (not changed).
- Building counts (`0x00782888`, per type, buildings with workers): college 77, drama school 78, gymnasium 79, stadium 80, podium 81, theater 82.
- Taking part (`0x00565A80`): Isthmian needs a working college, Nemean a working gymnasium or stadium, Pythian a working drama school, Olympian any of the four. When a flag in the games record is set, the Olympian year needs a working stadium instead (possibly Poseidon's Home Games; not traced).
- Coverage (`0x00566A90`): the share of people whose house has a philosopher (+0x6d), athlete (+0x6f) or actor (+0x71) visit, 0-100.
- Score (`0x00565D40`): Isthmian = philosophy / 4 + colleges + podiums; Nemean = athletics / 4 + 5 x stadiums + gymnasiums; Pythian = drama / 4 + drama schools + theaters; Olympian = (the three) / 3 + 4 x stadiums. Apollo's sanctuary adds 20 to each (`0x004E9440`).
- Hercules (`0x00565E30`, hero record 1 at `0x00F0E504`): while his hall stands, he hasn't come and his games requirement (6) isn't met, a non-zero score becomes score x score x 0.16 (`0x005E4800`) for the roll.
- Result (`0x00565E90`): a random 0-99 below the score wins; otherwise a score of 30 or more is second, lower is lost.
- Culture advisor (`0x00519BD0`, Zeus_Text group 58): score under 5 terrible, under 10 poor, under 20 ok, under 30 not bad, else good.
- Applied: eZeus uses these rules for Greek cities; the Hercules boost applies while his hall stands and the city has won no games. Its June begin / August end timing is kept; the original rolls when the games begin. The Poseidon home games aren't traced.

## Scenario "allowed structures" switches (verified in code and Zeus_Editor_Text.eng, 11 Nov 2026)
- The scenario holds 57 on/off values, 2 bytes each, at `0x00902D32` (copied in with the scenario block `0x00902D30`-`0x00902DF8`, all set to 1 for a new scenario). The editor lists them under "Allowed structures" in this order: Mint, Hunting Lodge, Olive Press, Grand Agora, Fishing Wharf, Corral, Granary, Common Agora, Storehouse, Trade Buildings, Stadium, Theater, Gymnasium, Winery, Sculpture Studio, Podium, College, Fountain, Horse Ranch, Drama School, Maintenance, Infirmary, Tax Office, Watchpost, Palace, Hero's Hall, Road Block, Water Crossing, Column, Parks, Avenues, Boulevards, Walls, Towers, Gatehouse, Bench, Gazebo, Flower Garden, Hedge Maze, Fish Pond, Armory, Trireme Wharf, Elite housing, Hippodrome, Chariot Factory, Tall Obelisk, Sundial, Topiary, Spring, Stone Circle, Short Obelisk, Water Park, Leaping Dolphin, Orrery, Shell Garden, Baths, Bird Bath.
- The build menu (`0x00402B80`) greys out a building when its value is 0. Model types per switch (0-based): 0 mint (and the silver mine button with it), 1 hunting lodge, 2 olive press, 3 grand agora, 4 fishing wharf (not read by the menu), 5 corral (any city in Poseidon, with cattle on the map), 6 granary, 7 common agora, 8 storehouse, 9 trade buildings (read elsewhere), 10 stadium / museum, 11 theater / laboratory, 12 gymnasium / bibliotheke, 13 winery, 14 sculpture studio, 15 podium / observatory, 16 college / university, 17 fountain, 18 horse ranch, 19 drama school / inventors' workshop, 20 maintenance office, 21 infirmary, 22 tax office, 23 watchpost, 24 palace, 25 hero's hall, 26 roadblock, 27 bridge, 28 doric, ionic and corinthian columns, 29 park, 30 avenue, 31 boulevard, 32 wall, 33 tower, 34 gatehouse, 35 bench, 36 flower garden (type 15), 37 gazebo (type 16), 38 hedge maze, 39 fish pond, 40 armory, 41 trireme wharf, 42 elite housing, 43 hippodrome (Atlantean cities), 44 chariot factory (also needs switch 18), 45-51 tall obelisk to water park and 52-56 leaping dolphin to bird bath (Poseidon only).
- Switches 36 and 37 are the other way round from the editor's labels: the game's "Gazebo" line greys out the Flower Garden and vice versa.
- Many buildings also need something on the map (the menu checks the city's resources), e.g. hunting lodge, workshops, armory, corral.
- Before 11 Nov 2026, eZeus (`readEpisodeAllowedBuildings`) read 36 as gazebo and 37 as flower garden, skipped 25, 26 and 28, used 5 only for Atlantean cities, and stored only 12 switches (the rest were always available).
- Applied 11 Nov 2026: eZeus maps all 57 as above. Buildings without a field of their own, and the hero's halls, are kept in eAvailableBuildings::fSwitchedOff, and the roadblock in fRoadblockOff (saved since eFileFormat::allowedSwitches; older saves and campaigns allow everything). The build menu, including the palace, stadium, museum and trade buttons and the roadblock button, respects them; the chariot factory also needs the horse ranch switch; eZeus's scenario editor lists the new switches. Not copied: the original's extra checks for map resources and for the Poseidon expansion.

## Hippodrome (verified in code, table `0x006429F0`, 5 numbers per length band)
Bands 4–10, 12–22, 24–34, 36–58, 60–98, 100–198, 200+: mood +1, +1, +1, +1, +1, +2, −5 at each mood update while working (`0x0056FDC0`); income 10, 20, 40, 60, 100, 200, 500 and horses 4, 4, 8, 8, 12, 12, 24 (as the site says); two more columns (0,1,1,1,2,3,0 and 0,0,0,1,1,3,15) not identified.

## Debt
The month counter rises each month in debt and resets when the treasury is positive; every 12 months a message is shown and the year counter rises (`0x00483B20`). After 36 months in debt the episode is lost (goal check `0x00526990`, message 0xB91). Applied 9 Nov 2026 (Nicolás: yes): eZeus now warns every full year in debt and loses the episode at 36 months.

## Apollo's blessing (verified in code, `0x004C5690`)
Cures every sick house, removes the plague walkers and raises city health by 50.

## Guardians
The Dragon and Cerberus use the single "Monster" row (500 HP, attack 50, armor 10).

## Fire and collapse (verified in code, `0x00492260`, step 44, every cycle; applied 9 Nov 2026)
- Damage counter += Model damage risk (column h) each cycle (×11 while burning); reset for houses with at most 1 resident, the vacant elite type, and terrain 0x10000; collapse at 1000. With damage risk 10: 100 cycles, about 6 months (the site's "60 ÷ risk months").
- Fire counter: on a 1-in-8 draw, += Model fire risk (column g) × random 1–5 (+3 more during a fire disaster mode); empty houses and buildings with no fire risk reset it; fire at 1000.
- The maintenance walker (figure 8, `0x004A5D90`) lowers both counters by 50 (Model row 121 BUILD_SUPER_OFFICE column j) for buildings within 2 tiles each time he passes.

## Culture and healer visits (verified in code, applied 9 Nov 2026)
- Step 35 (`0x00484D20`) lowers each house's access counters by 1 every cycle (house bytes +0x6c, +0x6d, +0x6f, +0x70, +0x71, +0x77, +0x78).
- A philosopher or other culture walker (`0x004A60E0`) and a healer (`0x004A66B0`) set the counter to 96. A visit therefore lasts 96 cycles, 6 months.
- Step 36 (`0x00484DE0`) adds up the culture points from those counters: Greek 20/15/20/25, Atlantean 20/20/15/25, plus 10 for the bonus building.
- In eZeus: a culture visit fills 8 steps of 1/16 year each (6 months). Healer care (100) now drops by 1 a cycle (about 6 months) whatever the food and water.
- Fixed an eZeus bug: small houses counted the culture timer twice, so visits wore off in half the time.

## House food and goods (verified in code, applied 9 Nov 2026)
- Food, monthly (`0x0048B8D0`, run at month end): a house eats population x 25 / 100 units (`0x004E5EF0` returns 25), split over the number of food types it needs (Model column i). Huts need none and eat none.
- Horses: a house with any horses eats 1 more unit a month, whatever the number of horses. If no food is left for them, it loses that unit's worth of horses (1).
- Fleece, oil, wine (`0x00484C50`, step 39 `0x004FE290`, on cycles 0 and 7 of each month): each needed good drops by the Model value (1), so 2 a month. eZeus already matched this, except that a devolved elite home kept using goods.
- The house needs check (`0x00484660`, status codes 1-29) compares the house bytes and shorts with the Model columns. The record is +0x1c people, +0x12 level, +0x64 wine, +0x66 oil, +0x68 fleece, +0x6a arms, +0x72 horses, +0x80 culture points.

## Service walkers (verified in code, `0x004DFB70`, applied 9 Nov 2026)
- The figure numbers follow the figure name list in Zeus_Text: 7 Clerk (tax collector), 8 Superintendent, 15 Competitor, 16 Philosopher, 17 Athlete, 18 Actor, 26 Peddler, 32 Healer, 60 Water Bearer, 61 Watchman, 72-75 Scholar, Astronomer, Inventor, Curator.
- Reach: every service scan (`0x0040D160` peddler, `0x004A5D90` superintendent, `0x004A5F70` watchman, `0x004A60E0`-`0x004A66B0` culture, water and healer, `0x004A67C0` tax) covers houses up to 2 tiles from the walker, a 5x5 square. eZeus used 1 tile.
- Visit counters set to 96 cycles (6 months), lowered by 1 a cycle in step 35. Competitor and Curator set +0x6c, Philosopher and Astronomer +0x6d, Athlete and Scholar +0x6f, Actor and Inventor +0x71, Water Bearer +0x77, Healer +0x78. A tax collector sets +0x4e to 50.
- Culture points (`0x00484DE0`): Greek competitor 20, philosopher 15, athlete 20, actor 25. Atlantean curator 20, astronomer 20, scholar 15, inventor 25. eZeus had inventor and astronomer swapped.
- Model house columns as the getter `0x00433D80` reads them (21 per row): 0 devolve appeal, 1 evolve appeal, 2 culture, 3 water, 4 stadium/museum (Estate only: needs a competitor or curator visit, `0x004843D0`) [not applied: Nicolás chose culture points only for the Estate, 9 Nov 2026], 5 soldiers %, 6 max horses, 7 horses needed, 8 food types, 9 fleece, 10 oil, 11 wine, 12 arms, 13 max armor, 14 crime increment, 15 crime base. The file header's enum numbers skip 4, so they are one off from here on.
- House water flag (+0x35, "water sup"/"well sup"/"no water" in the debug view, a Caesar III leftover): Nicolás confirms that in Zeus a common house gets water only from fountain walkers. Being near a fountain does nothing, and water works like healer visits. eZeus already does this (a water bearer's visit lasts 6 months). There are no wells in Zeus.

## Walker roaming distance (verified in code, 9 Nov 2026; no change needed)
- Each roaming walker's action sets a limit at figure +0x42 and counts up +0x44 until it reaches the limit, then heads home. The limits are: culture walkers (`0x004B4370`) and tax collector (`0x004AF550`) 3072; healer (`0x004B78A0`) and water bearer (`0x004C0BC0`) 2304; peddler (`0x004B6720`), superintendent (`0x004AF930`) and watchman (`0x004C0CF0`) 3840.
- Units: the counter grows by the walker's speed each tick (`0x004D6790`), and a walker moves speed/6 sub-steps a tick, so 6 per sub-step. A tile is 15 sub-steps (`0x004D69B0`), so 90 per tile.
- In tiles that is about 34, 26 and 43. eZeus uses HeavenGames' measured 35, 27 and 44 with the same groups. They agree within a tile, so these are kept.

## Workshops and corral (verified in code, step 31 `0x0048CFB0`, 9 Nov 2026)
- Shared workshop rule: each cycle, progress (+0x54) grows by 10 at full staff, 8 at 75%+, 6 at 50%+, 4 at 25%+ and 2 below that. At 320, 100 raw (1 load) becomes 100 goods, so 32 cycles (2 months) a load and 6 a year at full staff. This matches eZeus's HeavenGames value. eZeus scales smoothly with staff instead of in these steps (not changed).
- Output storage caps by building type: 200 by default; winery, oil press and others 300; sculpture studio 400; 500 and 800 for some types; corral 600.
- Corral (`0x004F9E90`): a slaughter takes 16 cycles (1 month) and gives 200 (2 loads). The next cow leaves the pen once 8 cycles have passed and the slaughter is done. Applied: corral_processing_period = 1 month, corral_kill_period = half a month.
- Corral fetching (`0x004F9960`): it tries to send a Rancher for cattle every 1, 4, 8, 16 or 32 cycles depending on staff (100%, 75%, 50%, 25%, less). At most 2 are out, and cattle held plus out stay under 3. Applied 9 Nov 2026: eCorral keeps at most 2 fetchers out (not saved) and uses these waits.

## Service building walker timing (verified in code, step 31 `0x0048CFB0`, applied 9 Nov 2026)
- Service buildings send one walker at a time. Once it's back, a counter (+0x32) counts cycles until the next leaves. The wait depends on staffing (full / 75%+ / 50%+ / 25%+ / any):
  - tax office, superintendent's office, guard post: 1 / 2 / 4 / 8 / 16 cycles
  - fountain: 2 / 4 / 8 / 16 / 30
  - infirmary, theater, stadium: 4 / 8 / 16 / 30 / 45
  - podium: 9 / 16 / 31 / 51 / 101
  - gymnasium: 5 / 6 / 11 / 18 / 31
- Agora (`0x0040D620`): 3 / 4 / 5 / 6 / 11 cycles, staffed by its stalls. eZeus agoras have no staff of their own, so they use 3.
- Culture venues allow 1 walker out at a time, 2 if blessed. Atlantean venues match their Greek counterparts: bibliotheke (`0x00432960`) like the gymnasium, observatory (`0x00432B00`) like the podium, laboratory (`0x00432E60`) and museum (`0x00433170`) like the theater and stadium. University and inventor's workshop (trainers) are not changed.
- Agora deliveries (`0x0040D160`): food up to residents / 2 per type each visit, capped at 2 x residents. Goods up to Model need x 4 (2x2 or larger) x 2 (x2 more for elite homes): 8, or 16 for elite. This matches eZeus.
- Superintendent (`0x004A5D90`): lowers fire and damage risk by Model row 121 column j (50). This matches eZeus.
- Workshops (winery, olive press, sculpture studio, armory) now use the 10/8/6/4/2 staffing steps (see Workshops above).
- Taxes (`0x00488250`, monthly): a house the tax collector has visited pays rate x people x Model column 18 (tax multiplier) x 2 x 50/100 (the difficulty factor `0x005F7F1C` is 50 on every difficulty) / 100. That is rate x people x multiplier, the same as eZeus. A blessed tax office's collector marks houses to pay x4 once (`0x004A67C0` flag +0x84). Applied: a blessed tax office sends one blessed collector, whose houses pay 4 times their tax (the flag is not saved, so a game reloaded mid-round loses it).
- Sheep and goats (`0x004F8D80`): wool +9 a month, ready at 100 (a year). A shearing gives 100 (1 load). This matches eZeus.

## Rescue cash: debt relief from allies (verified in code, applied 9 Nov 2026)
- Each month on cycle 12 (`0x00483C10`), `0x0047EE20` scans the world cities. A city is a candidate when the player's treasury is below 0, the city's attitude (+0x164) is 50 or more, it hasn't helped before (+0x175), and its relation (`0x0042CF10`) is code 0 or 3. One candidate is picked at random. Code 2 has an attack check, so it's the rival. Codes 1 and 4 share text with each other (`0x0042D8A0`), as vassal and colony (the tribute payers), which leaves 0 and 3 as ally and parent city.
- Amount (`0x0042D870`): (wealth + 1) x difficulty factor (`0x005EC648`: 5, 4, 3, 2.5, 2 from Beginner to Olympian) x 200 (`0x005D9090`).
- Accepting (`0x00474A80`, offer type 0x17) adds the money, marks the city +0x175 so it never offers again, and lowers that city's attitude (+0x164) by 10. The branch is chosen by flag 0x10 in the offer, which the rescue offer always sets (`0x0047F100` writes 0x110); without it the same code is an ordinary gift (message 0x262 instead of 0x2BC) with no attitude change.
- Applied: eBoardPlayer::rescueCash offers it as a cash gift shown with the original's own rescue messages (eventmsg "rescue_cash": "A helping hand from [city_name]", accepted, postponed, refused; since 10 Nov 2026) and marks the city when it offers. A refused offer therefore isn't repeated. Since 10 Nov 2026 accepting it also lowers the giver's attitude by 10. The new world-city save field is gated by eFileFormat::worldCityRescue.

## Building storage limits (verified in code, applied 9 Nov 2026)
- Cap table by building type (`0x0048A880` and the production step `0x0048CFB0`), in units (100 = 1 load): sheep and goat farms 500; foundry (type 50; the Model file lists it as "10"), orichalc refinery 500; mint, winery, oil press, armory, warship wharf (and type 81) 300; marble quarry, horse ranch, black marble workshop 800; sculpture studio, chariot workshop 400; corral 600; tax office 100000; everything else 200.
- `0x0048A880` is "can this building take a delivery of X": a workshop accepts raw material only while its raw stock (+0x3c) is under the cap. Raw-material buildings compare their own stock with the same cap before adding more. Timber mill, hunting lodge and urchin quay use a fixed 500 in their own code. Sheep and goat shearing (`0x004F8050`) and the corral (`0x004F9E90`) cap their stock the same way. Fisheries have no stock cap beyond the boat's single load.
- Applied (eZeus used 8 loads for all, and 4 raw for workshops): timber mill, foundry, refinery, hunting lodge, urchin quay, carding shed, dairy 5; mint 3; corral 6; marble quarry and black marble workshop stay 8; winery, olive press and armory accept 3 loads of raw material (sculpture studio stays 4).
- A blessed batch (4+ loads at once) may exceed the cap by 3. The original sends a blessed batch straight out on a cart (`0x0048D3DA` area).
- Workshop output (`0x0048CFB0`, types 0x35 winery, 0x36 oil press, 0x37 sculpture studio, 0x87 armory; the same code also serves onion and carrot farms): the workshop keeps no finished goods. When a batch is done (progress at `0x004855D0`) and no cart of its own is out, it resets the progress, uses the raw material (`0x00485610`: 100, a statue 400) and sends a cart with 100 units (a statue 400; a blessed batch x4, at least 400). While its cart is out nothing more leaves. Applied 11 Nov 2026: eZeus workshops hold at most 1 finished load (was 8), and a finished batch waits for room instead of losing its time. Not traced: where the progress itself is advanced.

## Adventure event types eZeus didn't read (verified in code, `0x0046DFD0`, applied 9 Nov 2026)
- The event dispatcher switches on the event type byte. 6, 7, 10 and 11 share `0x00472EC0`, which only marks the event done and sets off its linked event. 12 (`0x004733F0`) only marks it done. 17 (`0x0046E840`) raises a city's favor by the event amount (`0x0042CED0`), with message PHRASE_favor_change_*_I. 18 (`0x0046E970`) lowers it, with message *_D.
- Applied: new eFavorChangeEvent (save type favorChange, appended); adventure events 17 and 18 load as favor increases and decreases. 6, 7, 10 and 11 load as a favor change of 0, which does nothing but fire the linked events. 12 is skipped. Also fixed: an unreadable event no longer shifts the numbering, so linked events stay matched.
- Details for the military service factor: `0x004DDD70` counts companies (types 11-13: rabble, hoplite, horseman) whose "called out" flag is set. `0x00483C10` counts months in a row with no enemy soldiers (figures 40-42) or ships (63-64) on the map (`0x00FE9578`). It also tracks months until the next invasion (`0x00FE957C`), and the factor applies only when both are 2 or more. eZeus counts banners that aren't at home, peace months as months with no active invasion (saved, version cityPeaceMonths), and treats an invasion due within 60 days as too close.
- Tax coverage under 11%: the city mood also uses the "no taxes" row (applied 9 Nov 2026; eZeus had 5%, and only for houses).

## Enemy warships (verified in code, applied 9 Nov 2026)
- Figure 63 is "Enemy Transport" and 64 "Enemy Warship" (Zeus_Text figure names). Their actions are `0x004C24F0` and `0x004C2980` (figure action table `0x0063969C`, 10 ints per type).
- Spawning (`0x00503FD0`, sea invasions, invasion points 9-16, chosen in `0x00504600`): the number of warships is the event's byte 0x36. Event records in memory start 2 bytes before eZeus's reading (the type is at memory byte 2), so this is the field eZeus reads as godMonsterHeroId. Oceanid invasions (nation 8) bring none. The n-th warship waits 25n + 10 ticks, then sails to the river entry. It also records the current month and nation. Special event record 0x8c (`0x0091B7C4`, not a campaign event) gets soldiers / 4 warships instead; what triggers that record wasn't traced, so it isn't applied.
- Stats: the Model "Ship1" rows ("Non Player Greek Trireme", "<Nation> Warship"), the same at every difficulty: HP, attack, armor, armor vs missiles, missile attack, range, rate, speed. They are read into `0x007852A0` (stat getter `0x004E1B20`, column 6 = range, 7 = rate).
- Behaviour, by state (the original's figure ticks; about 13 eZeus ticks each):
  - 6: waits out the start delay.
  - 14 (`0x004D4500`): patrols. From the river entry it sails to the first landing point, from there to the river exit, otherwise back to the entry. After 75 + random 0-29 ticks it hunts.
  - 8 (`0x004D3F90`): hunts with target finder `0x004E1830` over the whole map. Scores: player rabble, hoplite or horseman (11-13) 100, only within missile range. Trade ship (20) or fishing boat (25) 10. Trireme (57) 500, or 1000 if that trireme is attacking a transport, less 10 for each figure already targeting it (+0x7F0). The highest score less distance wins. Against soldiers it stays put; against boats it sails until within 3 tiles. It chooses again every 24 ticks, or when the target leaves range. With no target it patrols. Taking on a soldier or trireme has an even chance of moving its departure month to next month.
  - Every moving state fires at the best target within range when its counter passes the rate (`0x004D47D0`; missile figure 52).
  - Below 16% hit points (state 10, `0x004D3DA0`): every 75 ticks it finds the nearest trireme or fishing boat (`0x004D0A10`, 18 x 18 tiles) and moves 10 tiles away from it.
  - Leaves (state 15, `0x004D4730`) through the river exit when the month is 4 after its recorded month, and disappears.
  - It never targets buildings.
- Looks: art set `0x006383C8` for nation 0 (Greek), otherwise `0x006383BC`. This matches the Citadel: a Greek warship looks like a trireme, the others like the sailed ship eZeus uses for the transport (SprMain 4833).
- Peace months for the military mood (`0x00483C10`) also reset while enemy ships (63-64) are on the map.
- Applied: eEnemyWarship and eEnemyWarshipAction (character type enemyWarship and action type enemyWarshipAction, both appended). Adventure invasions read the warship count (saved, version invasionWarships). Warships spawn with sea invasions. The bribe adds 200 x the difficulty multiplier per warship: 200, 600, 800, 1000 and 1400 (Citadel). Player triremes and soldiers treat warships like other boats. Enemy warships reset the peace-month counter.
- Not applied: the editor has no warship count field yet, and the 0x8c record rule above.

## Sanctuary priests and sacrifices (verified in code, applied 10 Nov 2026)
- Figure 65 is "Priest" (Zeus_Text figure names); his action is `0x004CA350`. Art: SprMain 8105-8427 (walk 12 frames, die 8, lifting a goat 18 frames then carrying it, lifting a sheep 9 frames then carrying it). Leading cattle uses Poseidon art (0x6A6E) that eZeus doesn't load, so eZeus shows the normal walk there.
- Sending priests (`0x004E97D0`, run each cycle, 16 a month, from the sanctuary step `0x004E94C0` while the sanctuary has workers): a byte timer (+0x32) goes down by 1 (0 wraps to 255; above 24 it goes back to 22, `0x004E9750`, which also runs when the sanctuary is finished). While it is below 20, the sanctuary sends a priest if it has its full staff (Model EMP, `0x004E9770`), has an altar, and has fewer than its limit out (`0x004EA330`: 3 for Zeus, Poseidon, Hades, Hera; 2 for Demeter, Apollo, Artemis, Athena, Atlas; 1 for Ares, Aphrodite, Hermes, Hephaestus, Dionysus). Sending the priest that fills the last slot clears the altar's sacrifice kind (+0x9F).
- The priest (`0x004E9930`, `0x004E9D90`) waits a tick, then takes the first sheep (49), goat (50) or bull (cattle 76 with subtype 2) in the figure list that no other priest has claimed; failing that any other cattle (subtype 3: the cow and calf stages); failing that the first granary (57), storehouse (58), pier (61) or trading post (62) with at least 400 food (4 loads, `0x004EA060`); failing that he disappears. If the animal is gone or the store is short when he gets there, he looks again after 11 ticks.
- At a sheep or goat (`0x004E99F0`) he removes it and plays the lifting animation, then carries it to the altar's road. A cow (subtype 3) follows him. A bull first stands facing him for 31 ticks (`0x004CAA60` state 0x15), then follows. At a store he takes 400 food, from the food types in order (`0x004E9E50`), and a cart follows him (eZeus draws the cart in front of him, like its cart pushers).
- At the altar (`0x004EA140`, `0x004EA1A0`): the count of priests on their way (+0xA1 on the altar) drops by one. He sacrifices once that count is below 1 and no sacrifice is being shown (+0x50 on the sanctuary): kind 1 sheep or goat, 2 food, 3 cattle (the eZeus altar overlays sheep, goods, bull). The sanctuary timer is then set to 24, and the priest and his cow or cart disappear. A sacrifice shows until the kind is cleared (see above); the count resets to 0 once it reaches minus the limit. A priest removed before reaching the altar takes one off the count (`0x004EA2B0`).
- The sanctuary panel (`0x005813F0`, text group 132) shows only: no workers, the four understaffed texts ("No sacrifices can be conducted"), priests out looking, conducting sacrifices, not looking right now, or "No suitable sacrifices are available" (timer below 19). The texts "This Sanctuary is inactive. Sacrifices must be performed here for the Sanctuary to function" and "This Sanctuary is active for another..." are never used, and nothing else found turns a sanctuary off. So the manual's "you will lose the benefits that the Sanctuary provides" isn't in the code; not applied.
- Not traced: the bull's own standing animation (0x6A3E), the cart's figure type, and a flag (+0xDA) that lets an understaffed sanctuary send priests.
- Applied (eSanctuary, ePriest, ePriestAction): the timer, limits, choice order, food taking and the turn rule as above, saved with eFileFormat::sanctuaryPriests. eZeus's old altar timer (a sacrifice every 100000 ticks shown for 25000, picking the kind by whether any bull or sheep existed) is gone. The priest's info panel uses his lines from text group 237.
