#!/usr/bin/perl

use strict;
use warnings;
use FindBin qw($Bin);
use File::Basename;

my $homework_folder = $Bin;
my $parent_dir = dirname($homework_folder);
my $bin_dir = "${homework_folder}/bin";
my $src_dir = "${homework_folder}/src";

die "Usage: $0 <-b | -s>\n" unless @ARGV == 1;

my $option = shift;
if ($option eq '-b') {
    print "Binary release requested. Proceed? (Y/N): ";
    my $confirm = <STDIN>;
    chomp $confirm;
    if ($confirm =~ /^[Yy]$/) {
        print "Enter hostname: ";
        my $hostname = <STDIN>;
        chomp $hostname;
        my $tar_file = "${homework_folder}_${hostname}.tar"; 
        chdir($parent_dir) or die "Cannot chdir to $parent_dir: $!";
        system("tar -cf \"$tar_file\" -C \"$parent_dir\" homework4/src/bin");

        print "Binary release created: $tar_file\n";
    } else {
        print "Binary release cancelled.\n";
    }
}

elsif ($option eq '-s') {
    print "Source release requested. Proceed? (Y/N): ";
    my $confirm = <STDIN>;
    chomp $confirm;
    if ($confirm =~ /^[Yy]$/) {
        system("make clean");
        my $tar_file = "${homework_folder}.tar";
        chdir($parent_dir) or die "Cannot chdir to $parent_dir: $!";
        system("tar -cf $tar_file -C $parent_dir homework4");
        print "Source release created: $tar_file\n";
    } else {
        print "Source release cancelled.\n";
    }
}

else {
    die "Invalid option. Use -b for binary release or -s for source release.\n";
}
