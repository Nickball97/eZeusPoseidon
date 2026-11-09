# Mod features (after Phase 0)

Working summary of the design decisions. Full document with history, tables and sources: https://claude.ai/artifact/Y1tCbnChuKEx14NbSQKuML

Design principles: one mechanic per gameplay area; historically grounded; keep the original Zeus feel; reuse existing eZeus systems and art (new art is the most expensive part).

Difficulty levels in code (`engine/edifficulty.h`): beginner, mortal, hero, titan, olympian.

## Roadmap (build in this order)

| # | Feature | Complexity | New art |
|---|---|---|---|
| 1 | Sanctuary costs scaled by difficulty | Low | None |
| 2 | Per-person fleece, oil and wine; difficulty production scaling | Low–Medium | None |
| 3 | Farm harvest seasons | Low–Medium | None |
| 4 | Walker gates (configurable roadblock) | Low–Medium | None |
| 5 | Food quality and diversity | Medium | None |
| 6 | Cisterns feeding fountains | Medium | Cistern, water cart |
| 7A | Invasion planning screen | Medium | UI only |
| 7B | Real-time battle | High | Battlefield tileset |

Note: Manor wine needs no fix. eZeus elite levels start at 0 (unevolved), so Manor is level 3 and `if(mLevel > 2)` in `buildings/eelitehousing.cpp` is correct.

## 1. Sanctuary costs

- Rebase done 9 Nov 2026: each sanctuary is rescaled at placement to the Zeus + Poseidon totals (marble · wood): Zeus 137·24, Hades 132·24, Demeter 110·20, Poseidon 103·20, Hera 88·20, Apollo 76·16, Atlas 76·20, Athena 55·16, Artemis 63·20, Hephaestus 44·12, Ares 40·12, Aphrodite 37·12, Hermes 35·12, Dionysus 31·12 (Citadel of Poseidon, sanctuary.html; Nicolás chose the Poseidon numbers over the Zeus-only ones 145·28 etc.). See `eSanctuary::applyOriginalMaterialTotals`; the difficulty multipliers below go on top of that.
- Multipliers by difficulty: marble (and black marble) ×1 / 1.25 / 1.5 / 1.75 / 2; wood ×1 / 1.5 / 2 / 2.5 / 3. Sculpture, and the marble paid to place the sanctuary, unchanged. Orichalc multiplier: open decision.
- Add `sanctMarbleMultiplier` / `sanctWoodMultiplier` in `engine/edifficulty.cpp`.
- Scale at use time in `buildings/sanctuaries/esanctbuilding.cpp` (stage checks, consumption, and the total-cost function the info panel uses). Pyramids save their base costs to the save file, so scaling at construction would double-scale on load.
- Round once per sanctuary, not per stage: each stage charges round(multiplier × base used so far including this stage) minus what was already charged. Per-tile rounding doubles the 1-marble floor tiles.
- Apply the same to Poseidon's pyramids. Depends on Phase 0 (Citadel, orichalc costs).

## 2. Goods economy

- Fleece and oil: 0.1 units per person per month in common housing, 0.3 in elite housing. Wine: 0.3 per person per month in Manors and Estates. Same housing levels as now (fleece from Homestead, oil from Apartment). Round monthly amounts to whole units, as food already is. Vanilla is a flat 2 units per house per month.
- Change `nextMonth()` in `buildings/esmallhouse.cpp` and `buildings/eelitehousing.cpp`.
- Make the storage cap in `agoraProvide()` proportional to use (about 4 months). The fixed cap of 8 would cause devolving.
- Production multiplier for food, fleece, oil and wine: 1.00 / 0.84 / 0.71 / 0.59 / 0.50. Apply where production is added, with a per-building counter for fractional remainders (fisheries don't use the shared timer). Marble, wood and bronze unaffected.

## 3. Farm harvest seasons

- One harvest a year tied to the calendar: wheat in July, carrots and onions in April. 8 loads in vanilla × the production multiplier. Understaffing shrinks the harvest instead of delaying it. Currently a fixed cycle all year in `buildings/efarmbase.cpp`.

## 4. Walker gates

- Emperor-style gates on roads with an on/off toggle per roaming walker category: agora peddlers; water carriers; culture (philosophers, actors, athletes, competitors); safety (watchmen, firefighters, healers); tax collectors. Destination walkers (carts, deliveries) always pass.
- Replace the single roadblock flag in `buildings/eroad.cpp` with category toggles; pass the walker's category into the roadblock check (`characters/actions/epatrolaction.cpp`, `characters/actions/walkable/ewalkableobject.cpp`); add a settings panel. Prototype as a configurable roadblock reusing roadblock art.

## 5. Food quality and diversity

- Quality depends only on the number of distinct food types a house gets (Emperor style, no condiments, no specific food required). Tiers: 1 Meager, 2 Simple, 3 Varied, 4 Hearty, 5+ Feast (names not final; alternative Poor / Simple / Balanced / Nutritious / Feast).
- Starting thresholds to playtest: common level 1 Meager, 3 Simple, 5 Varied; elite 1–2 Varied, 3–4 Hearty. Feast is optional and gives a hygiene/health bonus.
- Agora food vendor (`buildings/evendor.cpp`) stores a count per food type instead of one merged counter; carts prefer missing types; peddlers deliver quality with food; houses store it and it decays; evolution checks in `eSmallHouse::updateLevel()` and the elite equivalent; UI shows quality. New saved fields.

## 6. Cisterns and fountains

- Cistern built on meadow (fertile) or marsh terrain produces a new `water` resource (free bits in `engine/eresourcetype.h`); carts carry it by road to fountains; a fountain's water carrier only serves houses while it has stock. Water isn't stored in warehouses or traded. Building next to water: open decision.
- Rework `buildings/efountain.cpp` to work like an agora vendor. Art: cistern and water cart (a recolored cart at first).

## 7. Military invasion screen

- Tier A (first): before attacking another city, a planning screen shows enemy composition; the player picks companies and a tactic; unit matchups adjust the outcome; shows a predicted result with an auto-resolve button. Current resolution is one formula in `gameEvents/eplayerconquestevent.cpp`.
- Tier B (later): a generated battlefield where existing soldier AI fights. Design Tier A so Tier B can replace its auto-resolve step.

## Open decisions

- Final food tier names; food thresholds after playtesting.
- Cisterns next to water, or only on fertile and marsh tiles.
- Gate presentation: configurable roadblock or a full residential wall with gates.
- Orichalc cost multiplier.

## Saved ideas (not in scope)

Wine vinegar building; plague triggers (crowding, trade ships, sieges); repression (too many watch posts lowers sentiment); silphium as an imported luxury; births and deaths (a small natural trickle on top of immigration, e.g. about 1 new resident a month in a house with room, with Hera's sanctuary raising the birth rate; the manual gives no number, eZeus has no births today; amounts need playtesting).
