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
- A building's appeal is the highest over its footprint (+10 when an unidentified flag is set).

## Outlaws' behaviour (verified in code)
- Action table per figure type at `0x0063969C` (40 bytes each): disgruntled `0x004B5A20`, thief `0x004B5B70`, mugger `0x004B68B0`, vandal `0x004B6A00`.
- **Thief** heads for an elite home (`0x005C0630`). There he takes, from each good, a quarter of the stock or half the level's Model storage number if larger (food: column q, horses: g, armor: n), shows message 19:167 and leaves (`0x005C00C0`).
- **Vandal** heads for an elite home, stands there for a countdown of 51 steps, then destroys it and shows 19:168 (`0x005C0360`).
- A thief or vandal who finds no elite home leaves, and his house sends a **mugger** instead (`0x005BFF90` with figure 27).
- **Mugger:** when his attack countdown ends, the victim is removed (dies); no chance roll found.
- **Disgruntled citizen:** only roams.

## Sixth mood factor (likely military service)
`0x00566E70` counts the city's rabble, hoplite and horse companies (figure types 11–13) carrying a flag (probably out of their forts or away), once the city has had 2+ months of peace: 0 → 0, 1–2 → −1, 3–5 → −2, 6–8 → −3, 9–12 → −4, 13+ → −5.

## Plague
The plague walker (figure 66) calls an empty function on each tile, so in the original it spreads nothing as it walks.

## Plague outbreaks (verified in code, `0x00447FF0`, `0x004481B0`, `0x00448490`, `0x00448220`)
- City health starts at 80. Each cycle a 4-digit random draw (0–9999) against a chance from the health band (table `0x005F84A8`): health under 10 → 300 (3%), 10–19 → 250, 20–29 → 200, 30–44 → 100, 45–59 → 50, 60–74 → 25, 75–99 → 0.
- Outbreaks are capped: at most ceil(chance × 384 / 10000) counted over a period (and half that by one counter), and none while one is running.
- An outbreak picks up to 40% of the lived-in houses, those with the highest disease score (a per-house value, +20 when a flag is set), and infects them: each loses part of its people and sends out 3 plague walkers (timer 600). How the per-house score grows was not traced.

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
- Not yet read: Artemis (`0x0056AF80`, `0x00567C70`), Ares (`0x0046DAA0`), Hades (`0x00546E80`), Zeus.

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
- Roads on slopes use the paved image when the tile's appeal is above 4 (`0x004A0480`, called from the slope road picker `0x0045E110`); eZeus uses the same rule for all roads. Flat road selection was not traced.
- Plain ground picks one of four looks by appeal: below 0, 0–15, 16–29, 30+ (`0x004A0450`, `0x004A0190`). Not in eZeus.
- Appeal view colour (`0x0044BCF0`): −11 or less → 0; −10 … 69 → appeal/10 + 2; 70+ → 9; a tile at 0 with no building is not coloured.

## Hippodrome (verified in code, table `0x006429F0`, 5 numbers per length band)
Bands 4–10, 12–22, 24–34, 36–58, 60–98, 100–198, 200+: mood +1, +1, +1, +1, +1, +2, −5 at each mood update while working (`0x0056FDC0`); income 10, 20, 40, 60, 100, 200, 500 and horses 4, 4, 8, 8, 12, 12, 24 (as the site says); two more columns (0,1,1,1,2,3,0 and 0,0,0,1,1,3,15) not identified.

## Debt
The month counter rises each month in debt and resets when the treasury is positive; every 12 months a message is shown and the year counter rises (`0x00483B20`). After 36 months in debt the episode is lost (goal check `0x00526990`, message 0xB91). Applied 9 Nov 2026 (Nicolás: yes): eZeus now warns every full year in debt and loses the episode at 36 months.

## Apollo's blessing (verified in code, `0x004C5690`)
Cures every sick house, removes the plague walkers and raises city health by 50.

## Guardians
The Dragon and Cerberus use the single "Monster" row (500 HP, attack 50, armor 10).

## Fire and collapse (partly read, `0x00492260`, step 44, every cycle)
Per building (not on fire): a damage counter grows by the Model damage risk (column h) each cycle; it is reset for some houses and for tiles with terrain flag 0x10000. While below 1000, a 1-in-8 draw adds Model fire risk (column g) times a second factor to a fire counter; past 999 the building catches fire (with a message, rate-limited). Not applied: the second factor and the collapse branch need a closer read.
