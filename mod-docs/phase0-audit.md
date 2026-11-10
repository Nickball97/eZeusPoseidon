# Phase 0: eZeus completeness audit

Goal: make eZeus match the original Zeus and Poseidon before adding mod features. Audit done 5 Nov 2026 against upstream commit ec76519 (the fork was identical at that point).

Status meanings: **Missing** = no code found. **Partial** = some of it exists. **Reported** = an open GitHub issue describes it; not yet checked in the code.

Verify each item in the code (and, where possible, in the original game) before changing anything; the audit can be wrong.

## Features from the manuals

| Feature | Original behavior (source) | Status | Where to look |
|---|---|---|---|
| Hero hall requirements | Wine: Hercules 32, Jason 16, Odysseus 16, Theseus 16, Bellerophon 24. Jason: 3 triremes (Atlantean: 3 frigates). Bellerophon: 15 horses, 10,000 drachmas (HeavenGames: Heroes) | Partial | Commented out in `buildings/eheroshall.h` (`eHeroRequirementType`) and the matching `addRequirement` calls in `eheroshall.cpp` |
| Orichalc in pyramid costs | Every Atlantean pyramid, Monument to the Sky, shrine, Pyramid of the Pantheon, Altar and Temple of Olympus, Observatory Kosmika and Museum Atlantika needs orichalc (Poseidon manual, Pyramids) | Partial | Only `buildings/pyramids/epyramidwall.cpp` costs orichalc (`eSanctCost{0,2,0,1}`) |
| Consult Oracle | Right-click Zeus' Stronghold or the Oracle of Apollo, "Consult Oracle", get a prediction; works like prayer (Zeus manual, Consulting the Oracle) | Missing | Only prediction message texts are loaded |
| Soldier morale | Losses lower a company's morale; low morale makes it flee to the Palace; recovers with time; loaned troops never lose morale (Zeus manual, Morale) | Missing | No morale code in `characters/` or `engine/` |
| Symphonia Ithikos | Attacking an Atlantean ally makes every Atlantean leader your rival (Poseidon manual, The Atlanteans) | Missing | No matching code |
| Frigates and Frigate Wharf | Atlanteans build frigates instead of triremes; the wharf fits an Atlantean Fire launcher using orichalc (Poseidon manual, Military) | Missing | No "frigate" in the code; see the trireme for a model |
| Atlantean Fire on towers | Towers supplied with orichalc launch Atlantean Fire and are stronger (Poseidon manual, Military) | Missing | `buildings/etower.cpp` fires arrows only |
| Citadel of Poseidon | Atlantean version of the Promontory; same benefits; needs white marble, sculpture, wood, orichalc (Poseidon manual, Mythology) | Missing | `buildings/sanctuaries/esanctuaryblueprint.cpp` loads only `poseidon.txt` (21×14 Promontory). Needs a blueprint and art |
| Oceanid invaders | Oceanids swim underwater; frigates can't stop them; the Kraken can attack them submerged (Poseidon manual, The Atlanteans) | Missing | Oceanid soldier types exist but behave like land soldiers |
| Bridges | "Atlanteans build Bridges, rather than Water Crossings… Ships can pass under them" (Poseidon manual) | Partial, unconfirmed | Ships ignore all bridges (`characters/actions/walkable/ewalkableobject.cpp`). Check in the original game whether Greek crossings block ships |

Confirmed present: Pan-Hellenic games, prayer and god help, sacrifices, hippodrome revenue, crosswalks, colonies, raids, conquest, tribute, 14 gods, monsters, 8 heroes with Atlantean variants, disasters, orichalc mining, loaned Amazons and Ares' warriors, enchanted places, original adventure files.

## Open GitHub issues that affect gameplay (upstream)

| Issue | Problem |
|---|---|
| #76 | Saving fails (blocking) |
| #59, #30 | Trade fails with some cities; partners show 0/0 or negative stock |
| #55 | Towers can be built without 4 walls under them |
| #54, #52, #20 | Towers and the army don't attack wolves |
| #75 | Elite houses don't spawn noble walkers |
| #63 | Bribing an attacking city doesn't give the monument reward |
| #13 | Gods' blessings hit buildings outside the god's domain |
| #69 | Bellerophon tax requirement (code now asks for level 4; issue still open) |
| #44 | Hydra spawns in different places than the original |
| #73, #61 | Military overview missing soldiers in houses; overlays by category missing |
| #12 | Settler/walker dialogue missing |
| #4 | Painting many sheep or goats blocks roads |
| #58, #51, #62, #37, #31 | Undo, message inbox, audio settings, minimap, road cost preview |
| #43, #40, #28 | Missing text, shovel icon, adventure text |

Before starting an item, check the issue on GitHub for new comments from the maintainer: something may already be fixed or in progress upstream.

## Suggested order

1. Blocking bugs: saving (#76), trade (#59).
2. Cheap rule fixes: hero requirements, tower on walls (#55), orichalc pyramid costs, bribe monument (#63).
3. Systems that reuse existing parts: Consult Oracle (like prayer), soldier morale, Symphonia Ithikos.
4. Bigger Poseidon content: frigates and wharf, Atlantean Fire, Citadel of Poseidon, Oceanid underwater movement, bridge rules.

## Sources

- Zeus manual: https://cdn.akamai.steamstatic.com/steam/apps/566050/manuals/Zeus_-_Manual.pdf
- Poseidon manual (Steam, Zeus_-_Poseidon_-_Manual.pdf)
- HeavenGames heroes: https://zeus.heavengames.com/gameinfo/heroes/
- eZeus issues: https://github.com/MaurycyLiebner/eZeus/issues
