#ifdef NETDEBUG
#include "debug.h"
#include "ytalk.h"

#include <ctype.h>
#include <stdarg.h>

/* Log file name */

#define LOGNAME "debug-log"

static FILE *logfile = NULL;

int
__debug_printf (const char *format, ...)
{
  va_list args;
  int result;
  
  va_start (args, format);
  result = vfprintf (logfile, format, args);
  va_end (args);
  fflush (logfile);
  return result;
}

int
init_debug ()
{
  if (logfile != NULL)
    return -1;
  
  logfile = fopen (LOGNAME, "w");
  if (logfile == NULL)
    return -1;
  (void) setvbuf (logfile, NULL, _IOLBF, 0);
  return 0;
}

void
exit_debug ()
{
  (void) fclose (logfile);
  logfile = 0;
}

/* Debugging routines to dump cpacks.  */
void
debug_hexdump (const byte *array, int length)
{
  int i, j;
  
  for (i = 0; i < 250; i += 16) {
    debug_printf (("\n%08x ", i));
    for (j = i; j < i + 16 && j < 250; j++)
      debug_printf (("%02x ", array[j]));
    if (j % 16 != 0) {
      for (; j % 16 != 0; j++)
	debug_printf (("   "));
    }
    for (j = i; j < i + 16 && j < 250; j++)
      debug_printf (("%c", isgraph (array[j]) ? array[j] : '.'));
  }
  debug_printf (("\n"));
}

static const char *
code_to_name (s_char code)
{ 
  switch (code) {
  case EXPORT: return "EXPORT"; break;
  case IMPORT: return "IMPORT"; break;
  case ACCEPT: return "ACCEPT"; break;
  case AUTO: return "AUTO"; break;
  case CRYPT: return "CRYPT"; break;
  case CRYPTP: return "CRYPTP"; break;
  case CRYPTIV: return "CRYPTIV"; break;
  case CRYPTOFF: return "CRYPTOFF"; break;
  case CRYPTSIG: return "CRYPTSIG"; break;
  default: break;
  }
}

void
debug_checkcpack (cpack *crec)
{
  int i;
  int j;

  if (crec->code >= 0 || crec->code < -126 ||
      (crec->code > -118 && crec->code != -2))
    debug_printf (("Cpack code %d invalid...", crec->code));
  else
    debug_printf (("Cpack \"%s\"...", code_to_name (crec->code)));

  if (crec->code >= -122 && crec->code <= -118) {
    /* check crypto stuff for mutilation */
    for (i = 0; i < 250; i++) {
      if ((crec->ci_sig[i] & 0x80) == 0x80)
	break;
    }
    
    if (i == 250)
      debug_printf (("crypto stuff in 7-bit code..."));
    else
      debug_printf (("eight-bit clean..."));
  }

  debug_printf (("done\n"));
  debug_hexdump (crec->ci_sig, 250);
  debug_printf (("\n\n"));
}
#endif /* NETDEBUG */
