# Dragonrose Chess Engine
**Dragonrose is a weak chess engine written in C.** This is merely a passion project of mine, and I certainly don't expect much to come out of it, but I hope to learn a bit more about C and programming in general through this experience.
Although this engine is probably too trash for anyone to bother using as a reference, feel free to borrow and modify my code if you so wish, as long as you give credit.

## Dragonrose and VICE

Dragonrose is based on VICE, a chess engine written by Richard Allbert (Bluefever Software), which is a didactic chess engine aimed to introduce beginners to the world of chess programming. Huge credits to him for creating the YouTube series explaining the engine in great detail.

You can find the playlist here: [Link to playlist](https://www.youtube.com/playlist?list=PLZ1QII7yudbc-Ky058TEaOstZHVbT-2hg)

## Main features:

Search:
- Alpha beta search
- Iterative deepening
- Quiescence search
- Transposition table
  - Always replace
- Polyglot opening books
- MVV/LVA move ordering

Evaluation:
- Material
- Piece Square Table Bonuses

## Bugs to fix:
- The PickMove function needs a BestScore of -(very low) instead of 0
- Time management for x moves in x minutes causes losses
- ID loop needs to only exit when it has a legal move (i.e done depth 1 at least)
