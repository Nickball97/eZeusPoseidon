# Plan: match eZeus to the Citadel of Poseidon findings

Source: full read of https://poseidon.biancavanschaik.nl/ (about 100 pages), compared with the code on 9 Nov 2026. The site's housing and building tables are copies of the original `Model\Zeus_Model_*.txt` files, so where the two agree the number counts as verified. Where the site disagrees with the Model files, eZeus keeps the Model value (see "Leave as is").

How each phase runs:
1. Re-check every item against the current code before changing it (the audit can be wrong).
2. Make the change, build, install as eZeus-dev.
3. One commit per item or small group, pushed to the branch.
4. Tell Nicolás what to test. Anything without a source number is listed under "Needs a number" and is not coded until there is one.
5. New saved fields get a new `eFileFormat` version so old saves still load.

Ticks: 1 year = 127,750; 1 month ≈ 10,646. 100 units = 1 load.

---

## Phase 1: bugs and plain number fixes (small, low risk) — DONE 9 Nov 2026

Done and pushed, except: 1.13 (Demeter's blueprint does not map row for row onto the original, so it moves to Phase 4 with the sanctuary rework) and 1.22 (wolves hunting deer and boar ties into hunting, moved to Phase 3). Also done: Atlas (4.9) and 3.10 at 3/4.

| # | Change | Source | Where |
|---|---|---|---|
| 1.1 | Curses that target one building (Zeus destroys the palace, Demeter destroys farms) do nothing: the target is never stored. Store it like the general attack does | Bug (code) | `characters/actions/egodaction.h` `eLookForTargetedAttackGodAct::find` |
| 1.2 | Hercules' Hall also needs competitors (Zeus) / curators (Poseidon): add the 4th culture slot and require all walker types | Walkthroughs: Zeus and Europa ep. 2, Jason ep. 1, Atlantis Reborn ep. 3 | `buildings/eheroshall.cpp` (`provide`, status, requirement) |
| 1.3 | Odysseus' 8 elite houses must have residents | Trojan War ep. 4 | `eheroshall.cpp:634`, `eboardcity.cpp` `eliteHouses` |
| 1.4 | Achilles' sanctuary: Hephaestus or Athena per episode, not "Athena whenever available". Check whether the adventure file says which; if not, accept either | Trojan War ep. 5, Two Worlds ep. 2, Zeus and Europa ep. 7 | `eheroshall.cpp:50` |
| 1.5 | Olympian homestead tax multiplier 2 (Titan stays 1) | tax.html + Model file | `engine/edifficulty.cpp:960` |
| 1.6 | Demeter's farm blessing: 12 loads instead of 8 (×1.5, now ×4) | industries.html | `buildings/eemployingbuilding.cpp` `sBlessMultiplier` |
| 1.7 | Other blessings "4 loads once": timber mill, refinery, sculpture studio, orange lodge, corral, hunting lodge, carding shed; fishery and urchin quay 4 (now 12) | industries.html | `sBlessMultiplier`, each building's `useBlessing` call |
| 1.8 | Corral: 2 loads of meat per cow (now 3) | industries.html | `buildings/ecorral.cpp:118` |
| 1.9 | Growers' lodge holds 5 loads (now 8) | industries.html | `buildings/egrowerslodge.h:39` |
| 1.10 | Invasion sizes round down (now up) | enemy_appearance.html | `engine/eworldcity.cpp:408-458` |
| 1.11 | Centaurs: swordsmen counted as infantry, so bribes use the foot rate | enemy_appearance/bribes.html | `eworldcity.cpp:423`, `einvasionhandler.cpp:343` |
| 1.12 | Roadblock costs 2/3/4/5/10 by difficulty | buildingmodels.html, Nefermenu | `widgets/egamewidgetbuild.cpp:782` |
| 1.13 | Garden of Demeter 18×12 (blueprint has a 19th row) | sanctuary.html + `Model\Sanctuary_DEMETER.txt` | `sanctuaries/demeter.txt` |
| 1.14 | Mint sends 3 miners like the foundry (25 units each, 4 per load) | industries.html ("same as foundries") | `buildings/emint.cpp` |
| 1.15 | Elite food rounds down (Residence 1, Mansion 2 a month) | consumption.html | `buildings/eelitehousing.cpp:257` |
| 1.16 | Elite horse storage 1/2/4/4 and armor storage 2/2/4/4 by level | Model "ALL HOUSES" rows | `eelitehousing.cpp:169-176` |
| 1.17 | Soldier share 20% for hovel and homestead (25% above) | housing tables | `engine/eboardcity.cpp:1713` |
| 1.18 | Deer run at 1.25× | speed.html | `characters/efigurestats.cpp` (add deer) |
| 1.19 | Athena's prayer also gives olives (16) | Trojan War ep. 3, Perseus ep. 3, Jason ep. 3 | `characters/actions/godHelp/eathenahelpaction.cpp` |
| 1.20 | Horsemen count 2× hoplites in conquest (now 1.5) | Peloponnesian War ep. 3 | `army_strength_horseman_mult_d` (also in numbers.txt) |
| 1.21 | Wall sentries also shoot wolves | walkers/military.html | `characters/actions/earcheraction.cpp:88` |
| 1.22 | Wolves also hunt deer and boar | walkers/animals.html | `characters/actions/ewolfaction.cpp` |

Test: a city on Olympian with homesteads (tax), a Demeter blessing on a farm, a corral, an invasion's size, placing a roadblock, a curse from Zeus.

## Phase 2: houses (medium; changes how cities grow) — DONE 9 Nov 2026

Done and pushed: 2.1, 2.2 (with 6.6), 2.3, 2.4 for common houses, 2.5, 2.6, 2.7, plus no tax from abandoned elite homes. Left: plague in elite houses on Titan/Olympian (disease 5/10; the plague code only handles common houses), 2.8 (needs a number). An abandoned elite home still holds 6 people so it can receive goods (the original's vacant/devolved capacity is 0/1).

| # | Change | Source |
|---|---|---|
| 2.1 | Appeal to evolve/devolve per difficulty, from each `Zeus_Model_*.txt` (common and elite, including the "vacant" 40/46/50/50/55 to become a residence) | housing_*.html + Model files |
| 2.2 | Culture by points: philosopher 15, athlete 20, actor 25, competitor 20; science scholar 15, astronomer 25, inventor 20, curator 20; +10 citywide with a working stadium / museum. Needs per level from the Model file (hovel 15 … townhouse 45, residence 40/50 … estate 70/80 by difficulty). Replaces the "count walker types" rule | entertainment.html + Model `EVO_CULTURE` |
| 2.3 | Crime by house level and difficulty (Model crime increment; elite -20). Keep the outlaw types we added | housing_*.html + Model |
| 2.4 | Disease by house level and difficulty, elite houses included | housing_*.html + Model |
| 2.5 | Elite plots: nobody moves in (capacity 0, no tax) until storage holds 2 food, 1 fleece, 1 oil per house, as the original checks at placement | housing.html, Zeus and Europa ep. 3 |
| 2.6 | Fire risk per elite level (manor and estate lower) | buildingmodels.html + Model |
| 2.7 | People evicted when a house devolves walk off as homeless and look for a new home | walkers/housing.html |
| 2.8 | Low wages and long debt make people unhappy and leave (not only stop newcomers) | workers.html, walkers/housing.html — **needs a number** for how much |

Test: start the same map on Beginner and Olympian and watch house levels; a block with and without a stadium; crime and plague in huts vs townhouses; place elite plots with empty storehouses.

## Phase 3: farms and industry — DONE 9 Nov 2026

Done and pushed: 3.1 (vines and olives; oranges have no known season), 3.2, 3.3, 3.4 (plus wolves hunting deer and boar), 3.5 (plus Poseidon's 4-horse blessing), 3.6 (build time unchanged), 3.7, 3.8, 3.9. Fish per catch is still eZeus's 3 (missing-numbers #17).

| # | Change | Source |
|---|---|---|
| 3.1 | Vineyards and olive groves: harvest season (grapes Oct-Dec, olives Jan-Mar), lodge sends 3 harvesters, about 20 of each a year | industries.html |
| 3.2 | Dairy / carding shed: max 8 animals; each milked or shorn once a year, first time 6 months after placing | industries.html, walkers/other.html |
| 3.3 | Corral cattle: first ready 6 months after placing, calves 1 year; 6-7 a year | industries.html |
| 3.4 | Hunting lodge: 3 hunters; one new boar/deer a month after one is killed | industries.html |
| 3.5 | Horses born in even months; breeding scales with staffing | industries.html |
| 3.6 | Triremes: 2 wood + 2 armor each; wharf keeps 3 of each in stock. Build time **needs a number** (keep current until then) | industries.html |
| 3.7 | Distances: farms deliver up to ~300 tiles; horse ranch and chariot factory supply within 40 tiles | industrytable.html, industries.html |
| 3.8 | Food vendor: 8 loads first trip, then up to 6. Fleece/oil/wine: 4 loads, plus 2 more when stock falls below 200 units | walkers/agora.html |
| 3.9 | Fishery: one load per delivery trip | Perseus ep. 2 |
| 3.10 | Pillar of Atlas makes quarrymen faster — **needs a number** | industries.html |

Test: a full farm economy over 2-3 years; compare yearly output with industrytable.html.

## Phase 4: gods and sanctuaries — DONE 9 Nov 2026 (partly)

Done and pushed: 4.1 (rebase to the Zeus + Poseidon totals, per Nicolás; difficulty scaling is still mod feature 1), 1.13 Demeter layout, 4.2 (fisheries, urchin quays, piers; not ships), 4.3 (Apollo infirmaries, Hera corrals, Demeter farms, Artemis lodges; not infertility or people-to-cows), 4.4, 4.5 (carts only), 4.6, 4.9. Left: 4.7 (needs numbers), 4.8 dragon and Cerberus (after Phase 5 gives monsters hit points).

| # | Change | Source |
|---|---|---|
| 4.1 | Sanctuary material totals (already planned as mod feature 1: rebase to the original totals, then scale by difficulty) | sanctuary.html, HeavenGames |
| 4.2 | Poseidon's curse destroys fisheries, urchin quays, piers and ships | sanctuary.html |
| 4.3 | Apollo's curse also hits infirmaries; Hera's curse as the site describes; Demeter's infertility; Artemis curses hunting lodges | sanctuary.html |
| 4.4 | Hephaestus' sanctuary: no fires in the city | sanctuary.html |
| 4.5 | Hermes' sanctuary: cart pushers 1.25× speed (68 tiles a month); trade ships and caravans faster; partners visit more often (**needs a number**) | speed.html, sanctuary.html |
| 4.6 | Atlas: artisans' guild sends 2 artisans; quarrymen faster (see 3.10) | sanctuary.html |
| 4.7 | Bonuses that need numbers before coding: Athena stronger soldiers, Aphrodite city appeal, Hera fertility (mod-features "births"), Zeus opinion abroad, Apollo games odds and hygiene blessing, Artemis predator protection | sanctuary.html |
| 4.8 | Guardians: Ares' dragon kills invaders, wolves and boar; Hades' Cerberus defends | sanctuary.html |
| 4.9 | Atlas blesses refineries and timber mills, and his finished sanctuary makes white and black marble quarrymen work in 3/4 of the time, like Poseidon with fisheries (decided by Nicolás 9 Nov; 3/4 until a source number turns up) | sanctuary.html, Nicolás |

## Phase 5: military and monsters — DONE 9 Nov 2026 (partly)

Done and pushed: 5.1 (gods leave at 0 HP, monsters are slain, heroes go home; hero-vs-monster duels stay scripted), 5.3, 5.4, 4.8 (Dragon and Cerberus guard the city), 5.2 enemy warships and 5.6 priests (both 10 Nov 2026, from zeus.exe). Left: 5.5 (needs a number).

| # | Change | Source |
|---|---|---|
| 5.1 | Hit points and attack for gods (200/100), monsters (500/50) and heroes (300/50) so soldiers can wear them down (decided by Nicolás 9 Nov: yes) | figures.html |
| 5.2 | Enemy warships (trireme/frigate rows, all nations) and the 800-drachma bribe per warship | enemy_figures.html, enemy_bribes.html |
| 5.3 | Transport hit points by nation (Mayan 200, Phoenician 150) | enemy_figures.html |
| 5.4 | Arrival order: hoplites, then archers, then cavalry | enemy_appearance.html |
| 5.5 | Watchmen first try to talk a criminal round, then fight — **needs a number** for the chance | walkers/other.html |
| 5.6 | Priests walk sheep, goats or cattle to the altar for sacrifices | figures.html, walkers/other.html |

## Phase 6: buildings, walkers and fire — DONE 9 Nov 2026

All done and pushed: 6.1 (agora fire risk 6/12/12/12/20 on the agora, stalls 0; a burning agora takes its stalls with it), 6.2 (docks already had the pier risk), 6.3 (replaced by the exe's fire and collapse counters), 6.4 (exit = first road clockwise from the north corner), 6.5 (fountain and infirmary walkers return to the road tile north of the corner), 6.6 (done in Phase 2), 6.7 (exe: hippodrome mood), 6.8 (54.4 tiles a month).

| # | Change | Source |
|---|---|---|
| 6.1 | Agora burns as one building (risk 6/12/12/12/20); stalls don't | buildingmodels.html + Model |
| 6.2 | Docks use the pier fire risk (18 on Olympian) | buildingmodels.html |
| 6.3 | Collapse after a fixed time: 60 ÷ damage risk months without maintenance | buildingmodels.html legend |
| 6.4 | Walkers always leave from the first road found north of the building, going clockwise | walkerdist.html |
| 6.5 | Fountain and infirmary walkers return via the tile north of the building | walkerdist.html |
| 6.6 | Stadium and museum citywide +10 (part of 2.2) | entertainment.html |
| 6.7 | Hippodrome over 200 lengths lowers popularity — **needs a number** | hippodrome.html |
| 6.8 | Base walker speed 54.4 tiles a month (now 53.2) | speed.html |

## Phase 7: missing features (large)

| # | Feature | Source |
|---|---|---|
| 7.1 | Boulevard (3 wide, cost 15/24/30/36/45, appeal 3, -2 every 2 tiles, range 4, every tile counts) | buildingmodels.html, appealexceptions.html |
| 7.2 | At most 10 working piers | Life in the Mediterranean ep. 6 |
| 7.3 | Allies and vassals sending money when you are in debt — **needs a number** | Atlantis Reborn ep. 1, Athens ep. 1 |
| 7.4 | Adventure events eZeus can't read yet (event ids 6, 7, 10, 11, 12, 17, 18 and several city-status types): find what each is, then add them | `fileIO/ecampaignreadpak.cpp:295-322, 784-848` |
| 7.5 | Hotkeys: number keys for side-panel tabs, Tab for the overview map, Space for overlays, Home/PgUp/PgDn to rotate, company bookmarks; game speed in 10% steps | hotkeys.html |
| 7.6 | Cheats (Ctrl+Alt+C box and the cheat words) (decided by Nicolás 9 Nov: add them) | cheats.html |

## Leave as is (site disagrees with the original game files; eZeus follows the files)

Wall archer range, tower damage on Titan/Olympian, cattle hit points, carrot vs onion farm appeal, Ares and Poseidon sanctuary sizes, gatehouse size, the Olympian hut/shack appeal rows, boar strength (agreed with Nicolás earlier), mouse wheel zooms (UI choice).

## Needs a number (not coded until there is a source)

Wage/debt unhappiness (2.8), trireme build time (3.6), Pillar of Atlas quarry speed (3.10), Hermes trade frequency (4.5), the bonuses in 4.7, watchman persuasion chance (5.5), hippodrome popularity penalty (6.7), ally debt gifts (7.3), the exact "good/excellent" thresholds for hero halls (eZeus uses popularity ≥ 80, health ≥ 70, coverage ≥ 72%, tax coverage ≥ 88%, appeal ≥ 17, palace within 10 tiles).
