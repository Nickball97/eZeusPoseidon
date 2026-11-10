# What eZeus still lacks compared with the original

Status on 9 Nov 2026, after Phases 0-7 and the zeus.exe reading. Details and addresses for anything taken from the exe are in `original-exe-findings.md`.

## A. Features the manuals or sources describe that eZeus doesn't have

| # | Missing | Source | What it would take |
|---|---|---|---|
| A1 | Enemy warships (trireme/frigate rows for every nation) and the 800-drachma bribe per warship | Citadel: enemy_figures, enemy_bribes (plan 5.2) | New enemy ship figures, AI and art (the original has "Enemy Warship", figure 64) |
| A2 | Priests walking sheep, goats or cattle to the altar for sacrifices | Citadel: figures, walkers/other (plan 5.6) | New walker, art and sanctuary logic (the original has "Priest", figure 65) |
| A3 | Boulevard (3 wide; cost 15/24/30/36/45; appeal 3, -2 every 2 tiles, range 4) | Model BUILD_BOULEVARD, game text, Citadel (plan 7.1) | Graphics: not found in eZeus's images; unclear whether the original has art for it |
| A4 | Ground that looks better with high appeal (four looks: below 0, 0-15, 16-29, 30+) | zeus.exe `0x004A0450` | Mapping the original's ground images by appeal |
| A5 | Hotkeys not added: Shift+1-4 company bookmarks and 1-4 to select them; Tab for the overview map; M "freeze footprint"; F5-F7 window/resolution; pressing 1 twice for the full population report | Citadel: hotkeys | Overview map doesn't exist in eZeus; "freeze footprint" meaning unknown |
| A6 | Cosmetic cheats: Bowvine and Arrow (towers shoot cows), Cheese Puff (dairy workers in cheese suits), Mammaldrome (animals race in the hippodrome) | Citadel: cheats | Each needs art swaps |
| A7 | Apollo's sanctuary: +20 to the Pan-Hellenic games score | zeus.exe `0x00565D40` | eZeus's games use their own winning chance; the original's games scoring wasn't traced |
| A8 | Rescue cash shown with its own "A helping hand from..." message (eZeus uses the gift message) | zeus.exe, eventmsg | New message type |

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
| B9 | Flat road paving (which road image by appeal) | Slope roads pave above appeal 4 | Flat roads weren't traced (eZeus uses the slope rule for all) |
| B10 | Building appeal "+10 when a flag is set" | zeus.exe appeal code | What the flag is |

## C. Found in zeus.exe but not yet put into eZeus

None left: C1 (military service mood) and C2 (11% tax coverage) were applied on 9 Nov 2026.

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
