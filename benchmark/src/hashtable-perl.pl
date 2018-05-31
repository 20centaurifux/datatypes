#!/usr/bin/perl -l

use strict;

use List::Util qw(sum);
use Time::HiRes qw(gettimeofday tv_interval);


sub benchmark {
    my %map;

    open(my $file, "<", "words.txt") or die "can't open";
    while (<$file>) {
        foreach my $word(split(' ', $_)) {
            $map{$word}++;
        }
    }
    close($file);

    print scalar keys %map;
    print sum values %map;
}

my $start = [gettimeofday];
benchmark;
my $end = [gettimeofday];

print tv_interval($start, $end);
