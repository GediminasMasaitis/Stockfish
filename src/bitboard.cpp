/*
  Stockfish, a UCI chess playing engine derived from Glaurung 2.1
  Copyright (C) 2004-2021 The Stockfish developers (see AUTHORS file)

  Stockfish is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Stockfish is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <algorithm>
#include <bitset>
#include <array>

#include "bitboard.h"
#include "misc.h"

namespace Stockfish {

uint8_t PopCnt16[1 << 16];
uint8_t SquareDistance[SQUARE_NB][SQUARE_NB];

Bitboard SquareBB[SQUARE_NB];
Bitboard LineBB[SQUARE_NB][SQUARE_NB];
Bitboard BetweenBB[SQUARE_NB][SQUARE_NB];
Bitboard PseudoAttacks[PIECE_TYPE_NB][SQUARE_NB];
Bitboard PawnAttacks[COLOR_NB][SQUARE_NB];

Magic<  ROOK> RookMagics[SQUARE_NB];
Magic<BISHOP> BishopMagics[SQUARE_NB];

namespace {
  using KnownMagicArray = std::array<KnownMagic, SQUARE_NB>;

  // If using magic bitboards, it's possible to reduce the size of the
  // attack table (699 kB instead of 841 kB) by using specific offsets
  // in the table for each piece, to allow overlaps wherever possible.
  // We use magics and offsets originally found by Volker Annuss.
  constexpr KnownMagicArray KnownRookMagics = {
    KnownMagic { 0x00280077ffebfffeu,  41305 },
    KnownMagic { 0x2004010201097fffu,  14326 },
    KnownMagic { 0x0010020010053fffu,  24477 },
    KnownMagic { 0x0030002ff71ffffau,   8223 },
    KnownMagic { 0x7fd00441ffffd003u,  49795 },
    KnownMagic { 0x004001d9e03ffff7u,  60546 },
    KnownMagic { 0x004000888847ffffu,  28543 },
    KnownMagic { 0x006800fbff75fffdu,  79282 },
    KnownMagic { 0x000028010113ffffu,   6457 },
    KnownMagic { 0x0020040201fcffffu,   4125 },
    KnownMagic { 0x007fe80042ffffe8u,  81021 },
    KnownMagic { 0x00001800217fffe8u,  42341 },
    KnownMagic { 0x00001800073fffe8u,  14139 },
    KnownMagic { 0x007fe8009effffe8u,  19465 },
    KnownMagic { 0x00001800602fffe8u,   9514 },
    KnownMagic { 0x000030002fffffa0u,  71090 },
    KnownMagic { 0x00300018010bffffu,  75419 },
    KnownMagic { 0x0003000c0085fffbu,  33476 },
    KnownMagic { 0x0004000802010008u,  27117 },
    KnownMagic { 0x0002002004002002u,  85964 },
    KnownMagic { 0x0002002020010002u,  54915 },
    KnownMagic { 0x0001002020008001u,  36544 },
    KnownMagic { 0x0000004040008001u,  71854 },
    KnownMagic { 0x0000802000200040u,  37996 },
    KnownMagic { 0x0040200010080010u,  30398 },
    KnownMagic { 0x0000080010040010u,  55939 },
    KnownMagic { 0x0004010008020008u,  53891 },
    KnownMagic { 0x0000040020200200u,  56963 },
    KnownMagic { 0x0000010020020020u,  77451 },
    KnownMagic { 0x0000010020200080u,  12319 },
    KnownMagic { 0x0000008020200040u,  88500 },
    KnownMagic { 0x0000200020004081u,  51405 },
    KnownMagic { 0x00fffd1800300030u,  72878 },
    KnownMagic { 0x007fff7fbfd40020u,    676 },
    KnownMagic { 0x003fffbd00180018u,  83122 },
    KnownMagic { 0x001fffde80180018u,  22206 },
    KnownMagic { 0x000fffe0bfe80018u,  75186 },
    KnownMagic { 0x0001000080202001u,    681 },
    KnownMagic { 0x0003fffbff980180u,  36453 },
    KnownMagic { 0x0001fffdff9000e0u,  20369 },
    KnownMagic { 0x00fffeebfeffd800u,   1981 },
    KnownMagic { 0x007ffff7ffc01400u,  13343 },
    KnownMagic { 0x0000408104200204u,  10650 },
    KnownMagic { 0x001ffff01fc03000u,  57987 },
    KnownMagic { 0x000fffe7f8bfe800u,  26302 },
    KnownMagic { 0x0000008001002020u,  58357 },
    KnownMagic { 0x0003fff85fffa804u,  40546 },
    KnownMagic { 0x0001fffd75ffa802u,      0 },
    KnownMagic { 0x00ffffec00280028u,  14967 },
    KnownMagic { 0x007fff75ff7fbfd8u,  80361 },
    KnownMagic { 0x003fff863fbf7fd8u,  40905 },
    KnownMagic { 0x001fffbfdfd7ffd8u,  58347 },
    KnownMagic { 0x000ffff810280028u,  20381 },
    KnownMagic { 0x0007ffd7f7feffd8u,  81868 },
    KnownMagic { 0x0003fffc0c480048u,  59381 },
    KnownMagic { 0x0001ffffafd7ffd8u,  84404 },
    KnownMagic { 0x00ffffe4ffdfa3bau,  45811 },
    KnownMagic { 0x007fffef7ff3d3dau,  62898 },
    KnownMagic { 0x003fffbfdfeff7fau,  45796 },
    KnownMagic { 0x001fffeff7fbfc22u,  66994 },
    KnownMagic { 0x0000020408001001u,  67204 },
    KnownMagic { 0x0007fffeffff77fdu,  32448 },
    KnownMagic { 0x0003ffffbf7dfeecu,  62946 },
    KnownMagic { 0x0001ffff9dffa333u,  17005 }
  };

  constexpr KnownMagicArray KnownBishopMagics = {
    KnownMagic { 0x0000404040404040u,  33104 },
    KnownMagic { 0x0000a060401007fcu,   4094 },
    KnownMagic { 0x0000401020200000u,  24764 },
    KnownMagic { 0x0000806004000000u,  13882 },
    KnownMagic { 0x0000440200000000u,  23090 },
    KnownMagic { 0x0000080100800000u,  32640 },
    KnownMagic { 0x0000104104004000u,  11558 },
    KnownMagic { 0x0000020020820080u,  32912 },
    KnownMagic { 0x0000040100202004u,  13674 },
    KnownMagic { 0x0000020080200802u,   6109 },
    KnownMagic { 0x0000010040080200u,  26494 },
    KnownMagic { 0x0000008060040000u,  17919 },
    KnownMagic { 0x0000004402000000u,  25757 },
    KnownMagic { 0x00000021c100b200u,  17338 },
    KnownMagic { 0x0000000400410080u,  16983 },
    KnownMagic { 0x000003f7f05fffc0u,  16659 },
    KnownMagic { 0x0004228040808010u,  13610 },
    KnownMagic { 0x0000200040404040u,   2224 },
    KnownMagic { 0x0000400080808080u,  60405 },
    KnownMagic { 0x0000200200801000u,   7983 },
    KnownMagic { 0x0000240080840000u,     17 },
    KnownMagic { 0x000018000c03fff8u,  34321 },
    KnownMagic { 0x00000a5840208020u,  33216 },
    KnownMagic { 0x0000058408404010u,  17127 },
    KnownMagic { 0x0002022000408020u,   6397 },
    KnownMagic { 0x0000402000408080u,  22169 },
    KnownMagic { 0x0000804000810100u,  42727 },
    KnownMagic { 0x000100403c0403ffu,    155 },
    KnownMagic { 0x00078402a8802000u,   8601 },
    KnownMagic { 0x0000101000804400u,  21101 },
    KnownMagic { 0x0000080800104100u,  29885 },
    KnownMagic { 0x0000400480101008u,  29340 },
    KnownMagic { 0x0001010102004040u,  19785 },
    KnownMagic { 0x0000808090402020u,  12258 },
    KnownMagic { 0x0007fefe08810010u,  50451 },
    KnownMagic { 0x0003ff0f833fc080u,   1712 },
    KnownMagic { 0x007fe08019003042u,  78475 },
    KnownMagic { 0x0000202040008040u,   7855 },
    KnownMagic { 0x0001004008381008u,  13642 },
    KnownMagic { 0x0000802003700808u,   8156 },
    KnownMagic { 0x0000208200400080u,   4348 },
    KnownMagic { 0x0000104100200040u,  28794 },
    KnownMagic { 0x0003ffdf7f833fc0u,  22578 },
    KnownMagic { 0x0000008840450020u,  50315 },
    KnownMagic { 0x0000020040100100u,  85452 },
    KnownMagic { 0x007fffdd80140028u,  32816 },
    KnownMagic { 0x0000202020200040u,  13930 },
    KnownMagic { 0x0001004010039004u,  17967 },
    KnownMagic { 0x0000040041008000u,  33200 },
    KnownMagic { 0x0003ffefe0c02200u,  32456 },
    KnownMagic { 0x0000001010806000u,   7762 },
    KnownMagic { 0x0000000008403000u,   7794 },
    KnownMagic { 0x0000000100202000u,  22761 },
    KnownMagic { 0x0000040100200800u,  14918 },
    KnownMagic { 0x0000404040404000u,  11620 },
    KnownMagic { 0x00006020601803f4u,  15925 },
    KnownMagic { 0x0003ffdfdfc28048u,  32528 },
    KnownMagic { 0x0000000820820020u,  12196 },
    KnownMagic { 0x0000000010108060u,  32720 },
    KnownMagic { 0x0000000000084030u,  26781 },
    KnownMagic { 0x0000000001002020u,  19817 },
    KnownMagic { 0x0000000040408020u,  24732 },
    KnownMagic { 0x0000004040404040u,  25468 },
    KnownMagic { 0x0000404040404040u,  10186 }
  };

  // If using PEXT indexing or 32 bit magics, do not use reduced table size.
  Bitboard SlideAttackTable[HasPext || !Is64Bit ? 0x19000 + 0x1480 : 89524];

  template<PieceType Pt>
  void init_magics(Bitboard table[], Magic<Pt> magics[]);
}

/// safe_destination() returns the bitboard of target square for the given step
/// from the given square. If the step is off the board, returns empty bitboard.

inline Bitboard safe_destination(Square s, int step) {
    Square to = Square(s + step);
    return is_ok(to) && distance(s, to) <= 2 ? square_bb(to) : Bitboard(0);
}


/// Bitboards::pretty() returns an ASCII representation of a bitboard suitable
/// to be printed to standard output. Useful for debugging.

std::string Bitboards::pretty(Bitboard b) {

  std::string s = "+---+---+---+---+---+---+---+---+\n";

  for (Rank r = RANK_8; r >= RANK_1; --r)
  {
      for (File f = FILE_A; f <= FILE_H; ++f)
          s += b & make_square(f, r) ? "| X " : "|   ";

      s += "| " + std::to_string(1 + r) + "\n+---+---+---+---+---+---+---+---+\n";
  }
  s += "  a   b   c   d   e   f   g   h\n";

  return s;
}


/// Bitboards::init() initializes various bitboard tables. It is called at
/// startup and relies on global objects to be already zero-initialized.

void Bitboards::init() {

  for (unsigned i = 0; i < (1 << 16); ++i)
      PopCnt16[i] = uint8_t(std::bitset<16>(i).count());

  for (Square s = SQ_A1; s <= SQ_H8; ++s)
      SquareBB[s] = (1ULL << s);

  for (Square s1 = SQ_A1; s1 <= SQ_H8; ++s1)
      for (Square s2 = SQ_A1; s2 <= SQ_H8; ++s2)
          SquareDistance[s1][s2] = std::max(distance<File>(s1, s2), distance<Rank>(s1, s2));

  init_magics<  ROOK>(SlideAttackTable, RookMagics);
  init_magics<BISHOP>(SlideAttackTable, BishopMagics);

  for (Square s1 = SQ_A1; s1 <= SQ_H8; ++s1)
  {
      PawnAttacks[WHITE][s1] = pawn_attacks_bb<WHITE>(square_bb(s1));
      PawnAttacks[BLACK][s1] = pawn_attacks_bb<BLACK>(square_bb(s1));

      for (int step : {-9, -8, -7, -1, 1, 7, 8, 9} )
         PseudoAttacks[KING][s1] |= safe_destination(s1, step);

      for (int step : {-17, -15, -10, -6, 6, 10, 15, 17} )
         PseudoAttacks[KNIGHT][s1] |= safe_destination(s1, step);

      PseudoAttacks[QUEEN][s1]  = PseudoAttacks[BISHOP][s1] = attacks_bb<BISHOP>(s1, 0);
      PseudoAttacks[QUEEN][s1] |= PseudoAttacks[  ROOK][s1] = attacks_bb<  ROOK>(s1, 0);

      for (PieceType pt : { BISHOP, ROOK })
          for (Square s2 = SQ_A1; s2 <= SQ_H8; ++s2)
          {
              if (PseudoAttacks[pt][s1] & s2)
              {
                  LineBB[s1][s2]    = (attacks_bb(pt, s1, 0) & attacks_bb(pt, s2, 0)) | s1 | s2;
                  BetweenBB[s1][s2] = (attacks_bb(pt, s1, square_bb(s2)) & attacks_bb(pt, s2, square_bb(s1)));
              }
              BetweenBB[s1][s2] |= s2;
          }
  }
}

namespace {

  Bitboard sliding_attack(PieceType pt, Square sq, Bitboard occupied) {

    Bitboard attacks = 0;
    Direction   RookDirections[4] = {NORTH, SOUTH, EAST, WEST};
    Direction BishopDirections[4] = {NORTH_EAST, SOUTH_EAST, SOUTH_WEST, NORTH_WEST};

    for (Direction d : (pt == ROOK ? RookDirections : BishopDirections))
    {
        Square s = sq;
        while (safe_destination(s, d) && !(occupied & s))
            attacks |= (s += d);
    }

    return attacks;
  }


  // init_magics() computes all rook and bishop attacks at startup. Either magic
  // bitboards or PEXT indexing are used to look up attacks of sliding pieces.
  // As a reference see www.chessprogramming.org/Magic_Bitboards. In particular,
  // here we use the so called "fixed shift fancy magic bitboards" approach.
  // For 32 bits, we fall back to the regular "fancy magic bitboards" approach.
  template<PieceType Pt>
  void init_magics(Bitboard table[], Magic<Pt> magics[]) {
    // Optimal PRNG seeds to pick the correct magics in the shortest time
    int seeds[RANK_NB] = { 8977, 44560, 54343, 38998,  5731, 95205, 104912, 17020 };

    Bitboard occupancy[4096], reference[4096];
    int epoch[4096] = {}, cnt = 0, size = 0;

    for (Square s = SQ_A1; s <= SQ_H8; ++s)
    {
        // Board edges are not considered in the relevant occupancies
        Bitboard edges = ((Rank1BB | Rank8BB) & ~rank_bb(s)) | ((FileABB | FileHBB) & ~file_bb(s));

        Magic<Pt>& m = magics[s];
        KnownMagic knownMagic = Pt == ROOK ? KnownRookMagics[s] : KnownBishopMagics[s];

        // Given a square 's', the mask is the bitboard of sliding attacks from
        // 's' computed on an empty board.
        m.mask = sliding_attack(Pt, s, 0) & ~edges;

        // For 32 bit magics the index must be big enough to contain
        // all the attacks for each possible subset of the mask and so is 2 power
        // the number of 1s of the mask. Hence we deduce the size of the shift to
        // apply to the 64 or 32 bits word to get the index for a non-fixed shift.
        m.shift32 = 32 - popcount(m.mask);

        if (HasPext || !Is64Bit)
        {
            // For PEXT or fancy magic indexing, use the starting offset if on the
            // first square, and use the previous square's end offset as the current
            // square's starting offset. Rooks are stored in entries 0 through 0x18FFFF,
            // and bishops are stored in entries 0x190000 through 0x19147F.
            constexpr int startOffset = Pt == ROOK ? 0 : 0x19000;
            m.attacks = s == SQ_A1 ? table + startOffset : magics[s - 1].attacks + size;
        }
        else
        {
            // For magic bitboards indexing we use pre-computed magics values and
            // offsets. Since we are using the "fixed shift" approach we do not need
            // to calculate the shift, because the magic product will always resolve
            // a unique index using a 64-12 bit shift for rooks and 64-9 bit shift
            // for bishops.
            m.magic = knownMagic.magic;
            m.attacks = table + knownMagic.offset;
        }

        // Use Carry-Rippler trick to enumerate all subsets of masks[s] and
        // store the corresponding sliding attack bitboard in the attack table.
        Bitboard occupied = size = 0;
        do {
            occupancy[size] = occupied;
            reference[size] = sliding_attack(Pt, s, occupied);

            // If using PEXT we don't need magic numbers and can get index directly,
            // and if using 64 bit then existing magics are pre-computed.
            if (HasPext || Is64Bit) {
                unsigned index = m.index(occupied);
                m.attacks[index] = reference[size];
            }

            size++;
            occupied = (occupied - m.mask) & m.mask;
        } while (occupied);

        if (HasPext || Is64Bit)
            continue;

        PRNG rng(seeds[rank_of(s)]);
        // Find a magic for square 's' picking up an (almost) random number
        // until we find the one that passes the verification test.
        for (int i = 0; i < size; )
        {
            for (m.magic = 0; popcount((m.magic * m.mask) >> 56) < 6; )
                m.magic = rng.sparse_rand<Bitboard>();

            // A good magic must map every possible occupancy to an index that
            // looks up the correct sliding attack in the attacks[s] database.
            // Note that we build up the database for square 's' as a side
            // effect of verifying the magic. Keep track of the attempt count
            // and save it in epoch[], little speed-up trick to avoid resetting
            // m.attacks[] after every failed attempt.
            for (++cnt, i = 0; i < size; ++i)
            {
                unsigned idx = m.index(occupancy[i]);

                if (epoch[idx] < cnt)
                {
                    epoch[idx] = cnt;
                    m.attacks[idx] = reference[i];
                }
                else if (m.attacks[idx] != reference[i])
                    break;
            }
        }
    }
  }
}

} // namespace Stockfish
