# What eZeus still lacks compared with the original

Status on 11 Nov 2026, after Phases 0-7 and the zeus.exe reading (done since 9 Nov: Avenue/Boulevard, Pan-Hellenic games, rescue cash attitude, waterfront appeal, plague notes, all 57 scenario allowed-structures switches, workshop output, company bookmarks, monster behaviour files, hippodrome standing, the original angry-god curses incl. Demeter and Hera, angry Ares and Artemis armies, the M freeze-footprint key, F5, F8 and Tab). Details and addresses for anything taken from the exe are in `original-exe-findings.md`.

## A. Features the manuals or sources describe that eZeus doesn't have

| # | Missing | Source | What it would take |
|---|---|---|---|
| A5 | Hotkeys not added: F6 and F7 screen sizes (eZeus changes size only in the settings menu). Tab shows the overview tab, but the build buttons stay (the original hides them). (Company bookmarks, Ctrl/Alt+1-5 in the exe, a tab key pressed twice, M "freeze footprint", F5 full screen, F8 centre window and Tab were added on 11 Nov 2026.) | Citadel: hotkeys; zeus.exe `0x005C4A90` | Full reports exist for only 3 tabs |
| A6 | Cosmetic cheats: Bowvine and Arrow (towers shoot cows), Cheese Puff (dairy workers in cheese suits), Mammaldrome (animals race in the hippodrome) | Citadel: cheats | Each needs art swaps |

## B. Described by the sources, but the numbers or the rule weren't found

| # | Missing | What is known | What wasn't found |
|---|---|---|---|
| B3 | Zeus' sanctuary improves your standing abroad | Sanctuary description | His record is only used for god behaviour; no attitude bonus found |
| B11 | A sanctuary without sacrifices "loses its benefits" (Zeus manual) | The exe never shows its "This Sanctuary is inactive" text and nothing found turns a sanctuary off | Whether the manual describes a cut feature; not applied |

## C. Found in zeus.exe but not yet put into eZeus

None left: C1 (military service mood) and C2 (11% tax coverage) were applied on 9 Nov 2026. A1 (enemy warships) was applied on 9 Nov 2026 too; still open there: a warship count in the scenario editor (low priority). The special invasion record 0x8c turned out to be the spare slot used by angry Ares and Artemis armies, which come by land, so no warships apply (11 Nov 2026). A2 (sanctuary priests and sacrifices) was applied on 10 Nov 2026; not traced there: the bull's standing animation and the Poseidon art of a priest leading cattle.

## D. Upstream eZeus gaps (GitHub issues, not original-game rules)

Fixed here: #28 (11 Nov 2026, adventures in their own folder lost their text on Windows because file names were cut only at "/").

| Issue | Problem |
|---|---|
| #59 | Trade fails with some cities (not re-checked) |
| #73, #61 | Military overview misses soldiers in houses; overlays by category |
| #4 | Building over sheep or goats was fixed on 8 Nov 2026 (they move to free meadow). Still open, not verified against the original: erasing removes them, and the original may let many animals share one meadow |
| #58, #51, #62, #37 | Undo, message inbox, audio settings, minimap |
| #43, #40 | Missing text (owner suspects SDL2 textures after a window resize), shovel cursor (the red tint on things being cleared already exists in eZeus; only the cursor is missing) |

## E. Differences kept on purpose (Nicolás's decisions)

- An Estate needs only its culture points (the original also asks for a competitor or curator visit).
- Artemis hunts every wolf in her city and wolf dens stop once her sanctuary is finished (the original only sends her after an attacking wolf).
- Sanctuary material totals use the Poseidon values; scaling by difficulty is mod feature 1.
- Mouse wheel zooms (the original scrolls the side-panel tabs).
- The "leave as is" list in `site-audit-plan.md` (wall archer range, tower damage on Titan/Olympian, cattle hit points and others), where eZeus follows the original game files rather than the Citadel site.
- Walker roaming distances stay at HeavenGames' measured 27/35/44 tiles (the exe gives about 26/34/43).
