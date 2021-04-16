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

#ifndef BITBOARD_H_INCLUDED
#define BITBOARD_H_INCLUDED

#include <string>

#include "types.h"
#include "magicAttacks.h"

namespace Stockfish {

namespace Bitbases {

void init();
bool probe(Square wksq, Square wpsq, Square bksq, Color us);

} // namespace Stockfish::Bitbases

namespace Bitboards {

void init();
std::string pretty(Bitboard b);

} // namespace Stockfish::Bitboards

constexpr Bitboard AllSquares = ~Bitboard(0);
constexpr Bitboard DarkSquares = 0xAA55AA55AA55AA55ULL;

constexpr Bitboard FileABB = 0x0101010101010101ULL;
constexpr Bitboard FileBBB = FileABB << 1;
constexpr Bitboard FileCBB = FileABB << 2;
constexpr Bitboard FileDBB = FileABB << 3;
constexpr Bitboard FileEBB = FileABB << 4;
constexpr Bitboard FileFBB = FileABB << 5;
constexpr Bitboard FileGBB = FileABB << 6;
constexpr Bitboard FileHBB = FileABB << 7;

constexpr Bitboard Rank1BB = 0xFF;
constexpr Bitboard Rank2BB = Rank1BB << (8 * 1);
constexpr Bitboard Rank3BB = Rank1BB << (8 * 2);
constexpr Bitboard Rank4BB = Rank1BB << (8 * 3);
constexpr Bitboard Rank5BB = Rank1BB << (8 * 4);
constexpr Bitboard Rank6BB = Rank1BB << (8 * 5);
constexpr Bitboard Rank7BB = Rank1BB << (8 * 6);
constexpr Bitboard Rank8BB = Rank1BB << (8 * 7);

constexpr Bitboard QueenSide   = FileABB | FileBBB | FileCBB | FileDBB;
constexpr Bitboard CenterFiles = FileCBB | FileDBB | FileEBB | FileFBB;
constexpr Bitboard KingSide    = FileEBB | FileFBB | FileGBB | FileHBB;
constexpr Bitboard Center      = (FileDBB | FileEBB) & (Rank4BB | Rank5BB);

constexpr Bitboard KingFlank[FILE_NB] = {
  QueenSide ^ FileDBB, QueenSide, QueenSide,
  CenterFiles, CenterFiles,
  KingSide, KingSide, KingSide ^ FileEBB
};

extern uint8_t PopCnt16[1 << 16];
extern uint8_t SquareDistance[SQUARE_NB][SQUARE_NB];

extern Bitboard SquareBB[SQUARE_NB];
extern Bitboard BetweenBB[SQUARE_NB][SQUARE_NB];
extern Bitboard LineBB[SQUARE_NB][SQUARE_NB];
extern Bitboard PseudoAttacks[PIECE_TYPE_NB][SQUARE_NB];
extern Bitboard PawnAttacks[COLOR_NB][SQUARE_NB];


// Used for storing pre-computed magics and their positions in
// an attack table.
struct KnownMagic
{
  Bitboard magic;
  unsigned offset;
};

// Magic holds all magic bitboards relevant data for a single square
template<PieceType Pt>
struct Magic {
  Bitboard  mask;
  Bitboard  magic;
  const Bitboard* attacks;
  unsigned  shift32;

  // Compute the attack's index using either the BMI2 BEXT instruction,
  // or by the 'fixed shift fancy magic bitboards' approach.
  unsigned index(Bitboard occupied) const {

    if constexpr (HasPext)
        return unsigned(pext(occupied, mask));


    if constexpr (Is64Bit)
    {
        // Fixed shift - leave 12 bits for rooks, and 9 bits for bishops.
        constexpr unsigned fixedShift = 64 - (Pt == ROOK ? 12 : 9);
        return unsigned(((occupied & mask) * magic) >> fixedShift);
    }

    // When 64 bits are not available, can use multiple distinct
    // 32 bit operations in place of a 64-bit multiplication for a
    // significant performance, however this prevents use of the
    // current fixed shift magics implementation on 32 bit.
    unsigned lo = unsigned(occupied) & unsigned(mask);
    unsigned hi = unsigned(occupied >> 32) & unsigned(mask >> 32);
    return (lo * unsigned(magic) ^ hi * unsigned(magic >> 32)) >> shift32;
  }
};

constexpr std::array<Magic<ROOK>, SQUARE_NB> RookMagics = {
  Magic<ROOK> { 0x000101010101017e, 0x00280077ffebfffe, SlideAttackTable + 26304, 0 },
  Magic<ROOK> { 0x000202020202027c, 0x2004010201097fff, SlideAttackTable + 35520, 0 },
  Magic<ROOK> { 0x000404040404047a, 0x0010020010053fff, SlideAttackTable + 38592, 0 },
  Magic<ROOK> { 0x0008080808080876, 0x0040040008004002, SlideAttackTable + 8026, 0 },
  Magic<ROOK> { 0x001010101010106e, 0x7fd00441ffffd003, SlideAttackTable + 22196, 0 },
  Magic<ROOK> { 0x002020202020205e, 0x4020008887dffffe, SlideAttackTable + 80870, 0 },
  Magic<ROOK> { 0x004040404040403e, 0x004000888847ffff, SlideAttackTable + 76747, 0 },
  Magic<ROOK> { 0x008080808080807e, 0x006800fbff75fffd, SlideAttackTable + 30400, 0 },
  Magic<ROOK> { 0x0001010101017e00, 0x000028010113ffff, SlideAttackTable + 11115, 0 },
  Magic<ROOK> { 0x0002020202027c00, 0x0020040201fcffff, SlideAttackTable + 18205, 0 },
  Magic<ROOK> { 0x0004040404047a00, 0x007fe80042ffffe8, SlideAttackTable + 53577, 0 },
  Magic<ROOK> { 0x0008080808087600, 0x00001800217fffe8, SlideAttackTable + 62724, 0 },
  Magic<ROOK> { 0x0010101010106e00, 0x00001800073fffe8, SlideAttackTable + 34282, 0 },
  Magic<ROOK> { 0x0020202020205e00, 0x00001800e05fffe8, SlideAttackTable + 29196, 0 },
  Magic<ROOK> { 0x0040404040403e00, 0x00001800602fffe8, SlideAttackTable + 23806, 0 },
  Magic<ROOK> { 0x0080808080807e00, 0x000030002fffffa0, SlideAttackTable + 49481, 0 },
  Magic<ROOK> { 0x00010101017e0100, 0x00300018010bffff, SlideAttackTable + 2410, 0 },
  Magic<ROOK> { 0x00020202027c0200, 0x0003000c0085fffb, SlideAttackTable + 36498, 0 },
  Magic<ROOK> { 0x00040404047a0400, 0x0004000802010008, SlideAttackTable + 24478, 0 },
  Magic<ROOK> { 0x0008080808760800, 0x0004002020020004, SlideAttackTable + 10074, 0 },
  Magic<ROOK> { 0x00101010106e1000, 0x0001002002002001, SlideAttackTable + 79315, 0 },
  Magic<ROOK> { 0x00202020205e2000, 0x0001001000801040, SlideAttackTable + 51779, 0 },
  Magic<ROOK> { 0x00404040403e4000, 0x0000004040008001, SlideAttackTable + 13586, 0 },
  Magic<ROOK> { 0x00808080807e8000, 0x0000006800cdfff4, SlideAttackTable + 19323, 0 },
  Magic<ROOK> { 0x000101017e010100, 0x0040200010080010, SlideAttackTable + 70612, 0 },
  Magic<ROOK> { 0x000202027c020200, 0x0000080010040010, SlideAttackTable + 83652, 0 },
  Magic<ROOK> { 0x000404047a040400, 0x0004010008020008, SlideAttackTable + 63110, 0 },
  Magic<ROOK> { 0x0008080876080800, 0x0000040020200200, SlideAttackTable + 34496, 0 },
  Magic<ROOK> { 0x001010106e101000, 0x0002008010100100, SlideAttackTable + 84966, 0 },
  Magic<ROOK> { 0x002020205e202000, 0x0000008020010020, SlideAttackTable + 54341, 0 },
  Magic<ROOK> { 0x004040403e404000, 0x0000008020200040, SlideAttackTable + 60421, 0 },
  Magic<ROOK> { 0x008080807e808000, 0x0000820020004020, SlideAttackTable + 86402, 0 },
  Magic<ROOK> { 0x0001017e01010100, 0x00fffd1800300030, SlideAttackTable + 50245, 0 },
  Magic<ROOK> { 0x0002027c02020200, 0x007fff7fbfd40020, SlideAttackTable + 76622, 0 },
  Magic<ROOK> { 0x0004047a04040400, 0x003fffbd00180018, SlideAttackTable + 84676, 0 },
  Magic<ROOK> { 0x0008087608080800, 0x001fffde80180018, SlideAttackTable + 78757, 0 },
  Magic<ROOK> { 0x0010106e10101000, 0x000fffe0bfe80018, SlideAttackTable + 37346, 0 },
  Magic<ROOK> { 0x0020205e20202000, 0x0001000080202001, SlideAttackTable + 370, 0 },
  Magic<ROOK> { 0x0040403e40404000, 0x0003fffbff980180, SlideAttackTable + 42182, 0 },
  Magic<ROOK> { 0x0080807e80808000, 0x0001fffdff9000e0, SlideAttackTable + 45385, 0 },
  Magic<ROOK> { 0x00017e0101010100, 0x00fffefeebffd800, SlideAttackTable + 61659, 0 },
  Magic<ROOK> { 0x00027c0202020200, 0x007ffff7ffc01400, SlideAttackTable + 12790, 0 },
  Magic<ROOK> { 0x00047a0404040400, 0x003fffbfe4ffe800, SlideAttackTable + 16762, 0 },
  Magic<ROOK> { 0x0008760808080800, 0x001ffff01fc03000, SlideAttackTable + 0, 0 },
  Magic<ROOK> { 0x00106e1010101000, 0x000fffe7f8bfe800, SlideAttackTable + 38380, 0 },
  Magic<ROOK> { 0x00205e2020202000, 0x0007ffdfdf3ff808, SlideAttackTable + 11098, 0 },
  Magic<ROOK> { 0x00403e4040404000, 0x0003fff85fffa804, SlideAttackTable + 21803, 0 },
  Magic<ROOK> { 0x00807e8080808000, 0x0001fffd75ffa802, SlideAttackTable + 39189, 0 },
  Magic<ROOK> { 0x007e010101010100, 0x00ffffd7ffebffd8, SlideAttackTable + 58628, 0 },
  Magic<ROOK> { 0x007c020202020200, 0x007fff75ff7fbfd8, SlideAttackTable + 44116, 0 },
  Magic<ROOK> { 0x007a040404040400, 0x003fff863fbf7fd8, SlideAttackTable + 78357, 0 },
  Magic<ROOK> { 0x0076080808080800, 0x001fffbfdfd7ffd8, SlideAttackTable + 44481, 0 },
  Magic<ROOK> { 0x006e101010101000, 0x000ffff810280028, SlideAttackTable + 64134, 0 },
  Magic<ROOK> { 0x005e202020202000, 0x0007ffd7f7feffd8, SlideAttackTable + 41759, 0 },
  Magic<ROOK> { 0x003e404040404000, 0x0003fffc0c480048, SlideAttackTable + 1394, 0 },
  Magic<ROOK> { 0x007e808080808000, 0x0001ffffafd7ffd8, SlideAttackTable + 40910, 0 },
  Magic<ROOK> { 0x7e01010101010100, 0x00ffffe4ffdfa3ba, SlideAttackTable + 66516, 0 },
  Magic<ROOK> { 0x7c02020202020200, 0x007fffef7ff3d3da, SlideAttackTable + 3897, 0 },
  Magic<ROOK> { 0x7a04040404040400, 0x003fffbfdfeff7fa, SlideAttackTable + 3930, 0 },
  Magic<ROOK> { 0x7608080808080800, 0x001fffeff7fbfc22, SlideAttackTable + 72934, 0 },
  Magic<ROOK> { 0x6e10101010101000, 0x0000020408001001, SlideAttackTable + 72662, 0 },
  Magic<ROOK> { 0x5e20202020202000, 0x0007fffeffff77fd, SlideAttackTable + 56325, 0 },
  Magic<ROOK> { 0x3e40404040404000, 0x0003ffffbf7dfeec, SlideAttackTable + 66501, 0 },
  Magic<ROOK> { 0x7e80808080808000, 0x0001ffff9dffa333, SlideAttackTable + 14826, 0 },
};

constexpr std::array<Magic<BISHOP>, SQUARE_NB> BishopMagics = {
  Magic<BISHOP> { 0x0040201008040200, 0x007fbfbfbfbfbfff, SlideAttackTable + 5378, 0 },
  Magic<BISHOP> { 0x0000402010080400, 0x0000a060401007fc, SlideAttackTable + 4093, 0 },
  Magic<BISHOP> { 0x0000004020100a00, 0x0001004008020000, SlideAttackTable + 4314, 0 },
  Magic<BISHOP> { 0x0000000040221400, 0x0000806004000000, SlideAttackTable + 6587, 0 },
  Magic<BISHOP> { 0x0000000002442800, 0x0000100400000000, SlideAttackTable + 6491, 0 },
  Magic<BISHOP> { 0x0000000204085000, 0x000021c100b20000, SlideAttackTable + 6330, 0 },
  Magic<BISHOP> { 0x0000020408102000, 0x0000040041008000, SlideAttackTable + 5609, 0 },
  Magic<BISHOP> { 0x0002040810204000, 0x00000fb0203fff80, SlideAttackTable + 22236, 0 },
  Magic<BISHOP> { 0x0020100804020000, 0x0000040100401004, SlideAttackTable + 6106, 0 },
  Magic<BISHOP> { 0x0040201008040000, 0x0000020080200802, SlideAttackTable + 5625, 0 },
  Magic<BISHOP> { 0x00004020100a0000, 0x0000004010202000, SlideAttackTable + 16785, 0 },
  Magic<BISHOP> { 0x0000004022140000, 0x0000008060040000, SlideAttackTable + 16817, 0 },
  Magic<BISHOP> { 0x0000000244280000, 0x0000004402000000, SlideAttackTable + 6842, 0 },
  Magic<BISHOP> { 0x0000020408500000, 0x0000000801008000, SlideAttackTable + 7003, 0 },
  Magic<BISHOP> { 0x0002040810200000, 0x000007efe0bfff80, SlideAttackTable + 4197, 0 },
  Magic<BISHOP> { 0x0004081020400000, 0x0000000820820020, SlideAttackTable + 7356, 0 },
  Magic<BISHOP> { 0x0010080402000200, 0x0000400080808080, SlideAttackTable + 4602, 0 },
  Magic<BISHOP> { 0x0020100804000400, 0x00021f0100400808, SlideAttackTable + 4538, 0 },
  Magic<BISHOP> { 0x004020100a000a00, 0x00018000c06f3fff, SlideAttackTable + 29531, 0 },
  Magic<BISHOP> { 0x0000402214001400, 0x0000258200801000, SlideAttackTable + 45393, 0 },
  Magic<BISHOP> { 0x0000024428002800, 0x0000240080840000, SlideAttackTable + 12420, 0 },
  Magic<BISHOP> { 0x0002040850005000, 0x000018000c03fff8, SlideAttackTable + 15763, 0 },
  Magic<BISHOP> { 0x0004081020002000, 0x00000a5840208020, SlideAttackTable + 5050, 0 },
  Magic<BISHOP> { 0x0008102040004000, 0x0000020008208020, SlideAttackTable + 4346, 0 },
  Magic<BISHOP> { 0x0008040200020400, 0x0000804000810100, SlideAttackTable + 6074, 0 },
  Magic<BISHOP> { 0x0010080400040800, 0x0001011900802008, SlideAttackTable + 7866, 0 },
  Magic<BISHOP> { 0x0020100a000a1000, 0x0000804000810100, SlideAttackTable + 32139, 0 },
  Magic<BISHOP> { 0x0040221400142200, 0x000100403c0403ff, SlideAttackTable + 57673, 0 },
  Magic<BISHOP> { 0x0002442800284400, 0x00078402a8802000, SlideAttackTable + 55365, 0 },
  Magic<BISHOP> { 0x0004085000500800, 0x0000101000804400, SlideAttackTable + 15818, 0 },
  Magic<BISHOP> { 0x0008102000201000, 0x0000080800104100, SlideAttackTable + 5562, 0 },
  Magic<BISHOP> { 0x0010204000402000, 0x00004004c0082008, SlideAttackTable + 6390, 0 },
  Magic<BISHOP> { 0x0004020002040800, 0x0001010120008020, SlideAttackTable + 7930, 0 },
  Magic<BISHOP> { 0x0008040004081000, 0x000080809a004010, SlideAttackTable + 13329, 0 },
  Magic<BISHOP> { 0x00100a000a102000, 0x0007fefe08810010, SlideAttackTable + 7170, 0 },
  Magic<BISHOP> { 0x0022140014224000, 0x0003ff0f833fc080, SlideAttackTable + 27267, 0 },
  Magic<BISHOP> { 0x0044280028440200, 0x007fe08019003042, SlideAttackTable + 53787, 0 },
  Magic<BISHOP> { 0x0008500050080400, 0x003fffefea003000, SlideAttackTable + 5097, 0 },
  Magic<BISHOP> { 0x0010200020100800, 0x0000101010002080, SlideAttackTable + 6643, 0 },
  Magic<BISHOP> { 0x0020400040201000, 0x0000802005080804, SlideAttackTable + 6138, 0 },
  Magic<BISHOP> { 0x0002000204081000, 0x0000808080a80040, SlideAttackTable + 7418, 0 },
  Magic<BISHOP> { 0x0004000408102000, 0x0000104100200040, SlideAttackTable + 7898, 0 },
  Magic<BISHOP> { 0x000a000a10204000, 0x0003ffdf7f833fc0, SlideAttackTable + 42012, 0 },
  Magic<BISHOP> { 0x0014001422400000, 0x0000008840450020, SlideAttackTable + 57350, 0 },
  Magic<BISHOP> { 0x0028002844020000, 0x00007ffc80180030, SlideAttackTable + 22813, 0 },
  Magic<BISHOP> { 0x0050005008040200, 0x007fffdd80140028, SlideAttackTable + 56693, 0 },
  Magic<BISHOP> { 0x0020002010080400, 0x00020080200a0004, SlideAttackTable + 5818, 0 },
  Magic<BISHOP> { 0x0040004020100800, 0x0000101010100020, SlideAttackTable + 7098, 0 },
  Magic<BISHOP> { 0x0000020408102000, 0x0007ffdfc1805000, SlideAttackTable + 4451, 0 },
  Magic<BISHOP> { 0x0000040810204000, 0x0003ffefe0c02200, SlideAttackTable + 4709, 0 },
  Magic<BISHOP> { 0x00000a1020400000, 0x0000000820806000, SlideAttackTable + 4794, 0 },
  Magic<BISHOP> { 0x0000142240000000, 0x0000000008403000, SlideAttackTable + 13364, 0 },
  Magic<BISHOP> { 0x0000284402000000, 0x0000000100202000, SlideAttackTable + 4570, 0 },
  Magic<BISHOP> { 0x0000500804020000, 0x0000004040802000, SlideAttackTable + 4282, 0 },
  Magic<BISHOP> { 0x0000201008040200, 0x0004010040100400, SlideAttackTable + 14964, 0 },
  Magic<BISHOP> { 0x0000402010080400, 0x00006020601803f4, SlideAttackTable + 4026, 0 },
  Magic<BISHOP> { 0x0002040810204000, 0x0003ffdfdfc28048, SlideAttackTable + 4826, 0 },
  Magic<BISHOP> { 0x0004081020400000, 0x0000000820820020, SlideAttackTable + 7354, 0 },
  Magic<BISHOP> { 0x000a102040000000, 0x0000000008208060, SlideAttackTable + 4848, 0 },
  Magic<BISHOP> { 0x0014224000000000, 0x0000000000808020, SlideAttackTable + 15946, 0 },
  Magic<BISHOP> { 0x0028440200000000, 0x0000000001002020, SlideAttackTable + 14932, 0 },
  Magic<BISHOP> { 0x0050080402000000, 0x0000000401002008, SlideAttackTable + 16588, 0 },
  Magic<BISHOP> { 0x0020100804020000, 0x0000004040404040, SlideAttackTable + 6905, 0 },
  Magic<BISHOP> { 0x0040201008040200, 0x007fff9fdf7ff813, SlideAttackTable + 16076, 0 },
};

inline Bitboard square_bb(Square s) {
  assert(is_ok(s));
  return SquareBB[s];
}


/// Overloads of bitwise operators between a Bitboard and a Square for testing
/// whether a given bit is set in a bitboard, and for setting and clearing bits.

inline Bitboard  operator&( Bitboard  b, Square s) { return b &  square_bb(s); }
inline Bitboard  operator|( Bitboard  b, Square s) { return b |  square_bb(s); }
inline Bitboard  operator^( Bitboard  b, Square s) { return b ^  square_bb(s); }
inline Bitboard& operator|=(Bitboard& b, Square s) { return b |= square_bb(s); }
inline Bitboard& operator^=(Bitboard& b, Square s) { return b ^= square_bb(s); }

inline Bitboard  operator&(Square s, Bitboard b) { return b & s; }
inline Bitboard  operator|(Square s, Bitboard b) { return b | s; }
inline Bitboard  operator^(Square s, Bitboard b) { return b ^ s; }

inline Bitboard  operator|(Square s1, Square s2) { return square_bb(s1) | s2; }

constexpr bool more_than_one(Bitboard b) {
  return b & (b - 1);
}


constexpr bool opposite_colors(Square s1, Square s2) {
  return (s1 + rank_of(s1) + s2 + rank_of(s2)) & 1;
}


/// rank_bb() and file_bb() return a bitboard representing all the squares on
/// the given file or rank.

constexpr Bitboard rank_bb(Rank r) {
  return Rank1BB << (8 * r);
}

constexpr Bitboard rank_bb(Square s) {
  return rank_bb(rank_of(s));
}

constexpr Bitboard file_bb(File f) {
  return FileABB << f;
}

constexpr Bitboard file_bb(Square s) {
  return file_bb(file_of(s));
}


/// shift() moves a bitboard one or two steps as specified by the direction D

template<Direction D>
constexpr Bitboard shift(Bitboard b) {
  return  D == NORTH      ?  b             << 8 : D == SOUTH      ?  b             >> 8
        : D == NORTH+NORTH?  b             <<16 : D == SOUTH+SOUTH?  b             >>16
        : D == EAST       ? (b & ~FileHBB) << 1 : D == WEST       ? (b & ~FileABB) >> 1
        : D == NORTH_EAST ? (b & ~FileHBB) << 9 : D == NORTH_WEST ? (b & ~FileABB) << 7
        : D == SOUTH_EAST ? (b & ~FileHBB) >> 7 : D == SOUTH_WEST ? (b & ~FileABB) >> 9
        : 0;
}


/// pawn_attacks_bb() returns the squares attacked by pawns of the given color
/// from the squares in the given bitboard.

template<Color C>
constexpr Bitboard pawn_attacks_bb(Bitboard b) {
  return C == WHITE ? shift<NORTH_WEST>(b) | shift<NORTH_EAST>(b)
                    : shift<SOUTH_WEST>(b) | shift<SOUTH_EAST>(b);
}

inline Bitboard pawn_attacks_bb(Color c, Square s) {

  assert(is_ok(s));
  return PawnAttacks[c][s];
}


/// pawn_double_attacks_bb() returns the squares doubly attacked by pawns of the
/// given color from the squares in the given bitboard.

template<Color C>
constexpr Bitboard pawn_double_attacks_bb(Bitboard b) {
  return C == WHITE ? shift<NORTH_WEST>(b) & shift<NORTH_EAST>(b)
                    : shift<SOUTH_WEST>(b) & shift<SOUTH_EAST>(b);
}


/// adjacent_files_bb() returns a bitboard representing all the squares on the
/// adjacent files of a given square.

constexpr Bitboard adjacent_files_bb(Square s) {
  return shift<EAST>(file_bb(s)) | shift<WEST>(file_bb(s));
}


/// line_bb() returns a bitboard representing an entire line (from board edge
/// to board edge) that intersects the two given squares. If the given squares
/// are not on a same file/rank/diagonal, the function returns 0. For instance,
/// line_bb(SQ_C4, SQ_F7) will return a bitboard with the A2-G8 diagonal.

inline Bitboard line_bb(Square s1, Square s2) {

  assert(is_ok(s1) && is_ok(s2));

  return LineBB[s1][s2];
}


/// between_bb(s1, s2) returns a bitboard representing the squares in the semi-open
/// segment between the squares s1 and s2 (excluding s1 but including s2). If the
/// given squares are not on a same file/rank/diagonal, it returns s2. For instance,
/// between_bb(SQ_C4, SQ_F7) will return a bitboard with squares D5, E6 and F7, but
/// between_bb(SQ_E6, SQ_F8) will return a bitboard with the square F8. This trick
/// allows to generate non-king evasion moves faster: the defending piece must either
/// interpose itself to cover the check or capture the checking piece.

inline Bitboard between_bb(Square s1, Square s2) {

  assert(is_ok(s1) && is_ok(s2));

  return BetweenBB[s1][s2];
}


/// forward_ranks_bb() returns a bitboard representing the squares on the ranks in
/// front of the given one, from the point of view of the given color. For instance,
/// forward_ranks_bb(BLACK, SQ_D3) will return the 16 squares on ranks 1 and 2.

constexpr Bitboard forward_ranks_bb(Color c, Square s) {
  return c == WHITE ? ~Rank1BB << 8 * relative_rank(WHITE, s)
                    : ~Rank8BB >> 8 * relative_rank(BLACK, s);
}


/// forward_file_bb() returns a bitboard representing all the squares along the
/// line in front of the given one, from the point of view of the given color.

constexpr Bitboard forward_file_bb(Color c, Square s) {
  return forward_ranks_bb(c, s) & file_bb(s);
}


/// pawn_attack_span() returns a bitboard representing all the squares that can
/// be attacked by a pawn of the given color when it moves along its file, starting
/// from the given square.

constexpr Bitboard pawn_attack_span(Color c, Square s) {
  return forward_ranks_bb(c, s) & adjacent_files_bb(s);
}


/// passed_pawn_span() returns a bitboard which can be used to test if a pawn of
/// the given color and on the given square is a passed pawn.

constexpr Bitboard passed_pawn_span(Color c, Square s) {
  return pawn_attack_span(c, s) | forward_file_bb(c, s);
}


/// aligned() returns true if the squares s1, s2 and s3 are aligned either on a
/// straight or on a diagonal line.

inline bool aligned(Square s1, Square s2, Square s3) {
  return line_bb(s1, s2) & s3;
}


/// distance() functions return the distance between x and y, defined as the
/// number of steps for a king in x to reach y.

template<typename T1 = Square> inline int distance(Square x, Square y);
template<> inline int distance<File>(Square x, Square y) { return std::abs(file_of(x) - file_of(y)); }
template<> inline int distance<Rank>(Square x, Square y) { return std::abs(rank_of(x) - rank_of(y)); }
template<> inline int distance<Square>(Square x, Square y) { return SquareDistance[x][y]; }

inline int edge_distance(File f) { return std::min(f, File(FILE_H - f)); }
inline int edge_distance(Rank r) { return std::min(r, Rank(RANK_8 - r)); }


/// attacks_bb(Square) returns the pseudo attacks of the give piece type
/// assuming an empty board.

template<PieceType Pt>
inline Bitboard attacks_bb(Square s) {

  assert((Pt != PAWN) && (is_ok(s)));

  return PseudoAttacks[Pt][s];
}


/// attacks_bb(Square, Bitboard) returns the attacks by the given piece
/// assuming the board is occupied according to the passed Bitboard.
/// Sliding piece attacks do not continue passed an occupied square.

template<PieceType Pt>
inline Bitboard attacks_bb(Square s, Bitboard occupied) {

  assert((Pt != PAWN) && (is_ok(s)));

  switch (Pt)
  {
  case BISHOP: return BishopMagics[s].attacks[BishopMagics[s].index(occupied)];
  case ROOK  : return   RookMagics[s].attacks[  RookMagics[s].index(occupied)];
  case QUEEN : return attacks_bb<BISHOP>(s, occupied) | attacks_bb<ROOK>(s, occupied);
  default    : return PseudoAttacks[Pt][s];
  }
}

inline Bitboard attacks_bb(PieceType pt, Square s, Bitboard occupied) {

  assert((pt != PAWN) && (is_ok(s)));

  switch (pt)
  {
  case BISHOP: return attacks_bb<BISHOP>(s, occupied);
  case ROOK  : return attacks_bb<  ROOK>(s, occupied);
  case QUEEN : return attacks_bb<BISHOP>(s, occupied) | attacks_bb<ROOK>(s, occupied);
  default    : return PseudoAttacks[pt][s];
  }
}


/// popcount() counts the number of non-zero bits in a bitboard

inline int popcount(Bitboard b) {

#ifndef USE_POPCNT

  union { Bitboard bb; uint16_t u[4]; } v = { b };
  return PopCnt16[v.u[0]] + PopCnt16[v.u[1]] + PopCnt16[v.u[2]] + PopCnt16[v.u[3]];

#elif defined(_MSC_VER) || defined(__INTEL_COMPILER)

  return (int)_mm_popcnt_u64(b);

#else // Assumed gcc or compatible compiler

  return __builtin_popcountll(b);

#endif
}


/// lsb() and msb() return the least/most significant bit in a non-zero bitboard

#if defined(__GNUC__)  // GCC, Clang, ICC

inline Square lsb(Bitboard b) {
  assert(b);
  return Square(__builtin_ctzll(b));
}

inline Square msb(Bitboard b) {
  assert(b);
  return Square(63 ^ __builtin_clzll(b));
}

#elif defined(_MSC_VER)  // MSVC

#ifdef _WIN64  // MSVC, WIN64

inline Square lsb(Bitboard b) {
  assert(b);
  unsigned long idx;
  _BitScanForward64(&idx, b);
  return (Square) idx;
}

inline Square msb(Bitboard b) {
  assert(b);
  unsigned long idx;
  _BitScanReverse64(&idx, b);
  return (Square) idx;
}

#else  // MSVC, WIN32

inline Square lsb(Bitboard b) {
  assert(b);
  unsigned long idx;

  if (b & 0xffffffff) {
      _BitScanForward(&idx, int32_t(b));
      return Square(idx);
  } else {
      _BitScanForward(&idx, int32_t(b >> 32));
      return Square(idx + 32);
  }
}

inline Square msb(Bitboard b) {
  assert(b);
  unsigned long idx;

  if (b >> 32) {
      _BitScanReverse(&idx, int32_t(b >> 32));
      return Square(idx + 32);
  } else {
      _BitScanReverse(&idx, int32_t(b));
      return Square(idx);
  }
}

#endif

#else  // Compiler is neither GCC nor MSVC compatible

#error "Compiler not supported."

#endif

/// least_significant_square_bb() returns the bitboard of the least significant
/// square of a non-zero bitboard. It is equivalent to square_bb(lsb(bb)).

inline Bitboard least_significant_square_bb(Bitboard b) {
  assert(b);
  return b & -b;
}

/// pop_lsb() finds and clears the least significant bit in a non-zero bitboard

inline Square pop_lsb(Bitboard& b) {
  assert(b);
  const Square s = lsb(b);
  b &= b - 1;
  return s;
}


/// frontmost_sq() returns the most advanced square for the given color,
/// requires a non-zero bitboard.
inline Square frontmost_sq(Color c, Bitboard b) {
  assert(b);
  return c == WHITE ? msb(b) : lsb(b);
}

} // namespace Stockfish

#endif // #ifndef BITBOARD_H_INCLUDED
