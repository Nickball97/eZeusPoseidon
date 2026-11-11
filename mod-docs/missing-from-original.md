# What eZeus still lacks compared with the original

Status on 11 Nov 2026, after Phases 0-7 and the zeus.exe reading (done since 9 Nov: Avenue/Boulevard, Pan-Hellenic games, rescue cash attitude, waterfront appeal, plague notes, all 57 scenario allowed-structures switches, workshop output, company bookmarks, monster behaviour files, hippodrome standing). Details and addresses for anything taken from the exe are in `original-exe-findings.md`.

## A. Features the manuals or sources describe that eZeus doesn't have

| # | Missing | Source | What it would take |
|---|---|---|---|
| A5 | Hotkeys not added: Tab for the overview map; M "freeze footprint"; F5-F7 window/resolution. (Company bookmarks, Ctrl/Alt+1-5 in the exe, and a tab key pressed twice were added on 11 Nov 2026.) | Citadel: hotkeys; zeus.exe `0x005C4A90` | Overview map doesn't exist in eZeus; what M looks like in play wasn't worked out; full reports exist for only 3 tabs |
| A6 | Cosmetic cheats: Bowvine and Arrow (towers shoot cows), Cheese Puff (dairy workers in cheese suits), Mammaldrome (animals race in the hippodrome) | Citadel: cheats | Each needs art swaps |

## B. Described by the sources, but the numbers or the rule weren't found

| # | Missing | What is known | What wasn't found |
|---|---|---|---|
| B1 | Demeter's invasion "renders your land infertile" | Oracle and invasion messages | No code found; eZeus kills half the sheep and goats only |
| B2 | Hera's invasion "turns your citizens into cows" | Invasion message | No code found; eZeus kills half the cattle and orange trees and lowers attitudes by 20 |
| B3 | Zeus' sanctuary improves your standing abroad | Sanctuary description | His record is only used for god behaviour; no attitude bonus found |
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
