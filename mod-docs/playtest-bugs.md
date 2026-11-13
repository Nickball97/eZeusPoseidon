# Playtest bugs

Bugs found while playing eZeus-dev, newest first within each section. Move an entry to "Fixed" with the date and commit when it's done.

## Open

| # | Found | Bug | What we know |
|---|---|---|---|

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
| P2 | 12 Nov 2026 | Decoration switches seemed ignored | Already fixed on 11 Nov 2026 (commit 53e242fb, all 57 allowed-structures switches, decorations included). Checked 12 Nov 2026 by starting Athens through the Ages mission 1 fresh: the decorations menu offers only the Gazebo, as its switches say (bench, flower garden, hedge maze and fish pond off). Saves and custom adventures made before 11 Nov allow everything, which is likely what was seen. |
| P1 | 12 Nov 2026 | Event history stays empty (upstream #51) | Every message is now kept in the city's history (saved with the city; older saves start empty). The side panel box shows the latest one's short text, its arrows step through older ones and a click opens the full message. The Messages button opens the list (newest first, 10 rows with arrows): left-click reads, right-click deletes, "Delete opened messages" clears the read ones. Rules from zeus.exe, see original-exe-findings.md (12 Nov 2026). |
| P3 | 12 Nov 2026 | Message text should be pale, as in the original | The original writes the text inside a message's dark box in the same pale style as the side panel; eZeus used the dark carved style there. The message box's inner box now uses the pale style (12 Nov 2026). |
