# Findings from the original Zeus.exe

Read with Ghidra 12.2 from Nicolás's Steam `zeus.exe` (same build as GOG: compiled 2002-01-04 11:03, same size; checksum differs in a few bytes). Function names and the address map come from zeus-poseidon-arm64 (`analysis/symbols.tsv`). Tools live in `C:\Users\LENOVO\source\repos\re-tools` (see `gh.ps1`; decompiled output in `work\out`). "Cycle" = one of the 16 simulation steps per game month.

## Crime (verified in code)
- Each common house (types 2–8) has a crime level, 0–100. Elite houses have none.
- **When a house is built**, its crime level starts at the house table's "crime base" (Model column p).
- **Twice a month** (cycles 0 and 8; `0x005671A0` → `0x00566F20`), once the city has 350 or more people: crime += crime increment for the house level (column o) + (40 − city mood) / 2 (integer division). Then it is floored at the crime base and capped at 100.
- **Watchmen** (figure 61, `0x004DFB70` → `0x004A5F70`): every house within 2 tiles of the watchman loses the guardpost's "Risk Reducer" (Model `BUILD_GUARDPOST` column j = 50 on every difficulty), floored at the crime base. There is no persuasion: the watchman just lowers crime around him.
- **Outlaws** (`0x005BFDE0`, every cycle): for each threshold a house's crime level reaches, a 1-in-200 draw spawns that walker, if the house has none out already: 70 → disgruntled citizen (figure 22), 80 → mugger (27), 90 → thief (23), 100 → vandal (29). Table at `0x0065E898`.
- **Unidentified condition** (`0x004E9440`): when a particular god's sanctuary (building types 84–97, chosen through an object not yet identified) is in the city, houses need 10 less appeal to evolve and stay, and crime falls by 20 every update instead of rising.

## City mood (verified in code, `0x005671A0`, twice a month)
- Starts at 60, range 0–100. Each update adds the sum of six factors:
  - **taxes:** the tax-sentiment table per difficulty (already in eZeus);
  - **wages:** none −10, very low −5, low −2, normal 0, high +2, very high +4 (table `0x00641F20`);
  - **unemployment:** under 5% +1; 5–10% 0; 11–17% −1; 18–25% −2; 26% or more −3;
  - **food:** the average over houses of +2 for a fed house, −1, −2, −3 for a house unfed 1, 2, 3+ updates in a row;
  - **debt:** −2 after the first month in debt, −5 after two, −9 after three or more, and −2 more while the treasury is negative;
  - **a sixth factor** (`0x00566E70`) from counts kept by `0x004DDD70` (figures of types 11–13), −1 to −5; not identified yet.
- Below 350 people, negative factors count as 0.
- Drift toward the middle: mood under 11 +4, 11–20 +3, 21–30 +2, 31–40 +1, 41–60 0, 61–70 −1, 71–80 −2, 81–90 −3, above 90 −4. Below 41 the drift only stops a fall, it never pushes the mood up past it; above 60, likewise for a rise.

## Houses
- A house devolves only after failing its needs on 3 checks in a row (12 for the elite type at `0x004FE900`); a passing check resets the count.

## Appeal (from zeus-poseidon-arm64 docs/engine/terrain.md §7, applied 9 Nov 2026)
- Spread ring by ring from the footprint, at most 6 rings; the footprint itself gets nothing; each tile clamped to −100..100.
- A building's appeal is the highest over its footprint (+10 when an unidentified flag is set).
