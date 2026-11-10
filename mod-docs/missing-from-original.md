# What eZeus still lacks compared with the original

Status on 9 Nov 2026, after Phases 0-7 and the zeus.exe reading. Details and addresses for anything taken from the exe are in `original-exe-findings.md`.

## A. Features the manuals or sources describe that eZeus doesn't have

| # | Missing | Source | What it would take |
|---|---|---|---|
| A3 | Boulevard (3 wide; cost 15/24/30/36/45; appeal 3, step size 2, -2 a step, range 4). eZeus's Avenue also differs: the original's is 2 wide | Model BUILD_AVENUE (118) and BUILD_BOULEVARD (126), building names 118 and 126 with their own descriptions, zeus.exe: separate allowed-building switches (`0x00402B80`), placement preview `0x00419760` (width = 3 for type 126, else 2; the cursor tile must be a road), appeal from every tile (`0x00493C80`). Art: Zeus_General SPR_BOUL_* groups, which eZeus already uses for its Avenue | Trace how the original lays the 2 or 3 tiles along a road (which become trees, what happens to the road, cost per tile) before building it; then rework eZeus's Avenue to match |
| A5 | Hotkeys not added: Shift+1-4 company bookmarks and 1-4 to select them; Tab for the overview map; M "freeze footprint"; F5-F7 window/resolution; pressing 1 twice for the full population report | Citadel: hotkeys | Overview map doesn't exist in eZeus; "freeze footprint" meaning unknown |
| A6 | Cosmetic cheats: Bowvine and Arrow (towers shoot cows), Cheese Puff (dairy workers in cheese suits), Mammaldrome (animals race in the hippodrome) | Citadel: cheats | Each needs art swaps |
| A7 | Apollo's sanctuary: +20 to the Pan-Hellenic games score | zeus.exe `0x00565D40` | eZeus's games use their own winning chance; the original's games scoring wasn't traced |

## B. Described by the sources, but the numbers or the rule weren't found

| # | Missing | What is known | What wasn't found |
|---|---|---|---|
| B1 | Demeter's invasion "renders your land infertile" | Oracle and invasion messages | No code found; eZeus kills half the sheep and goats only |
| B2 | Hera's invasion "turns your citizens into cows" | Invasion message | No code found; eZeus kills half the cattle and orange trees and lowers attitudes by 20 |
| B3 | Zeus' sanctuary improves your standing abroad | Sanctuary description | His record is only used for god behaviour; no attitude bonus found |
| B4 | Monster behaviour files (`Model/monster_behavior_*.txt`: "NEVER" plus 9 numbers) | The files exist | What the numbers mean |
| B5 | Hippodrome table: two of its five columns (0,1,1,1,2,3,0 and 0,0,0,1,1,3,15) | zeus.exe `0x006429F0` | What they do |
| B6 | How a house's plague "disease score" grows before an outbreak | Outbreak picks the highest scores | The growth rule (eZeus uses its own) |
| B7 | Rescue cash: accepting sometimes lowers that city's attitude by 10 | zeus.exe `0x00474A80` | When exactly (one branch's condition is unclear) |
| B8 | Workshops' finished-goods limits | Cap table and cart code (sculpture carts 400 units, others 100) | Which stock the cap applies to on the output side (eZeus keeps 8) |
| B10 | Building appeal "+10 when a flag is set" | zeus.exe appeal code | What the flag is |
| B11 | A sanctuary without sacrifices "loses its benefits" (Zeus manual) | The exe never shows its "This Sanctuary is inactive" text and nothing found turns a sanctuary off | Whether the manual describes a cut feature; not applied |

## C. Found in zeus.exe but not yet put into eZeus

None left: C1 (military service mood) and C2 (11% tax coverage) were applied on 9 Nov 2026. A1 (enemy warships) was applied on 9 Nov 2026 too; still open there: a warship count in the scenario editor, and the warships of the original's special invasion record 0x8c (see original-exe-findings.md). A2 (sanctuary priests and sacrifices) was applied on 10 Nov 2026; not traced there: the bull's standing animation and the Poseidon art of a priest leading cattle.

## D. Upstream eZeus gaps (GitHub issues, not original-game rules)

| Issue | Problem |
|---|---|
| #59 | Trade fails with some cities (not re-checked) |
| #73, #61 | Military overview misses soldiers in houses; overlays by category |
| #4 | Painting many sheep or goats blocks roads |
| #58, #51, #62, #37 | Undo, message inbox, audio settings, minimap |
| #43, #40, #28 | Missing text, shovel icon, adventure text |

## E. Differences kept on purpose (Nicolás's decisions)

- An Estate needs only its culture points (the original also asks for a competitor or curator visit).
- Artemis hunts every wolf in her city and wolf dens stop once her sanctuary is finished (the original only sends her after an attacking wolf).
- Sanctuary material totals use the Poseidon values; scaling by difficulty is mod feature 1.
- Mouse wheel zooms (the original scrolls the side-panel tabs).
- The "leave as is" list in `site-audit-plan.md` (wall archer range, tower damage on Titan/Olympian, cattle hit points and others), where eZeus follows the original game files rather than the Citadel site.
- Walker roaming distances stay at HeavenGames' measured 27/35/44 tiles (the exe gives about 26/34/43).
