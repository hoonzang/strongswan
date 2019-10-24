/*
 * Copyright (C) 2013-2014 Andreas Steffen
 * HSR Hochschule fuer Technik Rapperswil
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#include "test_suite.h"

#include <utils/test.h>
#include <crypto/xofs/xof.h>
#include <crypto/drbgs/drbg.h>
#include <crypto/rngs/rng_tester.h>
#include <plugins/ntru/ntru_trits.h>
#include <plugins/ntru/ntru_poly.h>
#include <plugins/ntru/ntru_param_set.h>
#include <plugins/ntru/ntru_private_key.h>

IMPORT_FUNCTION_FOR_TESTS(ntru, ntru_trits_create, ntru_trits_t*,
						  size_t len, ext_out_function_t alg, chunk_t seed)

IMPORT_FUNCTION_FOR_TESTS(ntru, ntru_poly_create_from_seed, ntru_poly_t*,
						  ext_out_function_t alg, chunk_t seed, uint8_t c_bits,
						  uint16_t N, uint16_t q, uint32_t indices_len_p,
						  uint32_t indices_len_m, bool is_product_form)

IMPORT_FUNCTION_FOR_TESTS(ntru, ntru_poly_create_from_data, ntru_poly_t*,
						  uint16_t *data, uint16_t N, uint16_t q,
						  uint32_t indices_len_p, uint32_t indices_len_m,
						  bool is_product_form)

IMPORT_FUNCTION_FOR_TESTS(ntru, ntru_param_set_get_by_id,  ntru_param_set_t* ,
						  ntru_param_set_id_t id)

IMPORT_FUNCTION_FOR_TESTS(ntru, ntru_private_key_create, ntru_private_key_t*,
						  drbg_t *drbg, ntru_param_set_t *params)

IMPORT_FUNCTION_FOR_TESTS(ntru, ntru_private_key_create_from_data, ntru_private_key_t*,
						  drbg_t *drbg, chunk_t data)

IMPORT_FUNCTION_FOR_TESTS(ntru, ntru_public_key_create_from_data, ntru_public_key_t*,
						  drbg_t *drbg, chunk_t data)

/**
 * NTRU parameter sets to test
 */
static struct {
	key_exchange_method_t ke;
	char *name;
} params[] = {
	{ NTRU_112_BIT, "NTRU_112" },
	{ NTRU_128_BIT, "NTRU_128" },
	{ NTRU_192_BIT, "NTRU_192" },
	{ NTRU_256_BIT, "NTRU_256" }
};

/**
 * NTRU parameter set selection
 */
char *parameter_sets[] = {
		"x9_98_speed", "x9_98_bandwidth", "x9_98_balance", "optimum"
};

typedef struct {
	uint8_t c_bits;
	uint16_t N;
	uint16_t q;
	bool is_product_form;
	uint32_t indices_len;
	uint32_t indices_size;
	uint16_t *indices;
} poly_test_t;

typedef struct {
	ext_out_function_t alg;
	size_t hash_size;
	size_t seed_len;
	chunk_t seed;
	chunk_t trits;
	poly_test_t poly_test[2];
} trits_test_t;

uint16_t indices_ees439ep1[] = {
	367, 413,  16, 214, 114, 128,  42, 268, 346, 329, 119, 303, 208, 287, 150,
	  3,  45, 321, 110, 109, 272, 430,  80, 305,  51, 381, 322, 140, 207, 315,
	206, 186,  56,   5, 273, 177,  44, 100, 205, 210,  98, 191,   8, 336
};

uint16_t indices_ees613ep1[] = {
	245, 391, 251, 428, 301,   2, 176, 296, 461, 224, 590, 215, 250,  91, 395,
	363,  58, 537, 278, 291, 247,  33, 140, 447, 172, 514, 424, 412,  95,  94,
	281, 159, 196, 302, 277,  63, 404, 150, 608, 315, 195, 334, 207, 376, 398,
	  0, 309, 486, 516,  86, 267, 139, 130,  38, 141, 258,  21, 341, 526, 388,
	194, 116, 138, 524, 547, 383, 542, 406, 270, 438, 240, 445, 527, 168, 320,
	186, 327, 212, 543,  82, 606, 131, 294, 392, 477, 430, 583, 142, 253, 434,
	134, 458, 559, 414, 162, 407, 580, 577, 191, 109, 554, 523,  32,  62, 297,
	283, 268,  54, 539,   5
};

uint16_t indices_ees743ep1[] = {
	285,  62, 136, 655, 460,  35, 450, 208, 340, 212,  61, 234, 454,  52, 520,
	399, 315, 616, 496,  88, 280, 543, 508, 237, 553,  39, 214, 253, 720, 291,
	586, 615, 635, 596,  62, 499, 301, 176, 271, 659, 372, 185, 621, 350, 683,
	180, 717, 509, 641, 738, 666, 171, 639, 606, 353, 706, 237, 358, 410, 423,
	197, 501, 261, 654, 658, 701, 377, 182, 548, 287, 700, 403, 248, 137
};

uint16_t indices_ees1171ep1[] = {
	514, 702, 760, 505, 262, 486, 695, 783, 533,  74, 403, 847, 170,1019, 568,
	676,1057, 277,1021, 238, 203, 884, 124,  87,  65,  93, 131, 881,1102, 133,
	459, 462,  92,  40,   5,1152,1158, 297, 599, 299,   7, 458, 347, 343, 173,
   1044, 264, 871, 819, 679, 328, 438, 990, 982, 308,1135, 423, 470, 254, 295,
   1029, 892, 759, 789, 123, 939, 749, 353,1062, 145, 562, 337, 550, 102, 549,
	821,1098, 823,  96, 365, 135,1110, 334, 391, 638, 963, 962,1002,1069, 993,
	983, 649,1056, 399, 385, 715, 582, 799, 161, 512, 629, 979, 250,  37, 213,
	929, 413, 566, 336, 727, 160, 616,1170, 748, 282,1115, 325, 994, 189, 500,
	913, 332,1118, 753, 946, 775,  59, 809, 782, 612, 909,1090, 223, 777, 940,
	866,1032, 471, 298, 969, 192, 411, 721, 476, 910,1045,1027, 812, 352, 487,
	215, 625, 808, 230, 602, 457, 900, 416, 985, 850, 908, 155, 670, 669,1054,
	400,1126, 733, 647, 786, 195, 148, 362,1094, 389,1086,1166, 231, 436, 210,
	333, 824, 785, 826, 658, 472, 639,1046,1028, 519, 422,  80, 924,1089, 547,
   1157, 579,   2, 508,1040, 998, 902,1058, 600, 220, 805, 945, 140,1117, 179,
	536, 191
};

/**
 * Trits and Polynomial Test Vectors
 */
static trits_test_t trits_tests[] = {
	{	XOF_MGF1_SHA1, 20, 24,
		chunk_from_chars(
						0xED, 0xA5, 0xC3, 0xBC, 0xAF, 0xB3, 0x20, 0x7D,
						0x14, 0xA1, 0x54, 0xF7, 0x8B, 0x37, 0xF2, 0x8D,
						0x8C, 0x9B, 0xD5, 0x63, 0x57, 0x38, 0x11, 0xC2,
						0xB5, 0xCA, 0xBF, 0x06, 0x43, 0x45, 0x19, 0xD5,
						0xE7, 0x36, 0xD0, 0x29, 0x21, 0xDA, 0x02, 0x20,
						0x45, 0xF6, 0x5F, 0x0F, 0x10, 0x04, 0x2A, 0xE3,
						0x6A, 0x1D, 0xD5, 0x9F, 0x1D, 0x66, 0x44, 0x8F,
						0xFA, 0xC6, 0xCA, 0xA4, 0x6E, 0x3B, 0x00, 0x66,
						0xA6, 0xC9, 0x80, 0x5C, 0xF5, 0x2D, 0xD7, 0x72,
						0xC6, 0xD4, 0x4F, 0x30, 0x72, 0xA2, 0xAD, 0xE0,
						0x33, 0xE8, 0x55, 0xD5, 0xE6, 0xD6, 0x00, 0x1D,
						0xA8, 0x68, 0xFF, 0x97, 0x36, 0x8A, 0xF4, 0xD6,
						0xF1, 0xB6, 0x7E, 0x1F, 0x06, 0xCB, 0x57, 0xCB,
						0x35, 0x38, 0xF2, 0x2D, 0xF6, 0x20),
		chunk_from_chars(
				1, 2, 1, 0, 0,  1, 1, 1, 2, 0,  1, 0, 1, 1, 1,  0, 2, 0, 1, 1,
				0, 0, 0, 1, 1,  0, 2, 0, 2, 2,	1, 2, 2, 2, 1,  2, 1, 1, 0, 0,
				2, 0, 1, 1, 1,	0, 0, 0, 0, 1,  1, 2, 0, 0, 1,  0, 1, 0, 2, 0,
				0, 1, 0, 2, 1,  0, 0, 0, 2, 0,  0, 0, 1, 2, 2,	0, 0, 2, 0, 1,
				1, 2, 1, 1, 0,  0, 1, 1, 1, 2,	2, 1, 2, 0, 0,  2, 1, 0, 0, 1,
				0, 1, 1, 0, 0,	0, 1, 2, 2, 0,  1, 2, 1, 2, 0,  2, 0, 0, 0, 2,
				1, 2, 0, 0, 0,  2, 0, 0, 0, 2,  2, 1, 0, 2, 0,	1, 2, 0, 2, 1,
				0, 2, 2, 1, 0,  2, 1, 2, 2, 0,  2, 0, 2, 1, 2,  2, 0, 2, 0, 1,
				1, 2, 2, 2, 2,  1, 0, 1, 0, 2,  2, 0, 1, 1, 2,  2, 2, 0, 0, 1,
				0, 2, 0, 1, 0,  2, 1, 2, 1, 0,  1, 1, 2, 0, 0,  2, 1, 1, 2, 0,
				1, 2, 1, 1, 0,  1, 0, 2, 1, 1,  1, 2, 1, 0, 2,  0, 2, 0, 0, 2,
				2, 1, 0, 0, 2,  2, 0, 1, 1, 0,  0, 1, 1, 0, 1,  1, 2, 1, 2, 2,
				2, 0, 0, 0, 0,  1, 0, 0, 1, 2,  1, 2, 0, 2, 1,  1, 1, 0, 2, 2,
				1, 2, 2, 1, 0,  1, 0, 2, 2, 2,  1, 2, 1, 0, 0,  1, 0, 1, 1, 1,
				1, 1, 2, 0, 0,  2, 1, 0, 2, 1,  2, 1, 0, 2, 2,  0, 0, 1, 2, 1,
				2, 0, 1, 2, 1,  1, 2, 0, 2, 0,  2, 1, 1, 1, 0,  0, 0, 1, 2, 1,
				2, 2, 1, 2, 1,  1, 2, 1, 2, 0,  2, 2, 1, 0, 0,  1, 2, 0, 1, 1,
				2, 0, 0, 0, 1,  2, 2, 1, 2, 0,  0, 2, 1, 0, 2,  2, 2, 1, 1, 0,
				2, 1, 2, 1, 2,  2, 1, 2, 1, 1,  0, 1, 1, 1, 1,  2, 0, 2, 2, 1,
				0, 1, 1, 2, 1,  2, 0, 2, 1, 0,  1, 0, 1, 0, 1,  2, 0, 1, 1, 0,
				0, 1, 1, 2, 0,  2, 2, 0, 0, 0,  1, 1, 0, 1, 0,  1, 1, 0, 1, 1,
				0, 1, 2, 0, 1,  1, 0, 1, 2, 0,  0, 1, 2, 2, 0,  0, 2, 1, 2),
		{
			{	9, 439, 2048, TRUE, 9 + (8 << 8) + (5 << 16),
				countof(indices_ees439ep1), indices_ees439ep1
			},
			{	11, 613, 2048, FALSE, 55,
				countof(indices_ees613ep1), indices_ees613ep1
			}
		}
	},
	{	XOF_MGF1_SHA256, 32, 40,
		chunk_from_chars(
						0x52, 0xC5, 0xDD, 0x1E, 0xEF, 0x76, 0x1B, 0x53,
						0x08, 0xE4, 0x86, 0x3F, 0x91, 0x12, 0x98, 0x69,
						0xC5, 0x9D, 0xDE, 0xF6, 0xFC, 0xFA, 0x93, 0xCE,
						0x32, 0x52, 0x66, 0xF9, 0xC9, 0x97, 0xF6, 0x42,
						0x00, 0x2C, 0x64, 0xED, 0x1A, 0x6B, 0x14, 0x0A,
						0x4B, 0x04, 0xCF, 0x6D, 0x2D, 0x82, 0x0A, 0x07,
						0xA2, 0x3B, 0xDE, 0xCE, 0x19, 0x8A, 0x39, 0x43,
						0x16, 0x61, 0x29, 0x98, 0x68, 0xEA, 0xE5, 0xCC,
						0x0A, 0xF8, 0xE9, 0x71, 0x26, 0xF1, 0x07, 0x36,
						0x2C, 0x07, 0x1E, 0xEB, 0xE4, 0x28, 0xA2, 0xF4,
						0xA8, 0x12, 0xC0, 0xC8, 0x20, 0x37, 0xF8, 0xF2,
						0x6C, 0xAF, 0xDC, 0x6F, 0x2E, 0xD0, 0x62, 0x58,
						0xD2, 0x37, 0x03, 0x6D, 0xFA, 0x6E, 0x1A, 0xAC,
						0x9F, 0xCA, 0x56, 0xC6, 0xA4, 0x52, 0x41, 0xE8,
						0x0F, 0x1B, 0x0C, 0xB9, 0xE6, 0xBA, 0xDE, 0xE1,
						0x03, 0x5E, 0xC2, 0xE5, 0xF8, 0xF4, 0xF3, 0x46,
						0x3A, 0x12, 0xC0, 0x1F, 0x3A, 0x00, 0xD0, 0x91,
						0x18, 0xDD, 0x53, 0xE4, 0x22, 0xF5, 0x26, 0xA4,
						0x54, 0xEE, 0x20, 0xF0, 0x80),
		chunk_from_chars(
				1, 2, 2, 2, 2,  1, 2, 2, 0, 0,  2, 0, 0, 0, 0,  1, 2, 2, 2, 0,
				2, 0, 0, 2, 2,  1, 2, 0, 0, 1,  2, 1, 0, 0, 0,  1, 0, 2, 2, 1,
				1, 2, 0, 0, 0,  1, 2, 0, 2, 2,  1, 2, 1, 0, 1,  0, 1, 2, 1, 1,
				1, 2, 0, 1, 0,  2, 1, 1, 0, 0,  0, 1, 2, 0, 0,  1, 2, 1, 2, 0,
				2, 1, 1, 1, 2,  2, 2, 2, 1, 0,  0, 2, 0, 2, 0,  1, 1, 0, 2, 2,
				2, 0, 1, 0, 2,  2, 1, 0, 1, 0,  1, 0, 0, 2, 2,  0, 0, 1, 2, 0,
				1, 1, 1, 0, 0,  2, 0, 2, 1, 2,  2, 2, 0, 0, 2,  1, 0, 2, 0, 1,
				0, 1, 2, 0, 1,  2, 0, 1, 0, 1,  2, 0, 2, 2, 0,  1, 2, 2, 1, 2,
				2, 2, 0, 2, 1,  1, 1, 0, 0, 1,  0, 2, 0, 0, 1,  0, 1, 2, 0, 0,
				1, 2, 1, 0, 2,  1, 1, 0, 0, 2,  1, 2, 2, 2, 1,  2, 1, 1, 2, 2,
				0, 2, 0, 0, 2,  0, 0, 1, 1, 2,  0, 0, 0, 1, 2,  1, 1, 1, 1, 0,
				0, 0, 2, 0, 2,  0, 2, 2, 1, 2,  2, 0, 0, 1, 1,  1, 0, 1, 0, 1,
				0, 1, 2, 2, 0,  2, 1, 1, 0, 2,  1, 2, 1, 2, 1,  0, 0, 1, 0, 0,
				1, 0, 1, 0, 2,  0, 2, 0, 0, 1,  2, 0, 2, 0, 1,  1, 0, 2, 0, 0,
				1, 2, 1, 2, 1,  2, 1, 0, 1, 1,  2, 2, 1, 1, 0,  0, 2, 1, 2, 0,
				1, 0, 2, 0, 0,  1, 2, 0, 2, 0,  1, 1, 2, 2, 2,  2, 0, 0, 1, 2,
				1, 1, 1, 0, 2,  1, 2, 2, 0, 2,  0, 1, 2, 2, 0,  1, 1, 1, 0, 0,
				2, 0, 1, 0, 1,  0, 2, 1, 2, 0,  2, 1, 2, 1, 2,  2, 0, 2, 1, 0,
				2, 1, 2, 0, 0,  2, 0, 1, 2, 1,  1, 2, 0, 0, 0,  0, 1, 2, 0, 1,
				2, 2, 1, 0, 0,  1, 2, 1, 2, 0,  0, 1, 1, 0, 0,  0, 1, 0, 0, 0,
				2, 0, 1, 2, 1,  2, 0, 0, 0, 2,  1, 0, 0, 0, 1,  2, 2, 0, 0, 0,
				2, 2, 1, 1, 0,  1, 0, 2, 2, 0,  2, 1, 2, 1, 0,  2, 2, 2, 0, 0,
				0, 1, 1, 2, 1,  0, 0, 0, 0, 1,  2, 2, 1, 2, 1,  2, 0, 2, 0, 2,
				1, 1, 1, 2, 1,  2, 1, 2, 1, 1,  0, 1, 0, 2, 0,  0, 0, 2, 1, 2,
				2, 2, 2, 0, 1,  1, 1, 0, 1, 0,  2, 0, 2, 1, 0,  1, 2, 1, 1, 0,
				1, 2, 1, 0, 0,  2, 1, 0, 1, 1,  2, 2, 1, 1, 1,  2, 2, 2, 1, 0,
				0, 0, 0, 1, 1,  0, 0, 2, 2, 2,  2, 2, 0, 1, 2,  0, 1, 2, 0, 1,
				1, 0, 1, 1, 2,  2, 0, 1, 1, 0,  2, 2, 1, 1, 1,  2, 1, 2, 2, 1,
				1, 0, 1, 0, 2,  2, 1, 0, 2, 2,  2, 2, 2, 1, 0,  2, 2, 2, 1, 2,
				0, 2, 0, 0, 0,  0, 0, 1, 2, 0,  1, 0, 1),
		{
			{	13, 743, 2048, TRUE, 11 + (11 << 8) + (15 << 16),
				countof(indices_ees743ep1), indices_ees743ep1
			},
			{	12, 1171, 2048, FALSE, 106,
				countof(indices_ees1171ep1), indices_ees1171ep1
			}
		}
	}
};

START_TEST(test_ntru_trits)
{
	ntru_trits_t *mask;
	chunk_t trits;

	mask = TEST_FUNCTION(ntru, ntru_trits_create, trits_tests[_i].trits.len,
						 XOF_UNDEFINED, trits_tests[_i].seed);
	ck_assert(mask == NULL);

	mask = TEST_FUNCTION(ntru, ntru_trits_create, trits_tests[_i].trits.len,
						 trits_tests[_i].alg, chunk_empty);
	ck_assert(mask == NULL);

	mask = TEST_FUNCTION(ntru, ntru_trits_create, trits_tests[_i].trits.len,
						 trits_tests[_i].alg, trits_tests[_i].seed);
	ck_assert(mask);

	trits = chunk_create(mask->get_trits(mask), mask->get_size(mask));
	ck_assert(chunk_equals(trits, trits_tests[_i].trits));
	mask->destroy(mask);

	/* generate a multiple of 5 trits */
	mask = TEST_FUNCTION(ntru, ntru_trits_create, 10, trits_tests[_i].alg,
						 trits_tests[_i].seed);
	ck_assert(mask);

	trits = chunk_create(mask->get_trits(mask), mask->get_size(mask));
	ck_assert(chunk_equals(trits, chunk_create(trits_tests[_i].trits.ptr, 10)));
	mask->destroy(mask);
}
END_TEST

START_TEST(test_ntru_poly)
{
	ntru_poly_t *poly;
	uint16_t *indices;
	chunk_t seed;
	poly_test_t *p;
	int j, n;

	seed = trits_tests[_i].seed;
	seed.len = trits_tests[_i].seed_len;

	p = &trits_tests[_i].poly_test[0];
	poly = TEST_FUNCTION(ntru, ntru_poly_create_from_seed, XOF_UNDEFINED, seed,
						 p->c_bits, p->N, p->q, p->indices_len, p->indices_len,
						 p->is_product_form);
	ck_assert(poly == NULL);

	for (n = 0; n < 2; n++)
	{
		p = &trits_tests[_i].poly_test[n];
		poly = TEST_FUNCTION(ntru, ntru_poly_create_from_seed,
							trits_tests[_i].alg, seed, p->c_bits, p->N, p->q,
							p->indices_len, p->indices_len, p->is_product_form);
		ck_assert(poly != NULL && poly->get_size(poly) == p->indices_size);

		indices = poly->get_indices(poly);
		for (j = 0; j < p->indices_size; j++)
		{
			ck_assert(indices[j] == p->indices[j]);
		}
		poly->destroy(poly);
	}
}
END_TEST

typedef struct {
	uint16_t N;
	uint16_t q;
	bool is_product_form;
	uint32_t indices_len_p;
	uint32_t indices_len_m;
	uint16_t *indices;
	uint16_t *a;
	uint16_t *c;
} ring_mult_test_t;

uint16_t t1_indices[] = { 1, 6, 5, 3 };

uint16_t t1_a[] = { 1, 0, 0, 0, 0, 0, 0 };
uint16_t t1_c[] = { 0, 1, 0, 7, 0, 7, 1 };

uint16_t t2_a[] = { 5, 0, 0, 0, 0, 0, 0 };
uint16_t t2_c[] = { 0, 5, 0, 3, 0, 3, 5 };

uint16_t t3_a[]  = { 4, 0, 0, 0, 0, 0, 0 };
uint16_t t3_c[]  = { 0, 4, 0, 4, 0, 4, 4 };

uint16_t t4_a[]  = { 0, 6, 0, 0, 0, 0, 0 };
uint16_t t4_c[]  = { 6, 0, 6, 0, 2, 0, 2 };

uint16_t t5_a[]  = { 4, 6, 0, 0, 0, 0, 0 };
uint16_t t5_c[]  = { 6, 4, 6, 4, 2, 4, 6 };

uint16_t t6_a[]  = { 0, 0, 3, 0, 0, 0, 0 };
uint16_t t6_c[]  = { 5, 3, 0, 3, 0, 5, 0 };

uint16_t t7_a[]  = { 4, 6, 3, 0, 0, 0, 0 };
uint16_t t7_c[]  = { 3, 7, 6, 7, 2, 1, 6 };

uint16_t t8_a[]  = { 0, 0, 0, 7, 0, 0, 0 };
uint16_t t8_c[]  = { 0, 1, 7, 0, 7, 0, 1 };

uint16_t t9_a[]  = { 4, 6, 3, 7, 0, 0, 0 };
uint16_t t9_c[]  = { 3, 0, 5, 7, 1, 1, 7 };

uint16_t t10_a[] = { 0, 0, 0, 0, 0, 1, 0 };
uint16_t t10_c[] = { 0, 7, 0, 7, 1, 0, 1 };

uint16_t t11_a[] = { 4, 6, 3, 7, 0, 1, 0 };
uint16_t t11_c[] = { 3, 7, 5, 6, 2, 1, 0 };

uint16_t t2_indices[] = { 1, 6, 5, 2, 3 };

uint16_t t12_c[] = { 0, 1, 7, 7, 0, 1, 1 };
uint16_t t13_c[] = { 0, 1, 7, 7, 0, 7, 1 };
uint16_t t14_c[] = { 0, 1, 0, 31, 0, 31, 1 };
uint16_t t15_c[] = { 0, 5, 0, 2043, 0, 2043, 5 };
uint16_t t16_c[] = { 0, 5, 0, 32763, 0, 32763, 5 };

uint16_t t3_indices[] = { 7, 2, 3, 5, 0, 2, 3, 10, 7, 0, 8, 2 };

uint16_t t17_a[] = { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
uint16_t t17_c[] = { 7, 1, 0, 1, 1, 7, 0, 7, 7, 7, 2 };

ring_mult_test_t ring_mult_tests[] = {
	{  7,     8, FALSE, 2, 2, t1_indices, t1_a,  t1_c  },
	{  7,     8, FALSE, 2, 2, t1_indices, t2_a,  t2_c  },
	{  7,     8, FALSE, 2, 2, t1_indices, t3_a,  t3_c  },
	{  7,     8, FALSE, 2, 2, t1_indices, t4_a,  t4_c  },
	{  7,     8, FALSE, 2, 2, t1_indices, t5_a,  t5_c  },
	{  7,     8, FALSE, 2, 2, t1_indices, t6_a,  t6_c  },
	{  7,     8, FALSE, 2, 2, t1_indices, t7_a,  t7_c  },
	{  7,     8, FALSE, 2, 2, t1_indices, t8_a,  t8_c  },
	{  7,     8, FALSE, 2, 2, t1_indices, t9_a,  t9_c  },
	{  7,     8, FALSE, 2, 2, t1_indices, t10_a, t10_c },
	{  7,     8, FALSE, 2, 2, t1_indices, t11_a, t11_c },
	{  7,     8, FALSE, 3, 2, t2_indices, t1_a,  t12_c },
	{  7,     8, FALSE, 2, 3, t2_indices, t1_a,  t13_c },
	{  7,    32, FALSE, 2, 2, t1_indices, t1_a,  t14_c },
	{  7,  2048, FALSE, 2, 2, t1_indices, t2_a,  t15_c },
	{  7, 32768, FALSE, 2, 2, t1_indices, t2_a,  t16_c },
	{ 11,     8, TRUE, 197121, 197121, t3_indices, t17_a,  t17_c },
};

START_TEST(test_ntru_ring_mult)
{
	ntru_poly_t *poly;
	ring_mult_test_t *t;
	uint16_t *c;
	int i;

	t = &ring_mult_tests[_i];
	poly = TEST_FUNCTION(ntru, ntru_poly_create_from_data, t->indices, t->N,
						 t->q, t->indices_len_p, t->indices_len_m,
						 t->is_product_form);
	ck_assert(poly != NULL);

	c = malloc(t->N * sizeof(uint16_t));
	poly->ring_mult(poly, t->a, c);

	for (i = 0; i < t->N; i++)
	{
		ck_assert(c[i] == t->c[i]);
	}

	free(c);
	poly->destroy(poly);
}
END_TEST

int array_tests[] = { 0, 11, 12, 16 };

START_TEST(test_ntru_array)
{
	ntru_poly_t *poly;
	ring_mult_test_t *t;
	uint16_t *c;
	int i;

	t = &ring_mult_tests[array_tests[_i]];

	poly = TEST_FUNCTION(ntru, ntru_poly_create_from_data, t->indices, t->N,
						 t->q, t->indices_len_p, t->indices_len_m,
						 t->is_product_form);
	ck_assert(poly != NULL);

	c = malloc(t->N * sizeof(uint16_t));
	poly->get_array(poly, c);

	for (i = 0; i < t->N; i++)
	{
		ck_assert(c[i] == t->c[i]);
	}

	free(c);
	poly->destroy(poly);
}
END_TEST

START_TEST(test_ntru_param_set)
{
	ck_assert(TEST_FUNCTION(ntru, ntru_param_set_get_by_id, -1) == NULL);
	ck_assert(TEST_FUNCTION(ntru, ntru_param_set_get_by_id, 16) == NULL);
}
END_TEST

typedef struct {
	ntru_param_set_id_t id;
	chunk_t entropy;
	chunk_t encoding;
} privkey_test_t;

privkey_test_t privkey_tests[] = {
	{
		NTRU_EES401EP1,
		chunk_from_chars(
						0x0C, 0x2F, 0x24, 0xE1, 0xA4, 0x81, 0x26, 0xA2,
						0x6C, 0xEA, 0xCD, 0x1A, 0xF3, 0xEB, 0x3D, 0xBF,
						0xEA, 0xAE, 0xC3, 0x0D, 0xC1),
		chunk_from_chars(
						0x02, 0x03, 0x00, 0x02, 0x04, 0x3E, 0xF3, 0xCB,
						0x7A, 0x58, 0x13, 0x75, 0xBB, 0x87, 0xF5, 0xBF,
						0x2E, 0x18, 0xAE, 0x03, 0xAF, 0xB8, 0x33, 0x85,
						0xD8, 0xBF, 0x8A, 0xB5, 0x8C, 0xA6, 0xDF, 0x03,
						0x90, 0x1E, 0xE4, 0x83, 0xA4, 0x95, 0x40, 0xB5,
						0x08, 0x92, 0x29, 0xD8, 0x83, 0xA8, 0x42, 0xB2,
						0x69, 0xC2, 0x00, 0x8B, 0xAE, 0x80, 0x00, 0x4F,
						0x3D, 0xDD, 0xFB, 0xDB, 0x9A, 0xD8, 0x0F, 0xFF,
						0xBC, 0x21, 0xD5, 0xE6, 0x04, 0x9C, 0xDD, 0x3B,
						0x2D, 0x16, 0x4B, 0xC7, 0x3D, 0xBE, 0xDE, 0xBB,
						0x6F, 0xF4, 0x8A, 0x31, 0xCD, 0x23, 0x19, 0xC2,
						0x3C, 0xE1, 0xE2, 0xEE, 0xE4, 0xE7, 0x2E, 0xFC,
						0x5C, 0xDD, 0xAD, 0x0C, 0x9D, 0x98, 0xC5, 0x18,
						0x2A, 0x80, 0x21, 0x93, 0x61, 0xC4, 0x9A, 0x16,
						0xE8, 0x9B, 0xF7, 0x3B, 0x6D, 0x06, 0x91, 0x9E,
						0x71, 0x59, 0xBE, 0x8E, 0x65, 0x61, 0xB2, 0x69,
						0x9C, 0x82, 0x58, 0x0D, 0x63, 0x7A, 0x1F, 0x2A,
						0x1C, 0x2C, 0x92, 0x8C, 0x8D, 0xCA, 0x2B, 0x45,
						0x24, 0x79, 0xDB, 0x7F, 0x1D, 0x2F, 0xAB, 0x88,
						0x8C, 0x1D, 0xE3, 0x15, 0x8F, 0xCD, 0x46, 0x8C,
						0x45, 0x20, 0x88, 0x1C, 0x17, 0xE0, 0xE5, 0x89,
						0xF4, 0x60, 0x56, 0x3C, 0x6B, 0x9F, 0x2A, 0xD9,
						0xD0, 0xAE, 0x3B, 0xB6, 0xC2, 0xB7, 0x58, 0xC6,
						0x6E, 0x09, 0x36, 0x21, 0x0B, 0xDD, 0xE9, 0x52,
						0x33, 0x27, 0x39, 0xC8, 0x51, 0x59, 0x69, 0x25,
						0xC6, 0x3D, 0x19, 0x5C, 0x5E, 0x74, 0xD0, 0x62,
						0xD9, 0x26, 0x90, 0xC7, 0x64, 0x92, 0xA8, 0x72,
						0xD1, 0x77, 0x1F, 0x78, 0xC5, 0x11, 0xBD, 0x5D,
						0x3C, 0x1B, 0x1F, 0x8B, 0x5B, 0xE4, 0x5D, 0xA1,
						0x27, 0x6D, 0x20, 0x24, 0x32, 0x53, 0xF3, 0xB0,
						0xE6, 0x71, 0x61, 0xCC, 0xFC, 0x4A, 0x06, 0xDA,
						0xBE, 0xD7, 0x9F, 0x2F, 0xEB, 0x44, 0xD0, 0x8A,
						0x7D, 0x8E, 0x82, 0xF5, 0x84, 0xCF, 0x8E, 0xE5,
						0x4B, 0xA4, 0x30, 0x77, 0xBD, 0x14, 0xB9, 0x75,
						0x02, 0x68, 0xDF, 0x71, 0x89, 0x81, 0xF2, 0x95,
						0xC3, 0x67, 0x6E, 0x37, 0xE4, 0xD0, 0xC9, 0x1E,
						0x02, 0xDE, 0x2D, 0x79, 0x99, 0xE8, 0x7D, 0x5C,
						0x99, 0xF2, 0x1A, 0xDE, 0x12, 0x9B, 0xD1, 0x83,
						0x9B, 0x01, 0xD3, 0xEB, 0x2B, 0x8E, 0x9C, 0xA5,
						0x19, 0xE8, 0x2E, 0xFE, 0x23, 0x6E, 0xAD, 0x8F,
						0x3C, 0xAF, 0xB9, 0xE6, 0xDB, 0x07, 0xA4, 0x31,
						0x02, 0x2B, 0x6A, 0xA0, 0xFB, 0x51, 0x6C, 0xD0,
						0x26, 0xD5, 0xAD, 0x29, 0x65, 0x10, 0xCE, 0xF8,
						0x84, 0x4D, 0x1E, 0x37, 0x92, 0xA2, 0xD1, 0xFA,
						0xF6, 0xC0, 0x36, 0x4C, 0x23, 0x3A, 0x42, 0xAA,
						0xB8, 0x0D, 0x4E, 0xD4, 0x40, 0x61, 0xD5, 0x36,
						0x62, 0x23, 0x7C, 0x1C, 0x5E, 0xEA, 0x16, 0xAD,
						0x4F, 0x30, 0xF9, 0x16, 0x99, 0xCE, 0xC5, 0x50,
						0xAC, 0x8F, 0x6F, 0x98, 0xD7, 0xE3, 0x89, 0x6E,
						0x3A, 0x12, 0xCE, 0xA7, 0xA4, 0x17, 0x74, 0xDC,
						0xDB, 0xFA, 0xFF, 0xF9, 0x35, 0xD7, 0xF5, 0x77,
						0x03, 0xF5, 0xBF, 0x81, 0x6C, 0x9F, 0x62, 0xA6,
						0x8A, 0x5B, 0xA3, 0xEF, 0x9D, 0xC3, 0xF6, 0x3A,
						0x6A, 0xC0, 0x42, 0x71, 0xAF, 0x90, 0xCA, 0x1D,
						0x86, 0x78, 0xD7, 0x2C, 0xFE, 0xB6, 0x99, 0x15,
						0x8C, 0x10, 0x42, 0x92, 0x2C, 0x05, 0x43, 0x92,
						0x69, 0x05, 0x8D, 0x9E, 0xBC, 0xAB, 0x8F, 0x28,
						0xAA, 0x4B, 0xFB, 0x25, 0xD9, 0xAD, 0x29, 0xFF,
						0x33, 0x65, 0x14, 0xC3, 0x75, 0x1F, 0xCF, 0xFC,
						0x20, 0x83, 0xBF, 0xB9, 0xA5, 0x4B, 0x7B, 0xD9,
						0x07, 0x5C, 0xA1, 0xD1, 0x5A, 0x3E, 0x94, 0xF8,
						0x03, 0xDE, 0xB8, 0x94, 0x11, 0x92, 0x80, 0x77,
						0x57, 0x45, 0x1E, 0x6B, 0xA5, 0x15, 0xDB, 0x48,
						0xB6, 0x9E, 0x02, 0xF1, 0x61, 0x4A, 0xAC, 0x1D,
						0x49, 0xBC, 0xA9, 0x3F, 0x03, 0x50, 0xAC, 0x02,
						0x8E, 0x84, 0xE0, 0x12, 0x37, 0x76, 0xBC, 0x4A,
						0xF9, 0xC6, 0x74, 0x36, 0xFC, 0x92, 0x1D, 0x59,
						0x0C, 0x04, 0xD2, 0x14, 0xB7, 0x11, 0xE9, 0xE2,
						0xFE, 0x0C, 0xE1, 0xDA, 0x8B, 0xCA, 0x10, 0xA1,
						0x60, 0xB6, 0x57, 0x51, 0x00, 0xD6, 0x5B, 0x55,
						0x09, 0x60, 0xE8, 0x00, 0x40, 0x45, 0x56, 0xBA,
						0x83, 0x1E, 0x36, 0x12, 0x59, 0x4B, 0x19, 0x00,
						0x53, 0xAE, 0x62, 0xA6, 0x29, 0x39, 0xED, 0x87,
						0x24, 0x37, 0x1E, 0x1B, 0xCF, 0x3F, 0x3A, 0x71,
						0x31, 0xB5, 0x50, 0x8D, 0x4B, 0x53, 0x53, 0x75,
						0x3F, 0x33, 0x39, 0x09, 0x2A, 0x78, 0xA8, 0x71,
						0x3E, 0x63, 0xC5, 0x61, 0x73, 0xB6, 0xE1, 0x71,
						0x16, 0xDA, 0x06, 0xBF, 0x3F, 0x22, 0x74, 0x89,
						0x08, 0xD2, 0x05, 0x0B, 0x16, 0xC8, 0xF0, 0x17,
						0x4E, 0xA2, 0x65, 0x67, 0x6D, 0x02)
	},
	{
		NTRU_EES743EP1,
		chunk_from_chars(
						0x9B, 0xAB, 0x57, 0xDB, 0x2C, 0x60, 0x83, 0x48,
						0x9F, 0xC9, 0x70, 0x8F, 0x69, 0xF7, 0xB4, 0xBB,
						0x63, 0x5C, 0x9A, 0x63, 0x07, 0x80, 0x17, 0xD3,
						0xCD, 0xB1, 0x57, 0x79, 0xFE, 0x8D, 0x81, 0x70,
						0xEB, 0x50, 0xFA, 0x05, 0xFB, 0x97, 0xB2, 0xAB,
						0x25, 0xED, 0xD8, 0x18, 0x1C, 0xFE, 0x96, 0x7D),
		chunk_from_chars(
						0x02, 0x03, 0x00, 0x06, 0x10, 0x14, 0x53, 0x73,
						0x56, 0xF5, 0xA9, 0x34, 0xDE, 0xA6, 0x4D, 0x46,
						0x05, 0x9E, 0x80, 0xAE, 0xB6, 0x74, 0x91, 0xFF,
						0xFB, 0x48, 0xD3, 0x5C, 0x61, 0x12, 0x46, 0x02,
						0x9F, 0x53, 0x45, 0x87, 0x47, 0xBD, 0x6B, 0x26,
						0xF7, 0x36, 0xD3, 0x99, 0x1B, 0xD7, 0xEA, 0xA3,
						0xA8, 0x94, 0xFF, 0x93, 0x46, 0x7C, 0x2C, 0x5F,
						0x87, 0x8C, 0x38, 0xB3, 0x7B, 0xC6, 0x49, 0xE2,
						0x88, 0xCA, 0x67, 0x89, 0xD0, 0x6D, 0x7C, 0xAE,
						0x7C, 0x98, 0x84, 0xDA, 0x6B, 0x93, 0x92, 0xEF,
						0x4A, 0xD1, 0x4A, 0xD2, 0x5B, 0x13, 0xF8, 0x59,
						0x15, 0x2E, 0xBC, 0x70, 0x8D, 0x2D, 0xA9, 0x47,
						0xA1, 0x99, 0x19, 0x3F, 0x67, 0xE8, 0x18, 0xA7,
						0x17, 0x07, 0xB3, 0x14, 0xF6, 0x20, 0xA1, 0xD8,
						0x33, 0xE8, 0x08, 0x6A, 0xC1, 0x39, 0x99, 0x08,
						0xB4, 0x88, 0xEB, 0x48, 0x7D, 0xFB, 0xF5, 0xEF,
						0x03, 0x0D, 0x25, 0xB7, 0x98, 0xF3, 0xF1, 0x15,
						0x63, 0xE4, 0x0F, 0xFD, 0x54, 0x9F, 0x56, 0xE9,
						0xD1, 0x44, 0xE5, 0x89, 0x66, 0x14, 0x91, 0x1C,
						0xFD, 0xD6, 0xFD, 0x38, 0xAE, 0x39, 0xE3, 0xF7,
						0xCD, 0x77, 0xC2, 0xEA, 0x2E, 0xE4, 0xB7, 0x2B,
						0xBA, 0x7A, 0xD1, 0x75, 0xB8, 0x28, 0x65, 0x18,
						0xF4, 0xC6, 0xBD, 0xD0, 0x17, 0x7E, 0xEA, 0x86,
						0x7E, 0xFC, 0x95, 0xD6, 0x4C, 0x92, 0x01, 0xC3,
						0xFF, 0x04, 0x9B, 0xF8, 0xD6, 0xB3, 0x8F, 0x72,
						0xEF, 0x64, 0x09, 0x61, 0xF8, 0xE4, 0x48, 0xFC,
						0x0D, 0xEE, 0xEF, 0xA2, 0x9F, 0x3A, 0x2B, 0x1A,
						0xFB, 0x8B, 0xA0, 0x9C, 0x11, 0x0B, 0x97, 0x75,
						0x30, 0x7C, 0xB8, 0x9F, 0xEE, 0x3B, 0x53, 0x85,
						0x7D, 0xE9, 0xCB, 0xC4, 0x4D, 0xD7, 0x7F, 0x59,
						0x10, 0x72, 0x19, 0x3A, 0xC9, 0x38, 0xFE, 0xE8,
						0xB3, 0x06, 0x55, 0x8D, 0xA2, 0x5A, 0x3D, 0x79,
						0x67, 0x0E, 0x90, 0xC9, 0x25, 0x6D, 0x45, 0x9C,
						0x39, 0x79, 0x5F, 0x18, 0x35, 0x9F, 0xC1, 0x49,
						0x08, 0x6F, 0x1C, 0x47, 0x09, 0x0D, 0x49, 0x7C,
						0x3C, 0x7B, 0xB1, 0x09, 0x92, 0x1C, 0x4E, 0x5A,
						0xDA, 0x74, 0x9E, 0xBB, 0x55, 0x9D, 0xBB, 0x1E,
						0x43, 0x28, 0x62, 0xAF, 0x02, 0xB0, 0x1A, 0xEA,
						0x13, 0x0A, 0x70, 0x0F, 0x60, 0x0F, 0x62, 0xA2,
						0x4E, 0x1F, 0xB2, 0xEA, 0x06, 0xDD, 0x18, 0x02,
						0x6C, 0xF3, 0x82, 0xF1, 0x80, 0x7F, 0xA7, 0x2F,
						0xCC, 0xC6, 0x18, 0xEA, 0xFF, 0x1F, 0xAD, 0xC6,
						0xBA, 0x0C, 0x0E, 0x04, 0xB2, 0x58, 0x1D, 0xB6,
						0x01, 0xA3, 0x97, 0xDF, 0x7D, 0x9B, 0xB5, 0x0A,
						0xAD, 0x30, 0x2B, 0xC5, 0x67, 0x40, 0x07, 0xF1,
						0xD5, 0x6C, 0x11, 0x10, 0xE1, 0x69, 0x30, 0xAD,
						0x90, 0x06, 0xDB, 0xF8, 0xEA, 0x92, 0x9B, 0x39,
						0x57, 0x38, 0x7B, 0xE4, 0xB2, 0xA2, 0x89, 0xFD,
						0xB1, 0x6D, 0x88, 0x41, 0x62, 0x4D, 0x18, 0xB6,
						0x3F, 0x12, 0x81, 0xDE, 0xE6, 0xDC, 0x4A, 0x31,
						0x61, 0x26, 0xB1, 0x4B, 0x95, 0xC1, 0x69, 0xDC,
						0xDC, 0xAC, 0xD0, 0x15, 0xFC, 0x21, 0xC5, 0x20,
						0x5F, 0x97, 0x76, 0x41, 0xC1, 0xF2, 0xD7, 0x95,
						0x1D, 0x25, 0x23, 0x36, 0x86, 0xFA, 0x7E, 0xF4,
						0x14, 0x9F, 0x9D, 0x9F, 0xB2, 0xBB, 0x25, 0x1D,
						0xD5, 0x7A, 0x6F, 0x9E, 0xF7, 0xEF, 0x9D, 0x63,
						0x1E, 0xD5, 0xDE, 0x6A, 0xE6, 0x46, 0x48, 0x1F,
						0xE1, 0x0C, 0x4D, 0x82, 0xC9, 0x19, 0x3B, 0x65,
						0xA4, 0x06, 0x13, 0xB7, 0x04, 0xB1, 0x62, 0xF7,
						0x08, 0xAE, 0xED, 0x42, 0x6D, 0xCC, 0x6C, 0xA6,
						0x06, 0x06, 0x41, 0x3E, 0x0C, 0x89, 0x4C, 0xBD,
						0x00, 0x4F, 0x0E, 0xA9, 0x72, 0x06, 0x21, 0x82,
						0xD2, 0xB6, 0x6C, 0xB0, 0xB0, 0x01, 0x5B, 0xDD,
						0x05, 0xCE, 0x71, 0x6E, 0x00, 0x58, 0xC7, 0xA6,
						0x5B, 0xF6, 0xFB, 0x6B, 0x62, 0xB1, 0xE8, 0x4D,
						0xAC, 0xC0, 0x6B, 0xF4, 0x40, 0x69, 0xEE, 0x0D,
						0xE7, 0x82, 0x61, 0x8D, 0x35, 0x01, 0x97, 0x4E,
						0xF2, 0xCC, 0xF5, 0x7F, 0xBF, 0xE4, 0xEC, 0x9C,
						0xC4, 0xD2, 0xD9, 0x65, 0x78, 0x98, 0xD8, 0xB0,
						0xFA, 0xA8, 0xFB, 0xB0, 0xCE, 0x22, 0x5D, 0x0B,
						0x27, 0xDF, 0x0E, 0x63, 0x42, 0xFE, 0x89, 0x13,
						0x99, 0xB2, 0x02, 0x0B, 0xF6, 0x04, 0xB6, 0xAF,
						0x9F, 0x8C, 0xA6, 0x17, 0x0D, 0xD9, 0x5B, 0x45,
						0xE4, 0x08, 0x53, 0x51, 0xE0, 0xD5, 0x22, 0x72,
						0xBE, 0xAD, 0x74, 0x69, 0xB9, 0xFB, 0x91, 0xF8,
						0xC1, 0x89, 0x28, 0x71, 0x27, 0x62, 0xB1, 0xF0,
						0xFD, 0x78, 0xBC, 0x82, 0xFE, 0x76, 0xBE, 0x7B,
						0x47, 0x79, 0x32, 0x71, 0xAD, 0xD6, 0x76, 0x46,
						0xFB, 0x32, 0xE8, 0x4B, 0x98, 0x9A, 0xC6, 0x85,
						0xF2, 0xF1, 0x8A, 0xEC, 0xC2, 0x4E, 0x9B, 0x2F,
						0x2D, 0x6F, 0xC9, 0x9B, 0xB6, 0x14, 0x35, 0x6D,
						0xD6, 0x5B, 0xF3, 0x02, 0x5A, 0xE5, 0xBD, 0x00,
						0xF7, 0x6E, 0x51, 0xA7, 0xDB, 0x19, 0xAE, 0x01,
						0x01, 0x05, 0x94, 0x23, 0xF7, 0x5B, 0x07, 0x79,
						0xFF, 0x39, 0x58, 0x9C, 0x2A, 0xF7, 0x7E, 0x5D,
						0x81, 0xF9, 0x59, 0xFE, 0xB9, 0x9A, 0x96, 0x63,
						0x1F, 0x65, 0xF6, 0xF0, 0x3D, 0xEA, 0xD7, 0xC2,
						0x8A, 0xCF, 0xB5, 0x58, 0x74, 0x77, 0x23, 0xD6,
						0x72, 0x58, 0xA8, 0xAE, 0x31, 0x8A, 0x59, 0xEA,
						0x69, 0x14, 0x6A, 0x20, 0x78, 0x79, 0x28, 0x5A,
						0xE1, 0x76, 0x6F, 0xA6, 0x1A, 0x9E, 0x47, 0xD2,
						0xAF, 0x63, 0xF8, 0x06, 0xF6, 0xD8, 0xD5, 0x14,
						0xA8, 0xD1, 0xEE, 0x96, 0xCE, 0xBB, 0x8E, 0x22,
						0x69, 0x2F, 0x52, 0x06, 0xB6, 0x6F, 0xC8, 0x99,
						0x96, 0xEA, 0xC6, 0x1D, 0x96, 0x4C, 0x69, 0x95,
						0xFE, 0x74, 0x04, 0x3C, 0x55, 0xD9, 0x5F, 0xE0,
						0x41, 0x21, 0x43, 0x21, 0x5A, 0x50, 0x5D, 0x8B,
						0xE8, 0xB2, 0x51, 0x1B, 0x7C, 0x63, 0x50, 0xAE,
						0x97, 0x4F, 0xBA, 0x7D, 0xF2, 0xB6, 0xB6, 0x16,
						0x1D, 0x47, 0x9E, 0x19, 0x68, 0xD4, 0x6B, 0x2B,
						0x75, 0xCD, 0xAE, 0x65, 0x33, 0x38, 0xF6, 0x6D,
						0xC7, 0x3E, 0x46, 0x98, 0x9E, 0x98, 0x8B, 0x45,
						0x11, 0xA7, 0x12, 0x05, 0xB0, 0x01, 0xC3, 0x51,
						0xA0, 0xEE, 0x7C, 0x16, 0xD1, 0x42, 0x96, 0xC4,
						0xF0, 0x7B, 0x71, 0xCD, 0x50, 0x38, 0xA4, 0xB0,
						0x6E, 0x6F, 0xE0, 0xBD, 0xC4, 0xF7, 0x96, 0x2B,
						0xF1, 0x6D, 0x9F, 0xF3, 0x71, 0x89, 0xFA, 0xB4,
						0x44, 0xA4, 0x32, 0xDC, 0xB2, 0x55, 0x13, 0x31,
						0x83, 0x29, 0x66, 0x21, 0x3E, 0x89, 0xF8, 0x78,
						0x97, 0x9C, 0x64, 0xF9, 0x2C, 0x0A, 0x88, 0xBC,
						0xCA, 0x6F, 0x83, 0x42, 0xF6, 0xD7, 0x00, 0xC4,
						0x19, 0x52, 0xB0, 0x31, 0xA8, 0xBA, 0xE8, 0xD4,
						0xAD, 0x4B, 0x5D, 0xC0, 0x01, 0x20, 0x6C, 0xBB,
						0x1D, 0x9A, 0x1D, 0xD4, 0x19, 0xFD, 0x33, 0xAB,
						0xA0, 0x54, 0x50, 0x91, 0xE9, 0x75, 0x5C, 0x7E,
						0x7E, 0xB3, 0x24, 0x79, 0xAE, 0x10, 0x3C, 0xB4,
						0xB7, 0x0A, 0x1D, 0x86, 0xAD, 0x06, 0x95, 0xCB,
						0x84, 0x9B, 0x0E, 0x8B, 0x77, 0x7E, 0x3E, 0xD2,
						0xA6, 0xDF, 0xAD, 0x4E, 0xFB, 0x69, 0x23, 0xAC,
						0x7A, 0xCB, 0xAA, 0xB0, 0x22, 0xDD, 0xD2, 0xC6,
						0xC7, 0xAD, 0xD7, 0xDE, 0xEC, 0x6F, 0x08, 0x41,
						0x54, 0xD5, 0x52, 0xDC, 0x77, 0xE4, 0x72, 0xF9,
						0x16, 0xB1, 0xC9, 0xAF, 0xB1, 0x3B, 0x18, 0x99,
						0x20, 0x9F, 0x79, 0x63, 0x7B, 0x07, 0xC7, 0x35,
						0xDF, 0xBB, 0xCE, 0x66, 0x93, 0x1B, 0xF5, 0x82,
						0x25, 0x67, 0xC1, 0xF2, 0xF0, 0x89, 0x0F, 0xEF,
						0x84, 0x0D, 0x63, 0xB6, 0x7B, 0xD0, 0x40, 0x8E,
						0xDB, 0x94, 0xCC, 0x71, 0x3C, 0xDB, 0x36, 0x14,
						0x34, 0xFD, 0xA0, 0xB0, 0xC1, 0x45, 0x31, 0xF8,
						0x8D, 0xD8, 0x23, 0xB1, 0x05, 0x14, 0xA9, 0x55,
						0x3A, 0x1A, 0x37, 0x48, 0x68, 0x89, 0x3F, 0x15,
						0x25, 0xD4, 0x99, 0x53, 0x4C, 0x85, 0x98, 0x78,
						0x1D, 0x35, 0x4A, 0x83, 0x79, 0x9A, 0x29, 0x90,
						0x2B, 0x45, 0x76, 0x0C, 0x13, 0x80, 0x4A, 0xE0,
						0x40, 0xED, 0x6B, 0x2E, 0x2A, 0x43, 0xA9, 0x28,
						0xB0, 0x2F, 0x89, 0x01, 0x6B, 0x39, 0x8C, 0x5E,
						0x80, 0x61, 0xD9, 0xEE, 0x0F, 0x41, 0x75, 0xB5,
						0xAE, 0xB6, 0xC2, 0x42, 0x49, 0x8D, 0x89, 0xD8,
						0xF4, 0x78, 0x1D, 0x90, 0x46, 0x26, 0x4C, 0x56,
						0xB7, 0xC0, 0xD9, 0x98, 0x7B, 0x07, 0xA1, 0x20)
	}
};

START_TEST(test_ntru_privkey)
{
	rng_t *entropy;
	drbg_t *drbg;
	ntru_private_key_t *privkey;
	ntru_public_key_t *pubkey;
	ntru_param_set_t *params;
	uint32_t strength;
	chunk_t encoding, privkey_encoding, pubkey_encoding;

	params = TEST_FUNCTION(ntru, ntru_param_set_get_by_id,
						   privkey_tests[_i].id);
	strength = params->sec_strength_len * BITS_PER_BYTE;

	/* entropy rng will be owned by drbg */
	entropy = rng_tester_create(privkey_tests[_i].entropy);
	drbg = lib->crypto->create_drbg(lib->crypto, DRBG_HMAC_SHA256, strength,
									entropy, chunk_from_str("IKE NTRU-KE"));
	ck_assert(drbg != NULL);

	privkey = TEST_FUNCTION(ntru, ntru_private_key_create, drbg, params);
	ck_assert(privkey);
	ck_assert(privkey->get_id(privkey) == privkey_tests[_i].id);

	privkey_encoding = privkey->get_encoding(privkey);
	encoding = privkey_tests[_i].encoding;
	ck_assert(chunk_equals(privkey_encoding, encoding));

	/* load private key as a packed blob */
	privkey->destroy(privkey);
	privkey = TEST_FUNCTION(ntru, ntru_private_key_create_from_data,
							drbg, chunk_empty);
	ck_assert(privkey == NULL);

	encoding = chunk_clone(encoding);
	encoding.ptr[0] = NTRU_PUBKEY_TAG;
	privkey = TEST_FUNCTION(ntru, ntru_private_key_create_from_data,
							drbg, encoding);
	ck_assert(privkey == NULL);

	encoding.ptr[0] = NTRU_PRIVKEY_TRITS_TAG;
	privkey = TEST_FUNCTION(ntru, ntru_private_key_create_from_data,
							drbg, encoding);
	if (params->is_product_form)
	{
		ck_assert(privkey == NULL);
	}
	else
	{
		ck_assert(privkey != NULL);
		privkey->destroy(privkey);
	}

	encoding.ptr[0] = NTRU_PRIVKEY_INDICES_TAG;
	privkey = TEST_FUNCTION(ntru, ntru_private_key_create_from_data,
							drbg, encoding);
	if (params->is_product_form)
	{
		ck_assert(privkey != NULL);
		privkey->destroy(privkey);
	}
	else
	{
		ck_assert(privkey == NULL);
	}

	encoding.ptr[0] = NTRU_PRIVKEY_DEFAULT_TAG;
	encoding.ptr[1] = NTRU_OID_LEN - 1;
	privkey = TEST_FUNCTION(ntru, ntru_private_key_create_from_data,
							drbg, encoding);
	ck_assert(privkey == NULL);

	encoding.ptr[1] = NTRU_OID_LEN;
	encoding.ptr[2] = 0xff;
	privkey = TEST_FUNCTION(ntru, ntru_private_key_create_from_data,
							drbg, encoding);
	ck_assert(privkey == NULL);

	encoding.ptr[2] = params->oid[0];
	privkey = TEST_FUNCTION(ntru, ntru_private_key_create_from_data,
							drbg, encoding);
	privkey_encoding = privkey->get_encoding(privkey);
	ck_assert(chunk_equals(privkey_encoding, encoding));

	pubkey = privkey->get_public_key(privkey);
	pubkey_encoding = pubkey->get_encoding(pubkey);

	encoding.ptr[0] = NTRU_PUBKEY_TAG;
	encoding.len = pubkey_encoding.len;
	ck_assert(chunk_equals(pubkey_encoding, encoding));

	/* load public key as a packed blob */
	pubkey->destroy(pubkey);
	pubkey = TEST_FUNCTION(ntru, ntru_public_key_create_from_data,
						   drbg, encoding);
	pubkey_encoding = pubkey->get_encoding(pubkey);
	ck_assert(chunk_equals(pubkey_encoding, encoding));

	chunk_free(&encoding);
	privkey->destroy(privkey);
	pubkey->destroy(pubkey);
	drbg->destroy(drbg);
}
END_TEST

START_TEST(test_ntru_ke)
{
	chunk_t pub_key, cipher_text, i_shared_secret, r_shared_secret;
	key_exchange_t *i_ntru, *r_ntru;
	char buf[10];
	int k, n, len;

	k = (_i) / countof(parameter_sets);
	n = (_i) % countof(parameter_sets);

	len = snprintf(buf, sizeof(buf), "%N", key_exchange_method_names,
				   params[k].ke);
	ck_assert(len == 8);
	ck_assert(streq(buf, params[k].name));

	lib->settings->set_str(lib->settings,
				"libstrongswan.plugins.ntru.parameter_set", parameter_sets[n]);

	i_ntru = lib->crypto->create_ke(lib->crypto, params[k].ke);
	ck_assert(i_ntru != NULL);
	ck_assert(i_ntru->get_method(i_ntru) == params[k].ke);

	ck_assert(i_ntru->get_public_key(i_ntru, &pub_key));
	ck_assert(pub_key.len > 0);

	r_ntru = lib->crypto->create_ke(lib->crypto, params[k].ke);
	ck_assert(r_ntru != NULL);

	ck_assert(r_ntru->set_public_key(r_ntru, pub_key));
	ck_assert(r_ntru->get_public_key(r_ntru, &cipher_text));
	ck_assert(cipher_text.len > 0);

	ck_assert(r_ntru->get_shared_secret(r_ntru, &r_shared_secret));
	ck_assert(r_shared_secret.len > 0);

	ck_assert(i_ntru->set_public_key(i_ntru, cipher_text));
	ck_assert(i_ntru->get_shared_secret(i_ntru, &i_shared_secret));
	ck_assert(chunk_equals(i_shared_secret, r_shared_secret));

	chunk_clear(&i_shared_secret);
	chunk_clear(&r_shared_secret);
	chunk_free(&pub_key);
	chunk_free(&cipher_text);
	i_ntru->destroy(i_ntru);
	r_ntru->destroy(r_ntru);
}
END_TEST

START_TEST(test_ntru_retransmission)
{
	key_exchange_t *i_ntru;
	chunk_t pub_key1, pub_key2;

	i_ntru = lib->crypto->create_ke(lib->crypto, NTRU_256_BIT);
	ck_assert(i_ntru->get_public_key(i_ntru, &pub_key1));
	ck_assert(i_ntru->get_public_key(i_ntru, &pub_key2));
	ck_assert(chunk_equals(pub_key1, pub_key2));

	chunk_free(&pub_key1);
	chunk_free(&pub_key2);
	i_ntru->destroy(i_ntru);
}
END_TEST

chunk_t oid_tests[] = {
	{ NULL, 0 },
	chunk_from_chars(0x00),
	chunk_from_chars(0x01),
	chunk_from_chars(0x02),
	chunk_from_chars(0x02, 0x03, 0x00, 0x03, 0x10),
	chunk_from_chars(0x01, 0x04, 0x00, 0x03, 0x10),
	chunk_from_chars(0x01, 0x03, 0x00, 0x03, 0x10),
	chunk_from_chars(0x01, 0x03, 0xff, 0x03, 0x10),
};

START_TEST(test_ntru_pubkey_oid)
{
	key_exchange_t *r_ntru;
	chunk_t cipher_text;

	r_ntru = lib->crypto->create_ke(lib->crypto, NTRU_128_BIT);
	ck_assert(!r_ntru->set_public_key(r_ntru, oid_tests[_i]));
	ck_assert(r_ntru->get_public_key(r_ntru, &cipher_text));
	ck_assert(cipher_text.len == 0);
	r_ntru->destroy(r_ntru);
}
END_TEST

START_TEST(test_ntru_wrong_set)
{
	key_exchange_t *i_ntru, *r_ntru;
	chunk_t pub_key, cipher_text;

	lib->settings->set_str(lib->settings,
						  "libstrongswan.plugins.ntru.parameter_set",
			 			  "x9_98_bandwidth");
	i_ntru = lib->crypto->create_ke(lib->crypto, NTRU_112_BIT);
	ck_assert(i_ntru->get_public_key(i_ntru, &pub_key));

	lib->settings->set_str(lib->settings,
						  "libstrongswan.plugins.ntru.parameter_set",
						  "optimum");
	r_ntru = lib->crypto->create_ke(lib->crypto, NTRU_112_BIT);
	ck_assert(!r_ntru->set_public_key(r_ntru, pub_key));
	ck_assert(r_ntru->get_public_key(r_ntru, &cipher_text));
	ck_assert(cipher_text.len == 0);

	chunk_free(&pub_key);
	chunk_free(&cipher_text);
	i_ntru->destroy(i_ntru);
	r_ntru->destroy(r_ntru);
}
END_TEST

START_TEST(test_ntru_ciphertext)
{
	char buf_00[604], buf_ff[604];

	chunk_t test[] = {
		chunk_empty,
		chunk_from_chars(0x00),
		chunk_create(buf_00, sizeof(buf_00)),
		chunk_create(buf_ff, sizeof(buf_ff)),
	};

	key_exchange_t *i_ntru;
	chunk_t pub_key, shared_secret;
	int i;

	memset(buf_00, 0x00, sizeof(buf_00));
	memset(buf_ff, 0xff, sizeof(buf_ff));

	for (i = 0; i < countof(test); i++)
	{
		i_ntru = lib->crypto->create_ke(lib->crypto, NTRU_128_BIT);
		ck_assert(i_ntru->get_public_key(i_ntru, &pub_key));
		ck_assert(!i_ntru->set_public_key(i_ntru, test[i]));
		ck_assert(!i_ntru->get_shared_secret(i_ntru, &shared_secret));
		ck_assert(shared_secret.len == 0);

		chunk_free(&pub_key);
		i_ntru->destroy(i_ntru);
	}
}
END_TEST

START_TEST(test_ntru_wrong_ciphertext)
{
	key_exchange_t *i_ntru, *r_ntru, *m_ntru;
	chunk_t pub_key_i, pub_key_m, cipher_text, shared_secret;

	i_ntru = lib->crypto->create_ke(lib->crypto, NTRU_128_BIT);
	r_ntru = lib->crypto->create_ke(lib->crypto, NTRU_128_BIT);
	m_ntru = lib->crypto->create_ke(lib->crypto, NTRU_128_BIT);

	ck_assert(i_ntru->get_public_key(i_ntru, &pub_key_i));
	ck_assert(m_ntru->get_public_key(m_ntru, &pub_key_m));
	ck_assert(r_ntru->set_public_key(r_ntru, pub_key_m));
	ck_assert(r_ntru->get_public_key(r_ntru, &cipher_text));
	ck_assert(!i_ntru->set_public_key(i_ntru, cipher_text));
	ck_assert(!i_ntru->get_shared_secret(i_ntru, &shared_secret));
	ck_assert(shared_secret.len == 0);

	chunk_free(&pub_key_i);
	chunk_free(&pub_key_m);
	chunk_free(&cipher_text);
	i_ntru->destroy(i_ntru);
	m_ntru->destroy(m_ntru);
	r_ntru->destroy(r_ntru);
}
END_TEST

Suite *ntru_suite_create()
{
	Suite *s;
	TCase *tc;

	s = suite_create("ntru");

	tc = tcase_create("trits");
	tcase_add_loop_test(tc, test_ntru_trits, 0, countof(trits_tests));
	suite_add_tcase(s, tc);

	tc = tcase_create("poly");
	tcase_add_loop_test(tc, test_ntru_poly, 0, countof(trits_tests));
	suite_add_tcase(s, tc);

	tc = tcase_create("ring_mult");
	tcase_add_loop_test(tc, test_ntru_ring_mult, 0, countof(ring_mult_tests));
	suite_add_tcase(s, tc);

	tc = tcase_create("array");
	tcase_add_loop_test(tc, test_ntru_array, 0, countof(array_tests));
	suite_add_tcase(s, tc);

	tc = tcase_create("param_set");
	tcase_add_test(tc, test_ntru_param_set);
	suite_add_tcase(s, tc);

	tc = tcase_create("privkey");
	tcase_add_loop_test(tc, test_ntru_privkey, 0, countof(privkey_tests));
	suite_add_tcase(s, tc);

	tc = tcase_create("ke");
	tcase_add_loop_test(tc, test_ntru_ke, 0,
						countof(params) * countof(parameter_sets));
	suite_add_tcase(s, tc);

	tc = tcase_create("retransmission");
	tcase_add_test(tc, test_ntru_retransmission);
	suite_add_tcase(s, tc);

	tc = tcase_create("pubkey_oid");
	tcase_add_loop_test(tc, test_ntru_pubkey_oid, 0, countof(oid_tests));
	suite_add_tcase(s, tc);

	tc = tcase_create("wrong_set");
	tcase_add_test(tc, test_ntru_wrong_set);
	suite_add_tcase(s, tc);

	tc = tcase_create("ciphertext");
	tcase_add_test(tc, test_ntru_ciphertext);
	suite_add_tcase(s, tc);

	tc = tcase_create("wrong_ciphertext");
	tcase_add_test(tc, test_ntru_wrong_ciphertext);
	suite_add_tcase(s, tc);
	return s;
}
