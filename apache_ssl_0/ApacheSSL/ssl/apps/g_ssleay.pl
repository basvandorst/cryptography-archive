#!/usr/local/bin/perl

$mkprog='mklinks';
$rmprog='rmlinks';

print "#ifndef NOPROTO\n";
foreach (@ARGV)
	{ printf "extern int %s_main(int argc,char *argv[]);\n",$_; }
print "#else\n";
foreach (@ARGV)
	{ printf "extern int %s_main();\n",$_; }
print "#endif\n";


print <<'EOF';

typedef struct {
	char *name;
	int (*func)();
	} FUNCTION;

FUNCTION functions[] = {
EOF

foreach (@ARGV)
	{
	push(@files,$_);
	printf "\t{\"%s\",%s_main},\n",$_,$_;
	}

foreach ("md2","md5","sha","sha1")
	{
	push(@files,$_);
	printf "\t{\"%s\",dgst_main},\n",$_;
	}

foreach (
	"des", "des3", "idea", "rc4",
	"des-cbc","des-ede-cbc","des-ede3-cbc",
	"des-ecb","des-ede","des-ede3",
	"des-cfb","des-ede-cfb","des-ede3-cfb",
	"des-ofb","des-ede-ofb","des-ede3-ofb",
	"idea-cbc","idea-ecb","idea-cfb","idea-ofb")
	{
	push(@files,$_);
	printf "\t{\"%s\",enc_main},\n",$_;
	}

print "\t{NULL,NULL}\n\t};\n";

open(OUT,">$mkprog") || die "unable to open '$prog':$!\n";
print OUT "#!/bin/sh\nfor i in ";
foreach (@files)
	{ print OUT $_." "; }
print OUT <<'EOF';

do
echo making symlink for $i
/bin/rm -f $i
ln -s ssleay $i
done
EOF
close(OUT);
chmod(0755,$mkprog);

open(OUT,">$rmprog") || die "unable to open '$prog':$!\n";
print OUT "#!/bin/sh\nfor i in ";
foreach (@files)
	{ print OUT $_." "; }
print OUT <<'EOF';

do
echo removing $i
/bin/rm -f $i
done
EOF
close(OUT);
chmod(0755,$rmprog);
