# Dragonrose Chess Engine
**Dragonrose is a weak chess engine written in C.** This is merely a passion project of mine, and I certainly don't expect much to come out of it, but I hope to learn a bit more about C and programming in general through this experience.
Although this engine is probably too trash for anyone to bother using as a reference, feel free to borrow and modify my code if you so wish, as long as you give credit.

## Dragonrose and VICE

Dragonrose is based on VICE, a chess engine written by Richard Allbert (Bluefever Software), which is a didactic chess engine aimed to introduce beginners to the world of chess programming. Huge credits to him for creating the YouTube series explaining the engine in great detail.

You can find the playlist here: [Link to playlist](https://www.youtube.com/playlist?list=PLZ1QII7yudbc-Ky058TEaOstZHVbT-2hg)

## Main features:

Search:
- Negamax Alpha-beta search
- Quiesence search
- Iterative deepening
- Transposition table
  - Always replace
- MVV/LVA move ordering
- Null-move pruning
- Killer heuristics
- Polyglot opening books

Evaluation:
- Material
- Piece-square table bonuses
- Tapered eval
- Piece bonuses: Rook/queen open-file bonuses
- Pawn bonuses: Passed pawns, isolated pawns

## Changelogs:<br>
### 0.x:<br>
0.21: Minor speed boost.<br>
0.2: Fixed crash. Improved tapered eval. Elo gain: ~270<br>
0.11: Added tapered eval to material (point values) <br>
0.1: Added tapered eval to PSQT<br>

## Bugs to fix:
- The PickMove function needs a BestScore of -(very low) instead of 0
- Time management for x moves in x minutes causes losses
- ID loop needs to only exit when it has a legal move (i.e done depth 1 at least)
