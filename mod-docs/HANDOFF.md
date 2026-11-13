# Handoff: stability and cleanup (from 13 Nov 2026)

Written at the end of the "match the original" phase for the next session.
Read this first, then `CLAUDE.md` (untracked, Nicolás's own file) and the
other files in `mod-docs/`.

## In one paragraph (for Nicolás)

eZeusPoseidon now plays like the original in nearly every respect that the
manuals, HeavenGames and zeus.exe could confirm. The next job is not new
features but making the base solid. First, build a safety net: an automatic
check that runs cities with nobody playing and flags any change in how the
game behaves. Then fix bugs and tidy the code in small steps, each checked
against that net. The goal is that future mod features (see
`mod-features.md`) can be added without breaking things. You keep testing
in the game. The next session does the code work and tells you plainly what
to try after each step.

## Rules that carry over

- **Game values (numbers, rules, behaviour):** don't change any value
  without asking Nicolás first, unless it's an obvious bug. Some current
  values are his own creative choices and deliberately differ from the
  original, so "the original says otherwise" isn't a reason to change one.
  - When a value looks wrong, tell him what it is now, what you'd change it
    to and why. Name a source if there is one (HeavenGames
    https://zeus.heavengames.com/, the manuals, zeus.exe, the game's
    `Model\Zeus_Model_*.txt`). Then wait for his OK.
  - Never invent a number. When nothing confirms a value, say so.
  - Cleanup and refactors must not change any value at all. That is what
    the safety net checks.
- **Design docs:** check each claim in a design doc against the actual code
  before changing anything.
- **Original game files:** never commit them (art, text, `.eng`, maps, the
  exe, manuals).
- **Commits:** commit with clear messages and push to the work branch. No
  pull requests unless Nicolás asks. End every commit message with
  `Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>` (or the
  attribution the session gives).
- **Old saves:** not a priority (Nicolás, 13 Nov 2026). The game is still
  being built, and he'll deal with save compatibility once he's happy with
  the end product. A change may break saves from older builds; just say so
  when it does, so he knows to start a new city.
  - The version mechanism is still there if it's cheap to keep using:
    append a value to `eFileFormat` in `fileIO/efileformat.h` (latest:
    `messageHistory`, `messagePanel`, `godCurseVisit`).
  - A save must still reload correctly in the build that made it. The
    safety net's save-and-reload check covers this.
  - `CLAUDE.md` still says new saved fields must keep old saves loading.
    That line is out of date; Nicolás will edit it.
- **Working with Nicolás:** he isn't a programmer. Use plain language, say
  what to test in the game, and ask before anything hard to undo. He tests
  in the game himself. Don't drive the game with computer-use unless he asks.
- **Upstream:** no more merges from upstream eZeus (decided 12 Nov 2026), so
  deep refactors are allowed. The upstream paragraph in `CLAUDE.md` is out
  of date. Nicolás will edit it; don't commit `CLAUDE.md`.
- **The game while it runs:** only install a build when `eZeus-dev.exe`
  isn't running. Check with `tasklist`.

## Where things are

| What | Where |
|---|---|
| Repository | `C:\Users\LENOVO\source\repos\eZeusPoseidon` (GitHub `Nickball97/eZeusPoseidon`) |
| Work branch | `ccr-3448c71b-u8j6yh`. Holds about 195 commits that are not on `main`. See "Decisions for Nicolás". |
| Build | `cmd //c 'C:/Users/LENOVO/source/repos/eZeusPoseidon/tools/build.cmd'` (in the repo since 13 Nov 2026). Then search `C:\Users\LENOVO\source\repos\eZeus-build\build.log` for `error C\|error LNK`. Uses Ninja and MSVC (VS 18), Release, with `EZEUS_DEV_CHEATS=ON`. |
| Install | `powershell -ExecutionPolicy Bypass -File C:/Users/LENOVO/source/repos/eZeusPoseidon/tools/deploy.ps1`. Copies the exe as `Bin\eZeus-dev.exe` with its `.pdb`, plus `text/language.txt` and `sanctuaries/*.txt`. It also strips keys our code now owns from the installed `numbers.txt` (backup at `numbers.txt.orig`) and cuts sprite sheets with `tools/sgextract`. |
| Game data | `D:\Steam\steamapps\common\Zeus + Poseidon\eZeus-0.8.2-beta` (the original game is one folder up) |
| Game text | `Zeus_Text.xml` in the data folder (readable copy of `Zeus_Text.eng`); `eLanguage::zeusText(group, id)` reads it |
| Dev shortcuts and crash stack traces | `mod-docs/dev-shortcuts.md`, `edevcrash.cpp` (dev build only; stack printed to stdout) |
| zeus.exe reverse engineering | Ghidra output in `C:\Users\LENOVO\source\repos\re-tools\work\out\all.c`; findings in `mod-docs/original-exe-findings.md` |
| Feature gaps vs the original | `mod-docs/missing-from-original.md` |
| Bugs from playtests | `mod-docs/playtest-bugs.md` |
| Mod roadmap (after this work) | `mod-docs/mod-features.md` |

The build and install scripts were in `eZeus-tools`, outside the repo, so
they weren't versioned. They are now `tools/build.cmd` and `tools/deploy.ps1`
(the old copies in `eZeus-tools` still work for older branches).

## Priority 1: the safety net (do this before touching any code)

**Done on 13 Nov 2026** (branch `cleanup/safety-net`). How to run it, what it
covers and what building it turned up are in `mod-docs/safety-net.md`. The
notes below are the original brief.

eZeus has no automated tests. Nothing in a refactor can be called safe until
this exists.

**What to build:** a "headless run" mode in the dev build.
- Turn it on with a command-line flag, for example
  `eZeus-dev.exe --headless-check <map-or-save> <years>`.
- It loads a map or save, runs the simulation for N game years with no
  window drawing and nobody playing, then writes a plain-text summary:
  - the date;
  - population;
  - drachmas;
  - building counts by type;
  - stored goods;
  - popularity, hygiene and unrest;
  - the number of walkers by type;
  - how many messages were received.
- A small script runs it over a fixed set of maps and compares the result
  with a committed baseline.

**What to know first:**
- **Randomness.** Every random number goes through `eRand` (`erand.h` /
  `erand.cpp`; 114 call sites, no plain `rand()`). It is seeded from
  `std::random_device`, so add a way to fix the seed. That should be enough
  to make runs repeatable if nothing else gets in the way.
- **The thread pool.** Pathfinding runs on a thread pool (`eThreadPool`,
  `eThreadBoard` copies of the board, `engine/ethreadpool.*`). Results that
  arrive on different ticks from run to run will make two runs differ.
  - The headless mode may need to wait for each task before the next tick,
    or run the tasks on the main thread.
  - Run every map twice and confirm the summaries match exactly before
    trusting the net.
- **Time and the main loop.**
  - One game year is 365 × 350 ticks.
  - The frame loop is `eMainWindow::exec`.
  - The simulation step is `mBoard->incTime` in
    `widgets/egamewidgetpaint.cpp`.
  - Separate "advance the simulation" from "paint the frame" so the
    headless mode can call only the first. This is also a cleanup win in
    itself.
- **Test maps.** The two campaigns shipped in the repo's `Adventures/`
  (The Founding of Athens, The Sands of Betrayal) plus the original
  campaigns in the data folder. Read the maps from the data folder; don't
  copy them into the repo.
- **Save and reload.** A save made after the run should reload in the same
  build and give the same summary. That catches save-format slips.
- **Start from maps, not saves.** Old saves may stop loading as the game
  changes, so the baseline should start from the campaign maps and not
  from saved cities.
- **What it won't catch:**
  - drawing and interface bugs;
  - anything that needs a player to click (building, Undo, dialogs);
  - god and monster fights that the sample maps never trigger.
  Say so plainly to Nicolás.

**Done means:** the mode builds, two runs match exactly, the baseline is
committed and pushed, and Nicolás has been told in plain words what it
covers.

## Priority 2: bugs and stability

### Known open problems

| Problem | Status |
|---|---|
| #59: trade "buys nothing for years" with some cities | The one cause found was fixed on 6 Nov. Needs a save that shows it. |
| #43: missing text after a window resize | Owner suspects SDL2 textures being lost. Not seen by Nicolás. |
| #60 | Reported on Mac only. Low priority on Windows. |
| #4: sheep and goats | The clear-land tool removes them. Nicolás is fine with that. |
| Unrest cut-offs | The Overview's word and the Hygiene tab's sentence use different cut-offs. No source says which is right. Ask Nicolás whether to line them up. |
| Test city "GG Aperta" | Still installed in `Save/Nickball/` and `Adventures/Aperta` in the data folder. Ask Nicolás before deleting it. |

### Added 12–13 Nov and still waiting on Nicolás's playtest

If he reports problems, look here first:
- Undo (`eGameBoard::beginUndoAction` … `undo`). It is the riskiest: it
  restores terrain and buildings. Clearing land blocks undo; each action
  stays undoable for 500 original ticks.
- The message history and Messages window.
  - Files: `engine/emessagehistory.*`, `widgets/emessageswidget.*`.
  - The count on the Messages button.
  - Envelope icons.
  - Options → Messages, categories from the game's
    `Model/Zeus event message categories.txt`.
  - "Off" messages kept out of the side-panel box.
- Angry gods' curse visit: they walk to the building, curse it, and leave
  after 3 failed searches (`characters/actions/egodattackaction.*`).
- Palace tiles no longer walkable. The player's soldiers use their own
  walkable type, `eWalkableObject::sCreateSoldier`.
- The in-city Options (sound and speed) and the editor's Warships field.
- Side-panel colours: yellow / dark / white
  (`eDataWidget::sSetStatusColor`). The Overview now says "workers needed"
  when jobs go unfilled.

### Hunting for crashes

- The dev build prints a stack trace on a crash, using the `.pdb` that
  `deploy.ps1` copies next to the exe. Ask Nicolás to run the game from a
  terminal, or to send the console output, when it crashes.
- Likely sources:
  - pointers to buildings or characters that were deleted (see how
    `stdptr` is used);
  - the pathfinding threads reading a board that changed under them;
  - events that fire on a city that no longer exists.
- Past crash fixes in `git log --grep=crash` show the pattern.

## Priority 3: code cleanup and optimisation

Do this only with the safety net in place. Use small steps, one subject per
commit, each built and checked against the net.

### The biggest files

These are where most bugs hide:

| File | Lines |
|---|---|
| `engine/egameboard.cpp` | 4,550 |
| `widgets/egamewidgetpaint.cpp` | 4,029 |
| `textures/ebuildingtextures.cpp` | 3,365 |
| `engine/eboardcity.cpp` | 3,106 |
| `widgets/egamewidget.cpp` | 2,841 |
| `buildings/ebuilding.cpp` | 2,825 |
| `widgets/egamewidgetbuild.cpp` | 2,297 |
| `audio/esounds.cpp` | 2,248 |

(`offsets/*.h` are generated sprite tables; leave them.)

### Suggested order

**Status on 13 Nov 2026.** Steps 1 to 3 are done, each checked against the
safety net (same result on all seven adventures):
- Step 1 (branch `cleanup/housekeeping`): scripts in `tools/`, warnings on and
  at zero. About 400 number-conversion warnings are switched off, not reviewed.
- Step 2 (branch `cleanup/sim-step`): `eGameBoard::step`, `beginStep`, `endStep`.
- Step 3 (branch `cleanup/split-board`): `egameboard.cpp` and `eboardcity.cpp`
  each split into seven files by subject (`egameboardbuild.cpp`,
  `...disasters`, `...events`, `...military`, `...registry`, `...undo`;
  `eboardcitydistricts.cpp`, `...employment`, `...military`, `...people`,
  `...registry`, `...resources`). Functions were moved whole; each new file
  carries a copy of the old include list, which could be trimmed.
  Per-city data still on the board: `mLastEmploymentState` (not saved, so the
  workers-needed message can repeat after a load), `mUpdateAppeal`,
  `mDefeatedBy`.
- Each branch builds on the one before: `ccr-3448c71b-u8j6yh` ->
  `cleanup/safety-net` -> `save-load` -> `findings` -> `housekeeping` ->
  `sim-step` -> `split-board`. `main` is untouched (Nicolás, 13 Nov).
- Next: step 4 (numbers into `enumbers.cpp`).

Put the parts mods will touch first.

1. **Housekeeping.**
   - Move `eZeus-tools` (`build.cmd`, `deploy.ps1`) into the repo as
     `tools/`, so the build is versioned.
   - Turn on compiler warnings at level `/W3` and fix what matters. The log
     shows no warnings now, because the warning level is low.
2. **Split the simulation step from painting.** This is needed by the safety
   net anyway. `egamewidgetpaint.cpp` mixes drawing with game logic.
3. **`egameboard.cpp` and `eboardcity.cpp`.**
   - Split by subject: undo, messages, events, resources, military,
     finances.
   - Check that per-city data lives in `eBoardCity` and not in
     `eGameBoard`.
4. **Numbers.**
   - Move numbers that are hard-coded in C++ into `enumbers.cpp` /
     `numbers.txt`, where mods will tune them.
   - Keep the source comment next to each number.
   - Don't change any value. The net must stay identical.
5. **Buildings, gods, heroes.** These are the areas the roadmap's mod
   features (sanctuaries, goods economy, farms, walker gates) will extend.
   Look for copy-pasted logic between similar buildings.
6. **The side panel (`widgets/datawidgets/`).**
   - Every tab re-reads data every 20 frames.
   - Text is redrawn even when unchanged. `eLabelBase::setText` and
     `setFontColor` rebuild the texture each call, so skip it when nothing
     changed.
7. **Optimisation.**
   - Measure first, with a long headless run and a timer per subsystem.
   - Don't optimise blind. Likely spots are per-tick loops over every tile
     or building, and the thread pool.

Leave alone anything that works and that mods won't touch.

### Quirks known from this phase

- `drawBuildingModes` only runs for buildings on screen. The Security
  overlay for hidden elite houses is driven from their last floor tile.
- `eTexture` text wrapping: spaces were doubled before 13 Nov
  (`etexture.cpp` `textLines`). Check other text code for the same habit.
- The Overview employment entry had its conditions in the wrong order, so
  "workers needed" never showed. Similar logic slips are likely elsewhere
  in the side panel.
- `eButton::sCreate` sets the disabled picture to frame 3. Buttons from
  other picture sets may not have one.

## Decisions for Nicolás (ask, don't assume)

1. **Merge the work branch into `main`?** `main` is far behind; everything
   since the start lives on `ccr-3448c71b-u8j6yh`. Merging gives the
   cleanup a clean starting point. It changes `main` on GitHub, so it needs
   his explicit OK.
2. **A new branch for the cleanup**, for example `cleanup/safety-net` and
   then one per step, as `CLAUDE.md` asks.
3. **Delete the "GG Aperta" test city** from the game folder.
4. **The `CLAUDE.md` upstream paragraph:** his edit.

## After this work

Pick up the mod roadmap in `mod-features.md` (sanctuary costs, sanctuaries
needing sacrifices, goods economy, farm seasons, walker gates, food quality,
cisterns, invasion screen). Remaining gaps from the original are in
`missing-from-original.md`:
- F6/F7 screen sizes;
- cosmetic cheats;
- the home page redo, U1 in `playtest-bugs.md`;
- B3 and B11, which have no numbers in any source.
