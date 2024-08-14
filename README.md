# Dragonrose Chess Engine
**Dragonrose is a weak chess engine written in C.** This is merely a passion project of mine, and I certainly don't expect much to come out of it, but I hope to learn a bit more about C and programming in general through this experience. On top of this, the engine will be written with readability in mind, so 1) it
is easier to understand and 2) I am not good enough to optimise the code much :P. <br>
Although this engine is probably too trash for anyone to bother using as a reference, you are free to borrow and modify my code if you so wish, as long as you give credit to both me and Bluefever Software (see below for details). <br>
To use the engine, either grab the binary from releases, or build the project locally. In the future I will consider releasing Linux builds. <br>

## Dragonrose and VICE

Dragonrose is based on VICE, a chess engine written by Richard Allbert (Bluefever Software), which is a didactic chess engine aimed to introduce beginners to the world of chess programming. Huge credits to him for creating the YouTube series explaining the engine in great detail. <br>
Note that aside from most of the code itself, the internal book (VICEbook.bin) is also created by Allbert. <br>

You can find the playlist here: [Link to playlist](https://www.youtube.com/playlist?list=PLZ1QII7yudbc-Ky058TEaOstZHVbT-2hg)

## How to Use

- Challenge it on Lichess [here](https://lichess.org/@/DragonroseDev)
- To run it locally either download a binary from releases or build it yourself with the makefile (or CMake). With it you can pick one of two options:
  - Plug it into a chess GUI such as Arena or Cutechess
  - Directly run the executable (usually for testing). You can run it normally with ./Dragonrose or run a benchmark with ./Dragonrose bench

## UCI options

| Name  |      Type       | Default |  Valid values  | Description                                                                                             |
|:-----:|:---------------:|:-------:|:--------------:|:-------------------------------------------------------------------------------------------------------:|
| Hash  | integer (spin)  |    64   |   [1, 65536]   | Size of the transposition table in megabytes. 16 - 512 MB is recommended.                               |
| Book  | boolean (check) |  TRUE   |  TRUE / FALSE  | Whether to use the internal book (VICEbook.bin). The book and the binary must be in the same directory. |
| Bench |  CLI Argument   |    -    |        -       | Run `./Dragonrose bench` from a CLI to check nodes and NPS based on a 50-position suite (from [Heimdall](https://git.nocturn9x.space/nocturn9x/heimdall)). As it stands the node count is inconsistent even with the same version.|

## Main Features

Search:
- Negamax Alpha-beta search
  - PV-search
  - Null-move pruning
  - Futility pruning
  - Late move pruning
- Quiesence search
  - Delta pruning
- Move ordering: MVV/LVA, Killer heuristics, Priority moves (Castling, en passant)
- Iterative deepening
- Transposition table using "age"
- Polyglot opening books

Evaluation:
- Tapered eval
  - Material
  - Piece-square table bonuses
- King safety: pawn shield, open files, king tropism
- Piece bonuses: Rook/queen open-file bonuses
- Pawn bonuses: Passed pawns, isolated pawns, doubled pawns
- Drawn endgame detection: Basic material draw (e.g. K+R v K+B)

## Playing Strength

- Latest version is about 2350 CCRL in strength. At the moment it is quite inconsistent in tests, so the estimate may not be accurate.
- The Chesscom rating is estimated based on its games against human players (1800 - 2500). However it suffers greatly from small sample size, so take it with a grain of salt.

| Metric | Rapid | Blitz | Bullet |
| --- | --- | --- | --- |
| CCRL | 2350? | N/A | 2350? |
| Lichess (BOT) | 2221 ± 57 | 2074 ± 51 | 2104 ± 54 |
| Chesscom (est.) | 2591 ± 239 | 2760 ± 178 | 2659 ± 227 |

## Credits
- Richard Allbert (Bluefever Software) for VICE code and video playlist. The entire reason this engine even exists.
- [Chess Programming Wiki](https://www.chessprogramming.org/Main_Page). Great resource in general to learn concepts.
- Witek902 for [Caissa](https://github.com/Witek902/Caissa) chess engine. I borrowed its tapered eval weighting formula.
- nemequ, mbitsnbites, zhuyie et al. for [TinyCThread](https://github.com/tinycthread/tinycthread/tree/master).

## Changelogs <br>
### 0.x: <br>
0.28 (dev): Added extended futility pruning. Improved material draw detection. | Elo gain: ~15.
0.27c: Added doubled pawns. Improved time management. Minor code restructuring.
0.27b: Improved aspiration windows. Improved move ordering. Added OpenBench support. | Elo gain: ~5. <br>
0.27: Added late move pruning. Improved drawn endgame detection. Optimised king tropism. | Elo gain: ~50. <br>
0.26d: Added aspiration windows. <br>
0.26: Added king tropism. Improved time management. <br>
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

## To-do list
- Improve endgame knowledge
- Add SEE
- Pawn / bishop interaction
- Optimise movegen (magic bitboard)
- Search thread / LazySMP

## Bugs to fix:
- Non-deterministic node count
- May blunder threefold in a winning position due to how threefold is implemented
- Fix perft command freeze
- ID loop needs to only exit when it has a legal move (i.e done depth 1 at least)
- Obscure illegal move bug that occurs once every 100-200 games. Not replicable just with FEN.