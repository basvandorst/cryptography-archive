BEGIN { FS = "." }
/WINVN_VERSION/ {
  ignore = $1;
  major = $2;
  minor = $3;
  printf ("#define WINVN_VERSION \"WinVN 0.%s.%s\"\n", $2, $3+1);
}
