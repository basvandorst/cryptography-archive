#include "global.h"
#include "rsaref.h"
static unsigned char p0[] = {
 0xf1, 0x34, 0xa6, 0x8c, 0xf0, 0xaf,
 0x4b, 0x5f, 0x9d, 0xaa, 0x13, 0xdc,
 0x50, 0x2f, 0x80, 0x42, 0xb2, 0x93,
 0x89, 0x91, 0x47, 0x6e, 0x90, 0x64,
 0xf5, 0x01, 0x90, 0xed, 0x92, 0x70,
 0xe5, 0xab, 0xe3, 0x6f, 0x5a, 0x43,
 0xcf, 0xe2, 0x4f, 0x74, 0x93, 0xcd,
 0x26, 0x3a, 0x97, 0x0d, 0xaf, 0x74,
 0x89, 0xb8, 0x9a, 0x8a, 0x26, 0x14,
 0xd2, 0x22, 0x29, 0x36, 0x0d, 0x91,
 0x3a, 0x5a, 0xb9, 0xa9};
static unsigned char g0[] = {
 0x31, 0xc3, 0x07, 0x01, 0xda, 0x60,
 0x03, 0x71, 0xf6, 0x6a, 0xaf, 0x0f,
 0x0f, 0x8b, 0x2c, 0xa8, 0x25, 0x5e,
 0x0f, 0x51, 0x35, 0xc9, 0xb8, 0x83,
 0xf4, 0xe8, 0x52, 0xf7, 0x4b, 0xe7,
 0x29, 0x81, 0x4d, 0x76, 0xeb, 0x1d,
 0xf6, 0x13, 0x79, 0xc6, 0x20, 0x66,
 0x2b, 0xa8, 0x8a, 0xa1, 0x9a, 0xf5,
 0xca, 0x51, 0x95, 0x62, 0x31, 0xe4,
 0x6e, 0x4d, 0xd7, 0x72, 0x41, 0x06,
 0xe5, 0x13, 0x69, 0xbd};
static unsigned char p1[] = {
 0xa8, 0x02, 0xf3, 0x46, 0x47, 0xe1,
 0x42, 0x0a, 0x86, 0xca, 0xe6, 0x4e,
 0x95, 0xa9, 0xd2, 0xad, 0x90, 0x39,
 0x53, 0xad, 0x20, 0x88, 0x35, 0x27,
 0xc1, 0xb9, 0xc1, 0xd2, 0x9a, 0x07,
 0xde, 0xc8, 0x5d, 0xdd, 0x67, 0x36,
 0x50, 0xe4, 0xea, 0x2c, 0xb8, 0xf7,
 0x04, 0xd2, 0xb2, 0x34, 0xc1, 0xdd,
 0xae, 0x66, 0x20, 0x5f, 0xe3, 0x0b,
 0xac, 0x3f, 0x7d, 0x47, 0x81, 0xe6,
 0x57, 0xe8, 0xdf, 0x86, 0x06, 0x8f,
 0x81, 0xc4, 0x33, 0x71, 0x42, 0x7c,
 0x9d, 0x87, 0x18, 0x87, 0x7b, 0x81,
 0x4a, 0xb0, 0x62, 0x19, 0x86, 0xde,
 0x04, 0xea, 0xa2, 0x1b, 0x25, 0xb3,
 0xa7, 0x2e, 0x2c, 0xca, 0xe1, 0x3d};
static unsigned char g1[] = {
 0x80, 0xbb, 0xa6, 0x6e, 0x14, 0x6f,
 0x8e, 0x34, 0x99, 0x4e, 0x99, 0xde,
 0xe3, 0xdb, 0x9b, 0x73, 0x02, 0x05,
 0xcd, 0x37, 0x29, 0x1b, 0xd0, 0x75,
 0x34, 0x42, 0x93, 0x16, 0x86, 0x21,
 0x1f, 0x8e, 0x90, 0x77, 0x03, 0xd3,
 0x1f, 0x06, 0x51, 0x90, 0x01, 0x76,
 0x48, 0x04, 0x1b, 0xd1, 0x16, 0x68,
 0x8b, 0x5f, 0x23, 0x4d, 0xe1, 0x41,
 0x55, 0xa0, 0x03, 0x1c, 0xf7, 0xe4,
 0x47, 0x8f, 0xea, 0xf1, 0x42, 0x4a,
 0x04, 0x3b, 0x34, 0x86, 0x08, 0xef,
 0x55, 0x54, 0x52, 0xa6, 0x91, 0xb3,
 0x96, 0xcd, 0x09, 0xa8, 0xa2, 0x64,
 0xd8, 0x14, 0x41, 0x1f, 0xe6, 0x27,
 0x66, 0x54, 0xf3, 0xfd, 0xd3, 0xe3};
static unsigned char p2[] = {
 0xfa, 0x18, 0xad, 0x25, 0x10, 0xa2,
 0x22, 0x5e, 0x00, 0x1e, 0xb7, 0x09,
 0x80, 0x2c, 0x42, 0x22, 0x74, 0x30,
 0x5e, 0xbf, 0xfd, 0x11, 0x1b, 0xf9,
 0xf0, 0x1e, 0x6b, 0x9d, 0x89, 0xfd,
 0x2c, 0x70, 0x26, 0x96, 0x0a, 0x52,
 0x11, 0xf1, 0x53, 0x77, 0x37, 0xd1,
 0xbe, 0x3a, 0x2f, 0x3c, 0xcc, 0x5e,
 0xc1, 0x89, 0xfd, 0x04, 0x28, 0x5e,
 0x0f, 0x42, 0xa1, 0x8f, 0x37, 0xa9,
 0x80, 0xf6, 0x80, 0x7b, 0x59, 0xc5,
 0x87, 0x3d, 0xf0, 0x80, 0x40, 0x68,
 0x51, 0x19, 0x74, 0xea, 0x29, 0xe7,
 0xb6, 0x47, 0x0a, 0x22, 0xaf, 0xc7,
 0x32, 0xbe, 0x6b, 0x33, 0x64, 0xf2,
 0x96, 0xd0, 0x00, 0x42, 0xdf, 0x36,
 0x3b, 0x37, 0x34, 0xac, 0xd3, 0xb9,
 0x4a, 0x43, 0xde, 0x61, 0x92, 0xd8,
 0xa7, 0xc4, 0xe2, 0xb2, 0x5a, 0x74,
 0xc9, 0x89, 0x93, 0xae, 0x84, 0x26,
 0xb0, 0xd8, 0x8f, 0x1b, 0x74, 0x6d,
 0x3b, 0x79};
static unsigned char g2[] = {
 0x62, 0xe0, 0xa7, 0x2a, 0xf4, 0x9e,
 0x5e, 0xb7, 0x6d, 0xfd, 0x80, 0xd9,
 0x12, 0x8d, 0xcc, 0x8d, 0xd3, 0xb3,
 0xf6, 0xf1, 0x2d, 0xb0, 0x38, 0xa3,
 0x0f, 0xf5, 0x1d, 0x3c, 0x75, 0x24,
 0xc6, 0x59, 0xc4, 0xb3, 0xe3, 0xbc,
 0x90, 0x67, 0x83, 0xce, 0xec, 0xa8,
 0x15, 0x2c, 0x1e, 0x8a, 0x0a, 0x61,
 0xd2, 0x36, 0x22, 0xa6, 0x2a, 0x43,
 0xe5, 0x90, 0xac, 0x43, 0x2d, 0xf5,
 0xbd, 0x96, 0x4e, 0xe6, 0xa4, 0x47,
 0x4b, 0x2d, 0x8c, 0x4d, 0xd3, 0xd2,
 0x33, 0x39, 0x1a, 0xf5, 0x4d, 0x11,
 0xcc, 0xf6, 0x6b, 0xb2, 0x95, 0xaa,
 0x11, 0x5b, 0xd4, 0x02, 0x60, 0x79,
 0xbd, 0x68, 0x01, 0x57, 0x5b, 0x98,
 0x1e, 0x21, 0x04, 0x37, 0x12, 0x5d,
 0x66, 0x62, 0x07, 0x31, 0x85, 0xc9,
 0xfa, 0xf1, 0x64, 0xd4, 0x49, 0xfd,
 0xee, 0xc5, 0x2f, 0x9a, 0x3a, 0x06,
 0x77, 0x9d, 0x66, 0x69, 0x61, 0xef,
 0x92, 0x42};

R_DH_PARAMS dhparams[3] = {
 {p0, 64, g0, 64},
 {p1, 96, g1, 96},
 {p2, 128, g2, 128}}
;
