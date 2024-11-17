#!/usr/bin/perl

use strict;
use warnings;
use File::Find;
use File::Copy;

if (!@ARGV) {
    die "gibe input folder";
}

my $dir = $ARGV[0];
my @files;

find(sub {
    return unless -f;
    return unless /\.pl$/;
    push @files, $File::Find::name;
}, $dir);

foreach (@files) {
    print "$_\n";
    open(my $fh, "<", $_) or die "invalid fail $_\n";
    my $content = do { local $/; <$fh> };

    close $fh;

    next if ($content =~ /^use strict;$/m);

    rename($_, "$_.bkp");

    open(my $out, ">", $_) or die "faeled $_\n";

    $content =~ s/^(?!#)/\nuse strict;\n/m;
    print $out $content;
    close($out);
}
