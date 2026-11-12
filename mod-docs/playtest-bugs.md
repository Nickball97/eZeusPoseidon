# Playtest bugs

Bugs found while playing eZeus-dev, newest first within each section. Move an entry to "Fixed" with the date and commit when it's done.

## Open

| # | Found | Bug | What we know |
|---|---|---|---|
| P1 | 12 Nov 2026 | Event history stays empty | The side panel's message box (the original shows the latest message there, with up/down arrows to go through earlier ones) is never filled, and the scroll button for the message list (`coll.fMessages` in `widgets/egamemenu.cpp`) has no action. eZeus never built a message history (upstream issue #51). The row of four event icons above it (`eEventWidget`) does work. |
| P2 | 12 Nov 2026 | Decoration switches are ignored: every decoration can be built in a campaign that should forbid some | `eAvailableBuildings::available` (`buildings/eavailablebuildings.cpp`) covers farms, industry, sanctuaries, hero halls and commemorative monuments, but every decoration (parks, statues, gardens...) falls through to `default: return true`. Still to find: where the original's adventure files keep the decoration switches and whether eZeus reads them. |

## UI to redo

### U1. Home page (12 Nov 2026)

The original's screens, captured from Nicolás's Steam copy, are in `C:\Users\LENOVO\Fotos\Zeus Screenshots\Original home page` (outside the repo). The original goes through three screens where eZeus has one menu:

1. **Game chooser** (forest-and-temple painting, `Zeus_FE_MainMenu`): two framed panels with the Zeus and Poseidon logos, "Play Zeus" and "Play Poseidon" under them, "Poseidon Adventure Editor", "Activision Website" and "Quit", placed around the picture.
2. **Roster of Leaders** (Acropolis painting `Zeus_FE_Registry`, with the Zeus logo): the list of players, Create Leader, Delete Leader, Proceed, Return to main menu. eZeus has this window too, opened from the player's name button.
3. **Player menu** (forest-and-temple painting again): an empty framed panel at the top, the Zeus logo, the player's name, and Resume Adventure, Load Saved Adventure, Adventures, Custom Adventures, Tutorials and Return to Roster, scattered over the picture.

From there:
- **Adventures / Custom Adventures** (tree-and-sea painting `Zeus_FE_CampaignSelection`): a list window ("Choose an adventure", close button, "Begin Adventure"), a large framed picture of the selected adventure on the right, and a framed description under it (title in yellow).
- **Load Saved Adventure**: a dialog over the player menu ("Loading Adventure", file name, list, "Proceed?" with cancel and OK). eZeus's load window is already close to it.
- **Tutorials** draws incorrectly in the Steam copy (solid blue bars, a pink box); not a reference.

eZeus today: one menu on the Acropolis painting (New Adventure, Load Adventure, Adventure Editor, Options, Exit Game, the player's name button at the top left), and a single "new adventure" window. Options sit in the in-game menu in the original, not on the home page.

## Fixed

| # | Found | Bug | Fix |
|---|---|---|---|
| P3 | 12 Nov 2026 | Message text should be pale, as in the original | The original writes the text inside a message's dark box in the same pale style as the side panel; eZeus used the dark carved style there. The message box's inner box now uses the pale style (12 Nov 2026). |
