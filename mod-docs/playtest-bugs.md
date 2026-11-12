# Playtest bugs

Bugs found while playing eZeus-dev, newest first within each section. Move an entry to "Fixed" with the date and commit when it's done.

## Open

| # | Found | Bug | What we know |
|---|---|---|---|
| P1 | 12 Nov 2026 | Event history stays empty | The side panel's message box (the original shows the latest message there, with up/down arrows to go through earlier ones) is never filled, and the scroll button for the message list (`coll.fMessages` in `widgets/egamemenu.cpp`) has no action. eZeus never built a message history (upstream issue #51). The row of four event icons above it (`eEventWidget`) does work. |
| P2 | 12 Nov 2026 | Decoration switches are ignored: every decoration can be built in a campaign that should forbid some | `eAvailableBuildings::available` (`buildings/eavailablebuildings.cpp`) covers farms, industry, sanctuaries, hero halls and commemorative monuments, but every decoration (parks, statues, gardens...) falls through to `default: return true`. Still to find: where the original's adventure files keep the decoration switches and whether eZeus reads them. |

## Fixed

| # | Found | Bug | Fix |
|---|---|---|---|
| P3 | 12 Nov 2026 | Message text should be pale, as in the original | The original writes the text inside a message's dark box in the same pale style as the side panel; eZeus used the dark carved style there. The message box's inner box now uses the pale style (12 Nov 2026). |
