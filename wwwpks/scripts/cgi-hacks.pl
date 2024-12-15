#!/usr/contrib/bin/perl

# STOLE THE FOLLOWING FROM ai cluster /com/doc/web-support/cgi-hacks 

# Useful routines for writing CGI scripts (and general puttering about)

# '&find_form_args' takes as (first) argument the expected field names
# separated by '|', as in:

#        &find_form_args("name|address|phone|wants_pepperoni");

# On return, the variables whose names are those of the form fields are
# set to the appropriate values (unescaped, if necessary, so that for
# type-in fields what you get is what the user typed).  These would be
# $name, $address, $phone, and $wants_pepperoni in the above example.

# Second argument, if any, is the name of an associative array, indexed
# by field-name, which gets the values supplied for unexpected fields (i.e.,
# those with names not listed in the first argument).  If no second
# argument is supplied, and an unexpected field is found, we &cgidie;
# this is to help catch misspellings, and also may help with security
# (so that a malicious client can't cause arbitrary variables to be set,
# with potentially disruptive effects).  

# Return value is true if we clearly got arguments, false if the
# caller looked as if he was looking for a coversheet.  This is to
# facilitate such stuff as:

#   do { print <<EOF; exit(0); } unless (&find_form_args (...));
#   Content-type: text/html
#
#   ... contents of cover sheet
#   EOF

sub find_form_args
{
  local($var_ptrn, $catch_array) = @_;
  local($var_rx) = '^(' . $var_ptrn . ')=';

  local($is_get) = ($ENV{'REQUEST_METHOD'} eq 'GET');
  local($qry);
  local($nvars) = 0;

  if ($is_get) { $qry = $ENV{'QUERY_STRING'}; }
  else { read (STDIN, $qry, $ENV{'CONTENT_LENGTH'}); } 

  $qry =~ s/\+/ /g;

  foreach (split (/\&/, $qry))
  {
    if (s/$var_rx//)
      { eval "\$$1 = &unescape(\$_);"; }
    elsif ($catch_array ne '' && s/^([^=]*)=//)
      { eval "\$$catch_array{&unescape(\$1)} = &unescape(\$_);"; }
    else
      { &cgidie ("Unexpected field received for this form: $_"); }

    ++$nvars;
  }

  # Return...
  ($nvars > 0) || (! $is_get);
}

# Error handling ... the well-tempered CGI script doesn't croak without
# making sure that someone has a chance to see its dying gasp; that means
# giving it a proper Content-type line (and hoping it doesn't get too
# screwed up).

# The first argument is an error message, which gets potential HTML syntax
# ('<', '>', '&') escaped out so that it is seen by the user verbatim.  The
# second argument is optional; if present, it is treated as HTML (i.e.,
# left as is), for scripts which want to die a particularly elaborate
# death.

# Note that if the script has already produced output before calling this,
# things will be a little garbled all around, but still somewhat better than
# nothing.

sub cgidie
{
  local ($header, $body) = @_;
  $header = &HTMLize ($header);

  if ($body eq '') { $body = $header; $header = '' }
  if ($header eq '') { $header = "Error in processing your request"; }

  print <<EOF;
Content-type: text/html

<title> $header </title>
<h1> $header </h1>
$body
EOF
  exit(1);
}

# Self-referencing URL generation... the value of this function is, loosely
# speaking, the URL of the directory which the script resides in ... more
# precisely, it is the script's own URL (sans any PATH_INFO), with the name
# of the script itself, and the preceding '/', stripped off.

sub my_dir_url
{
  local ($sname) = $ENV{"SERVER_NAME"};
  local ($sport) = $ENV{"SERVER_PORT"};
  local ($hroot) = &dirstring($ENV{"SCRIPT_NAME"});

  "http://$sname:$sport$hroot"
}

# Useful string conversions...

# '&unescape' translates the %.. escapes used in URLs back into what the user
# originally typed.

# '&HTMLize' translates a random ASCII string into HTML by HTML-escaping
# stuff that might be mistaken for HTML syntax.

# '&capitalize' does the obvious.

sub unescape { local ($_) = @_; s/%(..)/pack ('H2', $1)/eg; $_ }
sub HTMLize { local ($_) = @_; s/\&/&amp;/g; s/\>/&gt;/g; s/\</&lt;/g; $_; }
sub capitalize { local($_) = @_; local ($l);
                 y/A-Z/a-z/; s+[a-z]+$l = $&; $l =~ y/a-z/A-Z/; $l+e; $_ }

# Wrestling with filenames 
# '&nodirs' strips the directories off of a pathname.
# '&dirstring' returns what &nodirs strips off (less the final '/');

sub dirstring { local ($_) = @_; s+/[^/]*$++; $_ }
sub nodirs { local ($_) = @_; s+^.*/++; $_ }

# Routines to deal with the filesystem.
# Note that most of these die if they can't do their jobs.

# Exceptions ... 'read_file and 'print_file' return distinctive comments
# if they can't get at the file, which (I expect to) show up in the final
# output...

sub create_file {
  local ($file, $contents) = @_; local (*dummy);
  open (dummy, ">$file") || &cgidie ("Couldn't create $file");
  ( (print dummy $contents) || &cgidie ("Couldn't write to $file"))
       unless ($contents eq '');
  close dummy;
}

sub copy_file {
  local ($dest, $source) = @_;
  local (*dst, *src);

  open (dst, ">$dest") || &cgidie("Couldn't write to $dest");
  open (src, "$source") || &cgidie("Couldn't read from $source");
  
  while (<src>) { (print dst) || &cgidie ("Couldn't write to $dest"); }
  close (dst); close (src);
}

sub print_file {
  local ($name) = @_; local (*file); local ($hname) = &HTMLize($name);
  if (! open (file, $name)) { print "<b>Couldn't open $hname!!!</b>"; return }
  while (<file>) { print }
  close (file)
}

sub read_file {
  local ($name) = @_; local (*file, $it); local ($hname) = &HTMLize($name);
  if (! open (file, $name)) { return "<b>Couldn't open $hname!!!</b>"; }
  $it = '';
  while (<file>) { $it .= $_ }
  close (file);
  $it
}

1;
