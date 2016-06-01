﻿/*
 * 技巧 (Gikou), a USI shogi (Japanese chess) playing engine.
 * Copyright (C) 2016 Yosuke Demura
 * except where otherwise indicated.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "move.h"

#include <cctype>
#include "position.h"
#include "zobrist.h"

constexpr Move::Key Move::kKeyDestination;
constexpr Move::Key Move::kKeyPromotion  ;
constexpr Move::Key Move::kKeySource     ;
constexpr Move::Key Move::kKeyDrop       ;
constexpr Move::Key Move::kKeyPieceType  ;
constexpr Move::Key Move::kKeyPiece      ;
constexpr Move::Key Move::kKeyCaptured   ;
constexpr Move::Key Move::kKeyPerfectHash;
constexpr Move::Key Move::kKeyAll        ;

bool Move::IsInferior() const {
  if (is_drop()) {
    return false;
  }

  Color stm = piece().color();
  PieceType pt = piece().type();

  // 1. 歩・角・飛の不成
  if (pt == kPawn || pt == kBishop || pt == kRook) {
    if (!is_promotion()) {
      if (from().is_promotion_zone_of(stm) || to().is_promotion_zone_of(stm)) {
        return true;
      }
    }
  }

  // 2. 2段目・8段目の香の不成
  if (pt == kLance) {
    if (!is_promotion()) {
      if (relative_rank(stm, to().rank()) == kRank2) {
        return true;
      }
    }
  }

  return false;
}

uint32_t Move::PerfectHash() const {
  assert(is_quiet() && !IsInferior());
  uint32_t hash = move_;
  hash += 0x1f98d073;
  hash ^= (hash >> 16);
  hash += (hash << 8);
  hash ^= (hash >> 4);
  uint32_t index = (hash >> 8) & 0x7ff;
  uint32_t key = (hash + (hash << 2)) >> 18;
  return key ^ phash_table_[index];
}

std::string Move::ToSfen() const {
  if (*this == kMoveNone) {
    return "none";
  } else if (*this == kMoveNull) {
    return "pass";
  }
  if (is_drop()) {
    assert(piece().ToSfen().length() > 0);
    char piece_char = static_cast<char>(std::toupper(piece().ToSfen()[0]));
    return std::string{piece_char, '*'} + to().ToSfen();
  } else {
    return from().ToSfen() + to().ToSfen() + (is_promotion() ? "+" : "");
  }
}

Move Move::FromSfen(const std::string& sfen, Piece moved, Piece captured) {
  assert(sfen.length() == 4 || sfen.length() == 5);
  bool drop = sfen[1] == '*';
  if (drop) {
    Square to = Square::FromSfen(sfen.substr(2, 2));
    return Move(moved, to);
  } else {
    Square from = Square::FromSfen(sfen.substr(0, 2));
    Square to   = Square::FromSfen(sfen.substr(2, 2));
    bool promotion = (sfen.length() == 5);
    return Move(moved, from, to, promotion, captured);
  }
}

Move Move::FromSfen(const std::string& sfen, const Position& pos) {
  assert(sfen.length() == 4 || sfen.length() == 5);
  bool drop = sfen[1] == '*';
  if (drop) {
    Color c = pos.side_to_move();
    PieceType pt = Piece::FromSfen(sfen.substr(0, 1)).type();
    Square to = Square::FromSfen(sfen.substr(2, 2));
    return Move(Piece(c, pt), to);
  } else {
    Square from = Square::FromSfen(sfen.substr(0, 2));
    Square to   = Square::FromSfen(sfen.substr(2, 2));
    bool promotion = (sfen.length() == 5);
    assert(pos.piece_on(from) != kNoPiece);
    assert(pos.piece_on(from).color() == pos.side_to_move());
    return Move(pos.piece_on(from), from, to, promotion, pos.piece_on(to));
  }
}

Move& Move::Flip() {
  if (is_drop()) {
    *this = Move(piece().opponent_piece(), to().inverse_square());
  } else {
    *this = Move(piece().opponent_piece(),
                 from().inverse_square(),
                 to().inverse_square(),
                 is_promotion(),
                 is_capture() ? captured_piece().opponent_piece() : kNoPiece);
  }
  return *this;
}

bool Move::IsOk() const {
  // 1. 特殊なケース
  if (*this == kMoveNone || *this == kMoveNull) {
    return true;
  }
  // 2. 共通の検証を行う
  if (   !piece().IsOk()
      || !captured_piece().IsOk()
      || !to().IsOk()
      || piece() == kNoPiece) {
    return false;
  }
  // 3. 追加的な検証を行う
  if (is_drop()) {
    if (!piece().is_droppable())
      return false;
    if (is_capture_or_promotion())
      return false;
  } else {
    if (!from().IsOk() || from() == to())
      return false;
    if (is_promotion()) {
      if (!piece().can_promote())
        return false;
      if (   !from().is_promotion_zone_of(piece().color())
          && !to().is_promotion_zone_of(piece().color()))
        return false;
    }
    if (is_capture()) {
      if (piece().color() == captured_piece().color())
        return false;
    }
  }
  // 4. 行きどころのない駒になっていないか調べる
  if (is_drop() || !is_promotion()) {
    if (piece().is(kPawn) || piece().is(kLance)) {
      if (relative_rank(piece().color(), to().rank()) == kRank1)
        return false;
    } else if (piece().is(kKnight)) {
      if (relative_rank(piece().color(), to().rank()) <= kRank2)
        return false;
    }
  }
  return true;
}

const Array<uint16_t, 2048> Move::phash_table_ = {
    0,56,2645,14571,1712,4793,7247,5717,
    11931,5174,13128,1819,14081,9322,4972,6164,
    11332,161,9922,10505,2851,5833,8170,3268,
    3868,4128,3473,14642,7533,11397,8525,10205,
    15542,3823,3268,7708,7179,11399,14600,13296,
    12943,8170,11735,11170,10528,3098,810,4567,
    4976,15192,13413,322,6005,4127,15130,7483,
    11153,13128,15555,9648,10627,15919,10610,5083,
    6707,3754,4577,2690,4544,10017,6985,7499,
    9648,12433,14025,9867,15776,3757,7950,319,
    10072,15222,6674,1132,10307,8871,13804,1211,
    3003,11762,14830,8540,1755,12642,14330,11108,
    8011,10079,14629,10136,8851,13128,495,15861,
    788,13890,5174,431,9854,15919,6936,6522,
    6392,15129,15316,15137,2932,13019,3769,3249,
    7257,10200,12447,9429,12438,5455,6971,902,
    1712,6674,9648,3098,9867,14500,14571,9922,
    9095,0,14835,13420,4534,8246,14185,10205,
    10757,5373,3228,8170,1768,6599,15478,15413,
    9171,9557,3268,6729,11895,6193,7247,12821,
    548,1562,15113,10731,4553,15391,8412,8753,
    11189,15601,105,3043,8646,15881,10509,4572,
    65,11939,9135,7802,5475,6957,4972,13106,
    1962,5833,10214,9835,1930,996,1652,8568,
    0,13128,16121,12005,15137,0,9095,15787,
    6465,2514,2076,6392,10307,11203,10248,4916,
    3190,12845,9743,12637,12997,279,7802,1984,
    13039,7370,15563,14338,9422,7951,4363,16038,
    14148,14657,2777,5930,10307,12732,8914,12753,
    15966,6846,14928,5568,14868,12018,780,14571,
    9267,1712,8267,15216,12214,3326,12947,10079,
    2624,12954,1464,16087,9774,10079,11382,14657,
    15364,12005,10088,15413,5008,15919,788,5373,
    10136,14447,12232,13880,9095,7951,4618,2993,
    12753,3268,15826,7531,6674,1652,8274,10087,
    6522,46,6289,1930,12186,9922,7715,265,
    7951,8473,7857,14292,3219,5990,4346,1218,
    3462,11608,10307,7656,12025,431,6939,12265,
    14758,16089,5455,6971,7802,3751,10685,6453,
    9108,10674,1064,15877,14530,4124,8111,12723,
    10881,3098,431,15787,8646,15364,3757,3473,
    12176,13831,16270,1483,1987,11249,6555,12577,
    6369,5291,8175,11302,15744,1863,286,9319,
    6674,11773,1104,1358,7084,12009,8931,7476,
    9314,2739,13128,13106,10954,11584,5232,4191,
    10753,16144,8270,9143,8208,8568,661,3615,
    12663,10340,15427,12821,9967,0,11158,13083,
    981,10307,3766,8274,1455,9778,1764,4250,
    10099,9095,1712,11152,15413,7247,6729,7662,
    6599,3268,5053,8579,10079,3789,14571,10136,
    8170,9922,6674,9648,11895,9867,15919,6522,
    3098,0,1863,11584,7764,8618,10088,6674,
    9141,11203,12433,9325,4067,6599,8011,12751,
    5988,431,10528,4641,2306,6240,13663,14571,
    8055,11376,5194,2914,6645,12895,11897,2266,
    13083,4420,6130,4743,8366,127,14224,4104,
    13153,3513,14366,11581,13182,15302,4300,6573,
    9044,8013,14416,15098,13106,5506,12516,11693,
    6674,4972,6645,8260,14548,6236,12112,10205,
    15762,9314,7494,5637,14647,10099,780,359,
    9867,3460,7144,13128,10926,16205,15293,2709,
    10312,9248,10205,8293,1329,6328,3478,5000,
    13784,1225,2233,8851,7247,15940,11935,1183,
    9257,13458,725,11443,14077,8274,3789,12561,
    16016,1271,7438,9557,14385,14868,10099,4226,
    1930,11222,10034,10476,1089,10742,2490,14447,
    11597,13975,3616,3156,15048,13180,9854,6857,
    13335,13420,9421,3228,9129,5282,11201,11574,
    12296,14642,0,3098,6674,9922,14500,9095,
    6522,8170,9648,3268,13732,15399,8011,9967,
    10205,6829,9576,9691,11041,6785,6237,15228,
    4955,319,10610,6971,9867,8545,3354,3071,
    4972,8744,13128,2019,1233,5373,4538,6729,
    297,8170,9095,5853,9648,13593,4335,4556,
    14520,8348,2623,10955,2616,8502,109,5995,
    4496,2780,13717,11733,9271,11576,10779,15478,
    10205,9867,0,717,14211,15066,2228,996,
    15084,10307,13890,7275,0,14571,4972,12433,
    6164,3614,2490,7215,14642,7046,9867,1084,
    5681,4000,8875,3416,11984,4354,9481,14241,
    16346,4682,2900,1116,1652,13933,1132,3809,
    16274,2779,6474,12528,4514,8306,286,4234,
    15923,11914,3614,1215,15478,6821,7247,15413,
    8870,11007,10237,5988,2902,981,1712,10077,
    8016,2135,0,8170,6729,9922,14112,4972,
    8851,12821,14571,6674,15216,52,56,6599,
    10340,1548,8274,3098,4867,2275,3766,5222,
    3071,15427,15601,4641,5761,10205,9648,13200,
    14224,3570,15787,1018,9514,2286,8618,9557,
    12488,16329,15344,4427,2965,1349,14565,9297,
    12821,16087,12895,11332,11557,13463,10286,4218,
    3868,7865,11773,7300,116,6951,0,6814,
    12623,4995,9557,1562,10205,0,0,1712,
    4972,1652,15478,9943,497,1673,12230,15505,
    10238,905,15591,3442,8170,9075,7509,12030,
    7447,3462,8296,249,3060,14114,8687,9288,
    13417,7438,5576,4290,7274,398,10533,10757,
    10340,7720,5222,8159,6328,7951,15919,2490,
    14207,3641,8532,10509,11258,15108,12159,13838,
    10009,16079,2938,14017,14629,8646,9817,9314,
    11170,6154,908,10386,8511,514,12112,5569,
    9463,9506,8870,1712,9292,1871,11756,13671,
    14868,3268,6674,8011,6674,14571,6522,11053,
    9171,9867,6599,14642,3662,4336,10205,15413,
    3610,3580,15861,6258,10555,13781,5717,15316,
    7694,11728,16343,13128,2903,1930,6193,9967,
    8550,4745,15267,1483,4278,11201,10117,8568,
    11875,15228,1863,10136,9081,1530,10726,734,
    10099,11895,12750,14506,0,6349,10386,5527,
    1913,7600,12559,2376,13072,13665,15282,10079,
    5230,16121,7247,11864,10030,13119,2169,4877,
    10008,6729,3098,8646,0,10635,5475,6920,
    15655,6321,56,7512,14801,3476,13128,8719,
    14767,8618,11338,13200,978,4317,6774,14571,
    404,6070,3222,5772,13996,15108,7238,12308,
    15428,12608,7152,7073,1489,5574,14752,14267,
    9557,13857,3071,10079,13180,11152,12821,13732,
    6846,12112,3614,8011,7247,14642,9648,2228,
    9835,9922,6674,9867,6729,0,3268,12230,
    1712,8348,3098,9095,4112,999,8308,11332,
    5044,2228,8889,5833,14294,4167,1723,7394,
    2993,8930,12389,13618,13118,7239,6758,7662,
    16304,13010,8155,2363,4577,6543,14571,7793,
    7032,9533,1768,13377,4316,8262,5761,6487,
    15630,7472,10497,13368,13128,2738,7822,9922,
    514,6906,11488,3740,4961,14506,7469,4972,
    3513,6715,14629,0,15316,9867,0,8170,
    1132,14960,9867,6829,11561,5075,13372,12732,
    9926,6972,11821,4304,12471,5511,13128,133,
    12561,6839,1241,14380,9356,4059,2776,7111,
    9922,4538,2078,12155,567,218,4544,14887,
    8105,2355,3217,4317,2292,12052,7590,6614,
    1712,12791,8288,8851,15413,1883,6555,2381,
    15919,4415,5749,5053,4598,13655,996,10008,
    10688,6138,9506,2273,6939,10230,10778,12308,
    9753,7187,15794,719,4067,11428,4512,15020,
    3071,404,0,8024,3268,7247,0,6522,
    14500,5962,999,9648,10079,9534,10340,3437,
    902,15205,8170,345,681,11332,3447,6474,
    15919,9095,8605,3098,14969,6637,1322,11762,
    4816,405,4440,5461,8119,10504,10757,4725,
    3360,11938,14154,15283,9476,13200,9506,4594,
    9637,5230,647,1279,5415,6240,5194,697,
    11762,3228,6555,14224,15084,6193,14854,5745,
    12230,3473,6674,2470,13270,46,15902,0,
    15794,9250,16023,15966,14151,5411,14482,9648,
    6993,90,6345,6914,3268,15882,14518,13175,
    11329,276,3875,8722,7428,14647,6658,9460,
    4300,13261,14520,5411,2294,14642,3751,1712,
    7454,5676,8031,7251,15464,12338,2135,2218,
    5475,9922,8246,15063,11561,12243,14868,10380,
    2275,7321,14857,10506,14399,9297,0,12821,
    12433,3732,14571,14629,0,6674,15427,3098,
    7435,12032,10307,14035,8170,12821,9922,6522,
    0,8170,5962,4290,514,6727,319,10136,
    15919,9095,13716,6674,15641,15216,14571,14128,
    13494,7802,5240,2623,3688,15769,13298,13270,
    6885,2632,12628,11290,4445,14100,7883,7308,
    11080,2288,6594,14879,14916,13743,5738,7674,
    10610,8642,8568,9047,7570,2490,2457,7323,
    13857,0,14642,9479,202,1573,3629,8071,
    12264,11902,3400,9938,3766,4862,6485,10212,
    12319,3510,2363,12559,2591,12577,0,8675,
    3268,8469,11394,5449,1148,7275,2372,15648,
    128,8288,8170,7323,12184,13083,3990,5284,
    10383,4454,3779,1052,14981,2187,4187,15662,
    5994,13019,15635,4119,13857,13128,7662,12487,
    5615,13042,431,6599,1209,12392,2382,12753,
    14148,4573,1712,15151,9557,7420,6674,13250,
    8417,6522,5884,0,5142,6114,5397,7871,
    1132,11728,8605,15861,11895,14571,6555,9648,
    10248,359,10610,3098,4067,14624,2228,3190,
    4782,11798,5000,5953,1605,12911,1890,7409,
    2902,14960,4484,15427,12606,6270,13795,6995,
    8051,11817,470,674,1227,1527,9866,6729,
    6845,14821,14037,8484,1156,7839,13270,7520,
    8485,4769,7046,7535,13584,436,7409,11136,
    7512,1224,5525,9648,3801,2932,8011,9763,
    5475,3043,8851,0,2896,0,9648,5853,
    5934,2228,2185,4642,11146,9867,3098,9227,
    1485,3476,46,8170,3803,14936,13007,3420,
    0,10227,845,4061,4342,7409,12264,8851,
    2738,8286,10307,3268,4538,6522,2294,9922,
    1433,10485,13542,1826,13250,4784,9460,4972,
    11053,3754,14642,10778,0,15413,6555,6599,
    7247,14571,8011,11332,11152,2569,2277,1863,
    1712,3098,4972,12821,2228,14506,3422,4484,
    7857,5415,16249,6971,10088,6674,13128,5240,
    10801,16151,13190,7792,2801,13042,378,5628,
    6801,11885,15427,15919,15861,2583,2294,2327,
    10389,3210,2827,10610,1345,7023,9076,13520,
    9506,10163,6199,2135,7088,2293,9922,15089,
    12538,9634,6829,3941,6674,3766,5121,10099,
    10801,12439,15902,5008,7870,9811,5814,3201,
    7094,11875,15794,9648,9422,0,11329,2436,
    2376,1648,11128,7615,12879,6476,15527,13370,
    6490,14132,8870,4594,13015,10117,16158,13996,
    11447,7857,10405,12862,15613,13593,15267,10616,
    3757,8055,9867,13200,13374,14642,1307,14660,
    15262,5874,15259,12166,5642,6666,2793,2601,
    9352,5782,8794,14354,1500,15463,2207,11739,
    3513,9870,4428,3870,14035,2218,5723,1417,
    6456,1849,10788,12470,11806,1118,10340,13356,
    9557,2926,14500,4250,8473,5275,15794,7421,
    12751,7792,6465,11841,14571,6674,9648,7247,
    6522,7323,14571,15427,14338,11308,11806,3183,
    11152,6476,8579,6971,1652,15205,3098,6568,
    8011,6674,13743,5194,9867,2774,2277,9337,
    12821,9550,3766,13128,1712,9835,6707,8170,
    7353,1371,14793,7618,7679,1868,454,1833,
    13785,488,12262,1883,6545,13784,12334,6303,
    12799,3485,1576,141,9001,6033,14571,13412,
    12510,4124,4972,0,14868,0,1982,6674,
    13473,15668,13128,14266,8746,15843,9648,11895,
    15315,11895,8809,6971,13938,14959,12963,13042,
    12753,6408,11171,15861,11480,15563,2480,13555,
    3662,15137,11976,610,11159,5433,6952,10649,
    10386,5832,2147,13025,11671,12971,11808,341,
    8099,5717,15300,12947,8753,12487,9922,1987,
    9867,14571,1358,6729,12230,12954,12322,902,
    431,8412,9648,15822,4972,8170,10205,0,
    16298,8985,11768,4417,7512,6674,11895,9095,
    3098,13489,12230,6599,10088,9557,6846,9506,
    789,4534,14657,1764,10731,10030,9044,1712,
    11573,5411,4641,10214,8306,1224,2233,4993,
    3823,12112,10138,12104,8979,3442,15013,6009,
    15478,1278,3003,4867,902,14571,9648,15861,
    12683,3516,1512,1768,12144,5834,3419,63,
    4338,13412,13412,2734,14488,9776,1921,7363,
    2677,15759,7808,15135,12945,5558,8170,13600,
    13071,5899,4779,2470,12239,8274,13128,13815,
    3660,15192,10351,2796,2902,9449,11912,6594,
    169,1456,14311,12623,15139,3763,3662,3473,
    5318,15907,9867,9095,11931,15635,2957,8511,
    5629,14269,8871,16307,6293,2470,1507,12221,
    15359,3501,10463,14642,8296,9095,3422,15115,
    7430,15826,319,69,14571,13042,13128,14969,
    10731,15478,5935,4972,11857,12523,10087,4657,
    13467,2019,13784,5605,8170,7247,6522,0,
    3098,4727,4538,12821,10823,2993,8016,15137,
    2348,1475,529,7512,10079,1361,6186,4768,
    7857,14975,9722,12498,14193,0,5775,0,
    7954,16214,10087,6674,6009,1712,2851,8698,
    4686,1017,916,2877,9047,2307,1029,13987,
    5061,13513,3067,15479,10307,10117,13906,7656,
    13369,5779,7724,7319,8220,12793,10849,3924,
    13837,12606,4300,3098,5222,15923,4439,0,
    8112,13740,14904,14629,12482,6299,12369,11711,
    8420,9011,9599,3062,12292,7673,1218,6599,
    6522,9835,14642,3766,15794,15413,1712,9721,
    8222,10998,11311,2623,4395,7767,16060,15656,
    14998,11469,7818,7511,15994,4188,12919,4661,
    8170,15267,13128,3193,8851,9867,0,9922,
    7819,4867,9065,5174,11100,4972,6555,12910,
    15635,4399,1224,8985,14571,9648,8011,7247,
    6838,3609,11906,10079,8170,11152,9557,2905,
};
