#!/usr/local/bin/perl

@files=("crypto/idea/i_ecb.c",
	"crypto/lhash/lhash.c",
	"crypto/md/md2_dgst.c",
	"crypto/md/md5_dgst.c",
	"crypto/pem/pem_lib.c",
	"crypto/bn/bn_lib.c",
	"crypto/dh/dh_lib.c",
	"crypto/rc4/rc4_enc.c",
	"crypto/rsa/rsa_lib.c",
	"crypto/sha/sha1dgst.c",
	"crypto/sha/sha_dgst.c",
	"crypto/asn1/asn1_lib.c",
	"crypto/x509/x509_vrf.c",
	"crypto/evp/evp_enc.c",
	"crypto/rand/md5_rand.c",
	"crypto/stack/stack.c",
	"crypto/txt_db/txt_db.c",
	"ssl/ssl_lib.c",
	"README",
	);

@a=localtime(time());
$time=sprintf("%02d/%02d/%02d",$a[3],$a[4]+1,$a[5]);

$ver=$ARGV[0];
($ver ne "") || die "no version number specified\n";

foreach $file (@files)
	{
	open(IN,"<$file") || die "unable to open $file:$!\n";
	open(OUT,">$file.new") || die "unable to open $file.new:$!\n";

	while (<IN>)
		{
		if (s/(SSLeay v )[^"]*(.)/\1$ver $time\2/)
			{
			print OUT;
			while (<IN>) { print OUT; }
			last;
			}
		print OUT;
		}
	close(IN);
	close(OUT);
	rename($file,"$file.old") || die "unable to rename $file:$!\n";
	rename("$file.new",$file) || die "unable to rename $file.new:$!\n";
	}
