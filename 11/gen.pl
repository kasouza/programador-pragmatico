#!/usr/bin/perl

use strict;
use warnings;

if (!@ARGV) {
    die "Usage: ./gen.pl input_file"
}

my $filename = $ARGV[0];

open(my $input, "<", $filename) or die $!;

$_ = <$input>;
if (!defined($_)) {
    die "wahapen";
}

my ($enum_name) = /^([a-zA-Z_]+)\n/;
if (!$enum_name) {
    die "invlaid enum nam"
}

my $lc_enum_name = lc($enum_name);
my $uc_enum_name = uc($enum_name);
my $array_name = "${uc_enum_name}\_${lc_enum_name}s[]";

my $output_c_filename = $lc_enum_name  . ".c";
my $output_h_filename = $lc_enum_name  . ".h";

open(my $output_c, ">", $output_c_filename) or die $!;
open(my $output_h, ">", $output_h_filename) or die $!;

# Header file boilerplate
print $output_h "#pragma once\n";
print $output_h "extern const char* $array_name;\n";
print $output_h "typedef enum {\n";

# Source file boilerplate
print $output_c "#include \"$output_h_filename\"\n";
print $output_c "const char* $array_name = {\n";

while (<$input>) {
    my ($enum_value) = /^([a-zA-Z_]+)\n/;
    if (!$enum_value) {
        next;
    }

    my $field_name = $uc_enum_name . "_" . $enum_value;

    # Header enum fields
    print $output_h "\t$field_name,\n";

    # Source enum field names
    print $output_c "\t[$field_name] = \"$field_name\",\n";
}

# Header file boilerplate
print $output_h "} $uc_enum_name;\n";

# Source file boilerplate
print $output_c "};";

close($input);
close($output_c);
close($output_h);
