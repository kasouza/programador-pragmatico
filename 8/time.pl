#!/usr/bin/perl

use warnings;
use feature 'say';

if (!@ARGV) {
    say 'please gibe strin';
    exit -1
}

$_ = shift;

/^(\d\d):(\d\d)$/ && doTime($1, $2, "", 24);
/^(\d\d):(\d\d)((?:am)|(?:pm)|(?:AM)|(?:PM))$/ && doTime($1, $2, $3, 24);
/^(\d\d)((?:am)|(?:pm)|(?:AM)|(?:PM))$/ && doTime($1, 0, "", 12);
/^(\d\d)$/ && doTime($1, 0, $2, 24);
die "MORRA PERL MORRA";

sub doTime {
    my ($hour, $minutes, $ampm, $maxhour) = @_;
    if ($hour < 0) {
        die "HORA MUITO PEQUENA";
    }

    if ($hour > $maxhour) {
        die "HORA MUITO GRANDE";
    }

    if ($minutes < 0) {
        die "MINUTO MUITO PEQUENO";
    }

    if ($minutes > 59) {
        die "MINUTO MUITO GRANDE";
    }

    if ($minutes > 0) {
        say $hour, ":", $minutes, $ampm;
    } else {
        say $hour, $ampm;
    }
    exit(0);
}
