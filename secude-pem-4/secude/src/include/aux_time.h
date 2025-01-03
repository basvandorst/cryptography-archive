		/* special value for type t_int */
#define T_INVALID	-1

		/* basic values for type t_delta */
#define T_1SEC		1
#define T_1MIN		60
#define T_1HOUR		( 60*60 )
#define T_1DAY		( 24*60*60 )
#define T_1MONTH	( 30*24*60*60 )		/* Aenderung J.Luehe 15.Juni 1992 */
#define T_1YEAR         ( 365*24*60*60 )


		/* SYSTEM DEPENDANT type definitions */
typedef int             t_int;
typedef int             t_delta;


typedef struct
  {
    int		sec;	/* seconds		0 -> 59			*/
    int		minu;	/* minutes		0 -> 59			*/
    int		hour;	/* hours		0 -> 23			*/
    int		day;    /* day of month		0 -> (27,28,29,30)	*/
    int		mon;	/* month		0 -> 11			*/
    int		year;   /* year			AD			*/
    int		zone;	/* time zone (minutes east of GMT) -720 -> 720	*/
  }
	T_REC;


static t_int            t_now();

static T_REC*           t_itor();
static t_int            t_rtoi();

static int              t_rvalid();



		/* X.409 representaton conversion */

#define TX_MAXLEN	17


static char*            t_rtox();
static T_REC*           t_xtor();


		/* miscellaneous */

static int              t_isleap();
static int              t_dpm();
static int              t_doy();
static int              t_dow();


		/* convenience */

static T_REC*           t_rnow();


		/* time interface */

#ifdef __STDC__
/***************************************************************
 *
 * Headers from file aux/aux_time.c
 *
 ***************************************************************/ 
       UTCTime	*aux_current_UTCTime	();
       char	*aux_readable_UTCTime	(UTCTime *tx);
       UTCTime	*aux_delta_UTCTime	(UTCTime *time);
       int	aux_interval_UTCTime	(UTCTime *time, UTCTime *notbefore, UTCTime *notafter);
       int	aux_cmp_UTCTime	(UTCTime *time1, UTCTime *time2);
       UTCTime	*delta_time_rec	(UTCTime *time, T_REC *delta);
       UTCTime	*get_nextUpdate	(UTCTime *lastUpdate);
       UTCTime	*get_date_of_expiry	();

#else
/***************************************************************
 *
 * Headers from file aux/aux_time.c
 *
 ***************************************************************/ 
       UTCTime	*aux_current_UTCTime	();
       char	*aux_readable_UTCTime	();
       UTCTime	*aux_delta_UTCTime	();
       int	aux_interval_UTCTime	();
       int	aux_cmp_UTCTime	();
       UTCTime	*delta_time_rec	();
       UTCTime	*get_nextUpdate	();
       UTCTime	*get_date_of_expiry	();

#endif
