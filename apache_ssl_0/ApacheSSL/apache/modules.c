/* modules.c --- automatically generated by Apache
 * configuration script.  DO NOT HAND EDIT!!!!!
 */

#include "httpd.h"
#include "http_config.h"

extern module core_module;
extern module mime_module;
extern module access_module;
extern module auth_module;
extern module negotiation_module;
extern module includes_module;
extern module dir_module;
extern module cgi_module;
extern module userdir_module;
extern module alias_module;
extern module common_log_module;
extern module asis_module;
extern module imap_module;

module *prelinked_modules[] = {
  &core_module,
  &mime_module,
  &access_module,
  &auth_module,
  &negotiation_module,
  &includes_module,
  &dir_module,
  &cgi_module,
  &userdir_module,
  &alias_module,
  &common_log_module,
  &asis_module,
  &imap_module,
  NULL
};
