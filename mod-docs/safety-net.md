# The safety net

An automatic check that the game still behaves the way it did before a code
change. Built on 13 Nov 2026, before any cleanup, because eZeus had no tests.

## In plain words

The check starts an adventure with no window and nobody playing. It builds
the same small test city every time (about 55 houses, fountains, an agora,
farms, sheep and goats, culture buildings, storage), lets the game run for
ten years and writes down what the city looks like at the end: population,
money, every building, stored goods, walkers, every event with its date.
It then saves the game, loads that save, and lets the loaded city run for
60 more days.

The same build always gives exactly the same result. So:

- **Result unchanged after a code change:** the change did not alter how
  the game plays (as far as the check can see).
- **Result changed:** the game now behaves differently. Either that was the
  point of the change (then the new result becomes the reference) or it is
  a mistake to track down before going on.

The reference results are kept in `tools/check/baseline/`, one text file per
adventure. They are plain text and can be read.

## How to run it

1. Build: `eZeus-tools\build.cmd`.
2. Run: `powershell -ExecutionPolicy Bypass -File tools\check\check.ps1`

It prints one line per adventure: `SAME`, `CHANGED` (with the first lines
that differ), `CRASHED`, or `UNSTEADY`. It takes a few minutes.

- `check.ps1 -Twice` also runs every adventure a second time and demands
  that both runs match. Use it when in doubt about the check itself.
- `check.ps1 -Update` keeps the new results as the reference. Only after
  looking at what changed and agreeing with it.

The game can stay open while the check runs: it uses its own copy of the
program (`Bin\eZeus-check.exe` in the game folder) and never touches
`eZeus-dev.exe`, your saves or your settings.

## What it covers

- The game does not crash while a city runs for ten years on each map.
- Housing, immigration, employment, food and goods production, storage,
  the agora, culture, taxes, hygiene, fires and collapses, popularity.
- The events the map's own scenario fires in those years (god visits,
  invasions, monsters, requests) and their dates.
- Saving and loading: a loaded save must look the same as the city that was
  saved, and keep behaving the same afterwards.
- Your own values in `numbers.txt` (the check reads the game folder's
  copy; the result notes its fingerprint).

Adventures used (`tools/check/maps.txt`): The Founding of Athens, The Sands
of Betrayal, Zeus and Europa, Athens through the Ages, Hercules' Labors, The
Trojan War, The Birth of Atlantis. The maps are read from the game folder;
none is copied into the repository.

## What it cannot catch

- **Anything on screen:** drawing, the interface, fonts, colours, the side
  panel, windows and menus. Nothing is drawn during the check.
- **Anything that needs a click:** placing buildings by hand, Undo,
  answering a request or an invasion, military orders, the world map,
  trade orders. Nobody answers, so those paths are not run.
- **Sound and music.**
- **Things the seven maps never reach in ten years**, or that the small
  test city has no building for: sanctuaries and heroes, elite housing and
  the army, trade posts, fleets, colonies, later episodes of an adventure.
- **Whether the behaviour is right.** The check only says "same as before".
  If something was already wrong when the reference was made, it stays
  wrong and the check stays green.
- **Speed.** A change that makes the game slower passes.

So the check does not replace playing. After each step you still get a short
list of what to try in the game.

## How it works (for the next session)

- Code: `eheadlesscheck.cpp` / `.h`, only in the developer build
  (`EZEUS_DEV_CHEATS`). `main.cpp` hands over to it when the command line
  has `--headless-check`.
- `eZeus-check.exe --headless-check <adventure> <years> [--seed N]
  [--out file] [--no-city] [--reload-days N] [--keep-saves] [--threads]
  [--trace N] [--trace-keep A B] [--fill N]`
- Made repeatable by:
  - `eRand::seed` (fixed seed; the game itself still seeds from the clock);
  - `eThreadPool::sSynchronous`: pathfinding tasks run at once on the main
    thread instead of on worker threads, so nothing depends on timing. The
    game itself keeps using threads.
- One step is what `eGameWidget::paintEvent` does at 100% speed: 100 ticks,
  then wait for the tasks. No picture or sound is ever loaded: every
  texture size is switched off, so the "load this picture" calls do nothing.
- The test city is placed with the game's own AI building code
  (`eAIDistrict::sBuild`), on the free block of land nearest to where
  settlers arrive, in three stages (day 0, 180, 540). Anything that burns
  down or collapses is built again, as a player would. Money is set to
  1,000,000 so the city is never lost to debt.
- Hunting tools, for when two runs of the same build differ:
  - `--trace N` prints a fingerprint of the saved game every N days; the
    first day on which two runs print different fingerprints is where they
    part ways (`--trace-keep A B` keeps the saves of those days to compare);
  - `--fill N` fills fresh memory with the byte N; two runs with different
    N differ exactly where the code saves or uses a value it never set.

## When the result changes on purpose

Changing a game value, fixing a bug in the simulation, changing the save
format or editing `numbers.txt` all change the result. Look at what changed
(`check.ps1` prints the first differing lines; the full new result is in
`tools/check/out/`), make sure it is what you meant, then run
`check.ps1 -Update` and commit the new baseline with the change.

## The third hunting tool

- `--rand-log file` notes, for every random number the game asks for, the
  place in the code that asked. Two runs that behave the same write the
  same file. `eZeus-check.exe --headless-check --rand-diff fileA fileB`
  prints the first place where the two runs part, with function names and
  source lines. This is what found the boar bug below within minutes, after
  comparing saves had not.

## What building it turned up (13 Nov 2026)

Making two runs match exactly meant finding everything that made the game
behave or save differently from one run to the next. Fixed along the way
(none changes a game value):

- **Hunters and boars** (`eHuntAction::mJudged`): a hunter remembered the
  boars he had already "judged" (1 in 5 charges him) by where they sat in
  the computer's memory. When a boar died and a new one happened to be put
  in the same spot, the new boar was skipped. Now the hunter can tell them
  apart. This was a real, if rare, bug in play.
- **Values saved before they were ever set** (the save held leftover
  memory): a delivery cart's task (`eCartTask`), dates that were never set
  (`eDate`), a monster event's monster (`eMonstersEventValue`), the Ares
  city of enlisted forces (`eEnlistedForces::fAresCity`), a walker's first
  movement target (`eMoveAction`). They now start with a fixed value.
- **Two lists saved in "memory order"** (patrol walkers' direction times,
  soldiers' places in a company): now saved in a fixed order. Old saves
  still load; the content is the same.

Seen but not fixed yet (for the bug-fixing phase; to check in the game
before deciding anything):

1. **A loaded save is not quite the city that was saved.** Fixed on
   13 Nov 2026 (branch `cleanup/save-load`), see the next section. What is
   left: loading re-deals the workers among the buildings at once instead
   of at the next regular moment (a building can show one worker more or
   fewer than before saving), and the save numbers things in a different
   order after a load. Neither loses anything.
2. **Taxes paid last year is 0.** Not a bug: as in the original, houses
   pay no tax until the city has a palace, and the test city had none. It
   has one now.
3. **555 walkers on The Sands of Betrayal.** Not a bug: that map has a
   computer-run rival city with some 170 delivery carts of its own. The
   check now lists walkers by city, and those out for more than a year.
4. **Agoras burning down.** A real bug, fixed; see "Other findings" below.
   The rest of the weak food supply was the test city's own layout (too
   few farms, a street out of reach of any maintenance office).
5. `eGameBoard::canBuildBase` takes (minX, maxX, minY, maxY) but the AI's
   pier code in `eaidistrict.cpp` passes (minX, minY, maxX, maxY). Only
   affects AI-built trade piers.
6. About 650 fields in the game's classes have no starting value in their
   declaration. Most are set in a constructor; the five above were not. A
   sweep for the rest is worth doing during cleanup (`--fill` shows the
   ones that reach a save).

## Save and load fixes (13 Nov 2026, branch `cleanup/save-load`)

The check showed that a loaded save was not the city that had been saved.
What was behind it:

- **Patrol walkers lost their memory on every load** (in eZeus since
  upstream). Each building that sends walkers along the roads (maintenance,
  water, tax, agora, culture, infirmary...) keeps a note of which way its
  walkers last turned at each crossing, so the next one goes the other way.
  Loading a save wiped those notes (they were read into the wrong place),
  so after every load walkers chose their way as if nobody had walked
  before. The save file also grew with every save and load, by about 10%
  each time. Fixed in `eDirectionLastUseTime::read`; saves made with the
  mistake are repaired when loaded.
- **Walkers out on the road and their building no longer shared those
  notes after a load**; each had its own copy. They are joined up again
  (`ePatrolAction::read`).
- **Agora stalls doubled their list of tiles on every load**
  (`eAgoraBase::setBuilding`).
- **The city's maintenance figure was not saved**, so it read 0 after a
  load until the next update, and that update came at once instead of when
  it was due. Both are saved now. This adds two numbers to the save: saves
  from older builds still load, saves from this build do not load in older
  builds (save format version 34).
- **Empty lists were not restored** (`mDefeatedBy`, an episode's events).
  Harmless, but it made a re-saved file differ.
- The "carrots 101, then 93 after loading" was not lost food. The city
  keeps a figure of its stored goods that it only refreshes now and then;
  the check now counts what is really in the storehouses.

On all seven adventures the reloaded city is now the same as the saved
one, and a re-saved file has the same size as the original.

A fourth hunting tool came out of this: `--save-who N` (and `--save2-who N`
for the save made after reloading) prints the exact code path that writes
byte N of the save. With a byte-by-byte comparison of two saves it tells
at once which value differs.

## Other findings (13 Nov 2026, branch `cleanup/findings`)

- **Every agora burned down sooner or later, and nobody could stop it.**
  When the fire risk was put on the agora as one building (to match the
  original's numbers), nothing told the game that the agora's stalls stand
  for it. Maintenance walkers passing by only "saw" the stalls and the
  road, so the agora's fire counter was never lowered and it caught fire
  after about 14 months on Hero, whatever the coverage. The fire was not
  drawn, and firefighters could not find it, so it burned until the agora
  collapsed. Now a stall (or an empty stall space) answers for its agora:
  maintenance reaches it, the fire shows on the stalls, firefighters put it
  out. No number changed (`eVendor`, `eAgoraSpace`: `isOnFire`,
  `setOnFire`, `provide`). In the check, the rival city on The Sands of
  Betrayal went from 1,512 to 3,302 people in ten years once its agora
  stopped burning.
- The test city changed with this step, on purpose: a palace (so taxes are
  paid), six farms instead of three, and a maintenance office on each of
  the two middle streets. The baseline was made again; results from before
  this step cannot be compared with it.
