#!/usr/bin/env perl
# Copyright (C) 2023 Free Software Foundation, Inc.
# This file is part of LibreDWG.

use strict;
use warnings;
use File::Basename;
my $in = shift;
open my $F, $in or die;

$in = basename $in, '.TXT';
# $in =~ s/\.TXT$//;
$in =~ s/-/_/g;
$in = lc $in;
print "/* ex: set ro ft=c: -*- mode: c; buffer-read-only: t -*- */\n";
print "// Copyright (C) 2023 Free Software Foundation, Inc.\n";
print "// This file is part of LibreDWG.\n";
print "// Generated by txt2h.pl.\n\n";
print "#include <stdint.h>\n\n";
print "static const uint16_t cptbl_$in\[\] = {\n";

my ($los, $his, $lod, $hid, @out, @exclow) = (0xffff, 0, 0xffff, 0);
LINE:
    while (<$F>) {
        /^#/ and next;
            if (/^(0x[0-9a-fA-F]+)\s+(0x[0-9a-fA-F]+)(.*)/) {
                my $src = $1;
                my $d = $2;
                my $cmt = $3;
                my $sn = hex($src);
                my $dn = hex($d);
                $cmt =~ s/\t#//;
                if ($cmt) {
                    $cmt = "\t// $cmt";
                }
                if ($sn <= 0x7f and $sn != $dn) {
                    # exceptions
                    if (($in eq 'johab' and $src eq '0x5C') or
                        ($in eq 'cp864' and $src eq '0x25'))
                    {
                        warn "$in: $src != $d\n";
                        push @out, "  [$src] = $d,$cmt\n";
                        push @exclow, "  [$src] = $d,$cmt\n";
                    } else {
                        die "$in: $src != $d";
                    }
                }
                $los = $sn if $sn < $los;
                $his = $sn if $sn > $his ;
                $lod = $dn if $dn < $lod;
                $hid = $dn if $dn > $hid;
                push @out, "  [$src] = $d,$cmt\n" if $sn > 0x7f;
        }
}

print "  [0x0] = $his,\t// size of vector\n";
print $_ for @out;
print "};\n";
print "#define MIN_", uc $in, "\t$los\n";
print "#define MAX_", uc $in, "\t$his\n";
print "#define MIN_", uc $in, "_UC\t$los\n";
print "#define MAX_", uc $in, "_UC\t$his\n";
if (@exclow) {
    print "static const uint16_t cptbl_exc_$in\[\] = {\n";
    print $_ for @exclow;
    print "};\n";
}