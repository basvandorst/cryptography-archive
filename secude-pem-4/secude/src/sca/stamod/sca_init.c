/*
 *  STARMOD Release 1.1 (GMD)
 */

/********************************************************************
 * Copyright (C) 1991, GMD. All rights reserved.                    *
 *                                                                  *
 *                                                                  *
 *                         NOTICE                                   *
 *                                                                  *
 *    Acquisition, use, and distribution of this module             *
 *    and related materials are subject to restrictions             *
 *    mentioned in each volume of the documentation.                *
 *                                                                  *
 ********************************************************************/

/*------------------------------------------------------------------*/
/* FILE  sca_init.c                                                 */
/* Initialization of global variables of SCA-IF                     */
/*------------------------------------------------------------------*/
#ifndef _SCA_
#include "sca.h"
#endif

#ifndef _STARMOD_
#include "stamod.h"
#endif

Request 	Gsct_request;		/* body of the SCT commands */
OctetString 	Gsct_response;		/* body of the SCT response */
int		Gsct_cmd;		/* INS of the SCT command   */
struct s_command Gicc_param;		/* body of the ICC command  */
OctetString     Gicc_apdu;		/* created icc command      */
SCTInfo  	sctinfo;	        /* structure of sct_info-parameter  */



struct SCA_ErrList  sca_errlist[] = {
/*       SW1          SW2           Message                                 */
{/*  0*/0x10,        0,            "T: not defined"                },
{/*  1*/0x00,        0x19,         "T: Device busy (port locked) "          },
{/*  2*/0x00,        0x21,         "T: Write error"                         },
{/*  3*/0x10,        0,            "T: not defined     "                    },
{/*  4*/0x00,        0x23,         "T: Memory error   "                     },
{/*  5*/0x00,        0x24,         "T: Open error "                         },
{/*  6*/0x10,        0x25,         "T: no port open"                        },
{/*  7*/0x10,        0,            "T: not defined"                       },
{/*  8*/0x10,        0,            "T: not defined       "                },
{/*  9*/0x10,        0,            "T: not defined"                      },
{/* 10*/0x00,        0x29,         "T: Length of TPDU-request is too long"  },
{/* 11*/0x00,        0x30,         "T: Device file not available"                  },
{/* 12*/0x10,        0,            "T: not defined"             },
{/* 13*/0x00,        0x32,         "T: Protocol has been resynchronized"   },
{/* 14*/0x00,        0x33,         "T: SCT should be reset "               },
{/* 15*/0x10,        0,            "T: not defined"                 },
{/* 16*/0x10,        0,            "T: not defined"                        },
{/* 17*/0x10,        0,            "T: not defined"                        },
{/* 18*/0x10,        0,            "T: not defined"                        },
{/* 19*/0x10,        0,            "T: not defined"                        },
/*                                                                    */
{/* 20*/0x41,        0x00,         "SCT: CLASS-Byte invalid   "             },
{/* 21*/0x41,        0x02,         "SCT: Key-ID invalid   "                 },
{/* 22*/0x41,        0x03,         "SCT: Algorithm Identifier invalid "     },
{/* 23*/0x41,        0x04,         "SCT: Operation mode invalid       "     },
{/* 24*/0x41,        0x06,         "SCT: Parameter in body incorrect       "},
{/* 25*/0x41,        0x07,         "SCT: Data-Length incorrect        "     },
{/* 26*/0x41,        0x09,         "SCT: P1 / P2 incorrect            "   },
{/* 27*/0x41,        0x0A,         "SCT: data length level 2 incorrect   "   },
{/* 28*/0x41,        0x0B,         "SCT: communication counter incorrect"   },
{/* 29*/0x41,        0x0C,         "SCT: wrong transition state         "   },
{/* 30*/0x41,        0x0D,         "SCT: CCS invalid         "   },
/*                                                                    */
{/* 31*/0x42,        0x00,         "SCT: Specified key not found "          },
{/* 32*/0x42,        0x01,         "SCT: Key and algorithm inconsistent  " },
{/* 33*/0x42,        0x02,         "SCT: Not allowed to replace existing key"},
{/* 34*/0x42,        0x03, 	   "SCT: Key-Information from keycard incorrect"},
{/* 35*/0x42,        0x04, 	   "SCT: Application Identifier invalid"},
{/* 36*/0x42,        0x05, 	   "SCT: Key Set inconsistent"},
{/* 37*/0x42,        0x06, 	   "SCT: Application Identifier not found"},
/*                                                                    */
{/* 38*/0x43,        0x02,         "SCT: Length of ciphertext incorrect"     },
{/* 39*/0x43,        0x03,         "SCT: Signature invalid            "      },
{/* 40*/0x43,        0x04,         "SCT: Key-Length invalid           "      },
{/* 41*/0x43,        0x05,         "SCT: no memory available          "      },
{/* 42*/0x43,        0x06,         "SCT: Authentication with Smartcard failed"  },
{/* 43*/0x43,        0x08,         "SCT: Execution denied                  "},
{/* 44*/0x43,        0x0A,         "SCT: Secure messaging key undefined    "},
{/* 45*/0x43,        0x0B,         "SCT: Authentication key undefined      "},
{/* 46*/0x43,        0x0C,         "SCT: smartcard - secure messaging error      "},
/*                                                                    */
{/* 47*/0x44,        0x00,         "SCT: No Smartcard in SCT          "     },
{/* 48*/0x44,        0x01,         "SCT: RESET of ICC not successful   "     },
{/* 49*/0x44,        0x02,         "SCT: Smartcard removed            "     },
{/* 50*/0x44,        0x03,         "SCT: Timeout - no answer from ICC "     },
{/* 51*/0x44,        0x04,         "SCT: Break from user              "     },
{/* 52*/0x44,        0x05,         "SCT: Timeout - no answer from user"     },
{/* 53*/0x44,        0x06,         "SCT: smartcard - data length level 2 incorrect"     },
/*                                                                    */
{/* 54*/0x45,        0x00,         "SCT: DSP - internal addressing error"    },
{/* 55*/0x45,        0x01,         "SCT: DSP - communication error (timeout)" },
{/* 56*/0x45,        0x02,         "SCT: DSP - parameter invalid"                  },
{/* 57*/0x45,        0x03,         "SCT: DSP - not available"                  },
{/* 58*/0x45,        0x04,         "SCT: DSP - general error"                  },
{/* 59*/0x45,        0xFF,         "SCT: DSP - system error"},
/*                                                                   */
{/* 60*/0x41,        0x0E,         "SCT:  Secure Messaging key between DTE and STARTERM not found"    },
{/* 61*/0x10,        0,            "ICC:  not defined              "    },
{/* 62*/0x60,        0x00,         "ICC:  Selection error; File not of type Parent_DF  "   },
{/* 63*/0x61,        0x00,         "ICC:  The READ command demands more bytes than the ICC I/O-buffer can receive"    },
{/* 64*/0x62,        0x00,         "ICC:  The file already exists"   },
{/* 65*/0x62,        0x81,         "ICC:  Data are inconsistent"    },
{/* 66*/0x62,        0x82,         "ICC:  The end of file/record was reached or the variable record is empty "    },
{/* 67*/0x62,        0x84,         "ICC:  File is locked " },
{/* 68*/0x62,        0x85,         "ICC:  Data structure not allowed " },
{/* 69*/0x63,        0x00,         "ICC:  The key fault presentation counter is run down" },
{/* 70*/0x64,        0x00,         "ICC:  ACF not found or internal structure invalid" },
{/* 71*/0x65,        0x00,         "ICC:  EEPROM error or the key fault presentation counter has no valid value"    },
/*                                                                    */
{/* 72*/0x67,        0x00,         "ICC:  The length of the PIN/NEW_PIN or the specified length in the command is invalid  "    },
{/* 73*/0x68,        0x00,         "ICC:  Wrong send sequence counter or wrong MAC       "    },
/*                                                                   */
{/* 74*/0x69,        0x00,         "ICC:  Either in the WORM-file the record exists already or the state-maschine not allows the command "    },
{/* 75*/0x69,        0x81,         "ICC:  Read/write/close/lock access not allowed"    },
/*                                                                   */
{/* 76*/0x69,        0x83,         "ICC:  Key locked"    },
{/* 77*/0x69,        0x85,         "ICC:  Invalid key selection or DELETE FILE command not allowed or file not active         "    },
{/* 78*/0x6A,        0x00,         "ICC:  No random number exists or parameter value is invalid"    },
{/* 79*/0x6A,        0x82,         "ICC:  Access on file is not allowed "    },
{/* 80*/0x6A,        0x83,         "ICC:  Record identifier/record number not found or invalid      "    },
{/* 81*/0x6A,        0x84,         "ICC:  Not enough space available   "    },
{/* 82*/0x6A,        0x86,         "ICC:  Parameter value in p1/p2 invalid"   },
{/* 83*/0x6B,        0x00,         "ICC:  Offset invalid"    },
{/* 84*/0x6D,        0x00,         "ICC:  Instruction code invalid"   },
{/* 85*/0x6F,        0x80,         "ICC:  The PUK-record refers to no PIN-record "   },
{/* 86*/0x6F,        0x81,         "ICC:  system error    "   },
{/* 87*/0x6F,        0x82,         "ICC:  Clear PIN not allowed "   },
{/* 88*/0x69,        0x82,         "ICC:  Security mode in CLA-Byte missing"},
{/* 89*/0x63,        0x81,         "ICC:  Authentication failed; 1 attempt left "},
{/* 90*/0x63,        0x82,         "ICC:  Authentication failed; 2 attempts left "   },
{/* 91*/0x63,        0x83,         "ICC:  Authentication failed; 3 attempts left "   },
/*                                                                    */
{/* 92*/0x63,        0x84,         "ICC:  Authentication failed; 4 attempts left  "   },
{/* 93*/0x63,        0x85,         "ICC:  Authentication failed; 5 attempts left "   },
{/* 94*/0x63,        0x86,         "ICC:  Authentication failed; 6 attempts left "   },
{/* 95*/0x63,        0x87,         "ICC:  Authentication failed; 7 or more attempts left  "   },
{/* 96*/0x69,        0x01,         "ICC:  File attribute(s) invalid "   },
{/* 97*/0x6F,        0x87,         "ICC:  Record identifier or record number invalid"   },
{/* 98*/0x91,        0x02,         "ICC:  Warning; PIN authentication disengaged  "   },
{/* 99*/0x91,        0x04,         "ICC:  Warning; PIN authentication successful because of disengaged PIN"   },
{/*100*/0x91,        0x08,         "ICC:  Warning; KFPC is reset but PIN value could not be changed "   },
{/*101*/0x00,        101,     "STA:  Local memory of the installed SCTs is not created      "   },
/*                                                                    */
{/*102*/0x00,        102,     "STA: wrong send sequence counter "   },
{/*103*/0x00,        103,     "STA: parameter value invalid "   },
{/*104*/0x00,        104,     "STA: invalid algorithm identifier "   },
{/*105*/0x00,        105,     "STA: invalid argument            "   },
{/*106*/0x00,        106,     "STA: no memory available         "   },
{/*107*/0x00,        107,     "STA: received MAC invalid        "   },
{/*108*/0x00,        108,     "STA: APDU too long or length of parameter invalid"   },
{/*109*/0x00,        109,     "STA: shell variable STAMOD in environment not found" },
{/*110*/0x00,        110,     "STA: can't open Installation File    " },
{/*111*/0x00,        111,     "STA: Installation File empty  " },
{/*112*/0x00,        112,     "STA: Installation File not successfully closed"},
{/*113*/0x00,        113,     "STA: Read error on Installation File     "},
{/*114*/0x00,        114,     "STA: SCT-ID unknown                      "},
{/*115*/0x00,        115,     "STA: Instructioncode not supported       "},
{/*116*/0x00,        116,     "STA: Baudvalue not allowed (< 2400)      "},
{/*117*/0x00,        117,     "STA: Data can`t be encrypted             "},
{/*118*/0x00,        118,     "STA: Data can`t be decrypted             "},
{/*119*/0x00,        119,     "STA: Can't generate sessionkey           "},
{/*120*/0x00,        120,     "STA: Error after RSA encryption          "},
{/*121*/0x00,        121,     "STA: Can't set RSA key            "},
{/*122*/0x00,        122,     "STA: wrong send sequence counter from SCT"   },
{/*123*/0x00,        123,     "STA: parameter inconsistent  "},
{/*124*/0x00,        124,     "STA: invalid key length  "},
{/*125*/0x00,        125,     "STA: error in response data  "},
{/*126*/0x00,        126,     "STA: mixed function calls not allowed "},
{/*127*/0x00,        127,     "STA: key from keycard not installed in SCT "},
{/*128*/0x00,        128,     "STA: hash-function error  "},
{/*129*/0x00,        129,     "STA: algorithm identifiers are not identical"},
{/*130*/0x00,        130,     "STA: no sessionkey for the integrity mode generated"},
{/*131*/0x00,        131,     "STA: no sessionkey for the concealed mode generated"},
{/*132*/0x00,        132,     "STA: error while discarding padding bytes"},
{/*133*/0x00,        133,     "STA: padding method in the ICC does not belong to the algorithm"},
{/*134*/0x00,        134,     "STA: smartcard already inserted"},
{/*135*/0x00,        135,     "STA: smartcard still inserted - must be removed"},
{/*136*/0x00,        136,     "STA: SCT in state zero"},
{/*137*/0x00,        137,     "STA: Error in PKCS routine"},
{/*138*/0,           0,       "STA: Error number not defined    "}};

