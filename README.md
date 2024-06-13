# Dragonrose Chess Engine
**Dragonrose is a weak chess engine written in C.** This is merely a passion project of mine, and I certainly don't expect much to come out of it, but I hope to learn a bit more about C and programming in general through this experience. On top of this, the engine will be written with readability in mind, so 1) it
is easier to understand and 2) I am not good enough to optimise the code much :P. <br>
Although this engine is probably too trash for anyone to bother using as a reference, you are free to borrow and modify my code if you so wish, as long as you give credit to both me and Bluefever Software (see below for details). <br>
To use the engine, either grab the binary from releases, or build the project locally. In the future I will consider releasing Linux builds. <br>

## Dragonrose and VICE

Dragonrose is based on VICE, a chess engine written by Richard Allbert (Bluefever Software), which is a didactic chess engine aimed to introduce beginners to the world of chess programming. Huge credits to him for creating the YouTube series explaining the engine in great detail. <br>

You can find the playlist here: [Link to playlist](https://www.youtube.com/playlist?list=PLZ1QII7yudbc-Ky058TEaOstZHVbT-2hg)

## Main features:

Search:
- Negamax Alpha-beta search
  - PV-search
  - Null-move pruning
  - Futility pruning
- Quiesence search
  - Delta pruning
- Iterative deepening
- Transposition table using "age"
- MVV/LVA move ordering
- Killer heuristics
- Polyglot opening books

Evaluation:
- Tapered eval
  - Material
  - Piece-square table bonuses
- King safety: pawn shield, open files, king tropism
- Piece bonuses: Rook/queen open-file bonuses
- Pawn bonuses: Passed pawns, isolated pawns

## Credits
- Richard Allbert (Bluefever Software) for VICE code and video playlist. The entire reason this engine even exists.
- nemequ, mbitsnbites, zhuyie et al. for [TinyCThread](https://github.com/tinycthread/tinycthread/tree/master).
- Chess Programming [Wiki](https://www.chessprogramming.org/Main_Page). Great resource in general to learn concepts.
- Witek902 for [Caissa](https://github.com/Witek902/Caissa) chess engine. I borrowed tapered eval from there.

## Changelogs: <br>
### 0.x: <br>
0.26: Added king tropism. Improved time management.
0.25: Added futility pruning. | Elo gain: ~20. <br> 
0.24c: Enabled O3 optimisation. Added mate ouptut. <br>
0.24b: Fixed delta pruning. Optimised memory (variable sizes). <br>
0.24: Added delta pruning. Added punishments for minor pieces in front of pawns. | Elo gain: ~40. <br>
0.23: Added king safety to evaluation (pawn shield and punish open files near king). <br>
0.22: Improved time management (tested on Lichess). Slight speed boost. <br>
0.21: Slight speed boost. <br>
0.2: Fixed crash. Improved tapered eval. | Elo gain: ~240. <br>
0.11: Added tapered eval to material (point values). <br>
0.1: Added tapered eval to PSQT. <br>

## To-do list:
- Tune futility pruning
- Add SEE
- Add aspiration windows
- Pawn / bishop interaction
- Optimise movegen (magic bitboard)
- Search thread / LazySMP

## Bugs to fix:
- The PickMove function needs a BestScore of -(very low) instead of 0
- ID loop needs to only exit when it has a legal move (i.e done depth 1 at least)
