#define YYEMPTY (-1)
#define DST 257
#define SRC 258
#define HOST 259
#define GATEWAY 260
#define NET 261
#define PORT 262
#define LESS 263
#define GREATER 264
#define PROTO 265
#define BYTE 266
#define ARP 267
#define RARP 268
#define IP 269
#define TCP 270
#define UDP 271
#define ICMP 272
#define DECNET 273
#define LAT 274
#define MOPRC 275
#define MOPDL 276
#define TK_BROADCAST 277
#define TK_MULTICAST 278
#define NUM 279
#define INBOUND 280
#define OUTBOUND 281
#define LINK 282
#define GEQ 283
#define LEQ 284
#define NEQ 285
#define ID 286
#define EID 287
#define HID 288
#define LSH 289
#define RSH 290
#define LEN 291
#define OR 292
#define AND 293
#define UMINUS 294
typedef union {
	int i;
	u_long h;
	u_char *e;
	char *s;
	struct stmt *stmt;
	struct arth *a;
	struct {
		struct qual q;
		struct block *b;
	} blk;
	struct block *rblk;
} YYSTYPE;
extern YYSTYPE yylval;
