use strict;

use Getopt::Std;

our $opt_v;     # -v    verbose mode
getopts("v");

my @files;
if ($#ARGV >= 0) {
    @files = @ARGV;
    $opt_v = 1; # switch to verbose if files were specified
} else {
    @files = glob 'test_*.c';
}

my %groups;

#
# scan test sources
#

for my $c (@files) {

    my $group_name = substr($c, 5, -2);

    print "\n$c\n" if $opt_v;

    @{$groups{$group_name}{OBJECTS}} = ();  # start with empty OBJECTS list

    open(my $SRC, $c) or die "Can't open $c: $!\n";
    while (<$SRC>) {
        if (/\/\/\s+OBJECT\s+(\S+)/) {
            print "    OBJECT $1\n" if $opt_v;
            push @{$groups{$group_name}{OBJECTS}}, $1;
            next;
        }
        if (/^\s*((int|void)\s+((setup|teardown|test)(_\w+)?)\s*\(\s*void\s+\*\*\s*\w+\s*\))/) {
            ${$groups{$group_name}{FUNCTIONS}}{$3} = $1;
            # check if it's a test function
            if ($2 eq 'void') {
                my $name =  substr($3, 5);
                print "    TEST $name\n" if $opt_v;
                push @{$groups{$group_name}{TESTS}}, $name;
            } else {
                print "    FUNCTION $3\n" if $opt_v;
            }
            next;
        }
    } 
    close($SRC);

}

#
# generate defs.am and sources for run's
#

open(my $AM, '> defs.am') or die;

my $ts = scalar(localtime);
print $AM "# generated on $ts\n\n";

my $tests = '';
for my $g (sort keys %groups) {
    $tests .= " run_$g";
}
print $AM "TESTS =$tests\n\n";
print $AM "check_PROGRAMS =$tests\n\n";

for my $g (sort keys %groups) {
    my $objects = join(' ',@{$groups{$g}{OBJECTS}});
    print $AM <<EOT;
run_${g}_SOURCES = run_${g}.c test_${g}.c data.c functions.c
run_${g}_LDADD = $objects

EOT

    open(my $C, "> run_${g}.c") or die;
    print $C <<EOT;
// generated on $ts
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

EOT
    for my $f (sort keys %{$groups{$g}{FUNCTIONS}}) {
        print $C "${$groups{$g}{FUNCTIONS}}{$f};\n";
    }

    print $C <<EOT;

int main(void) {
    const struct CMUnitTest tests[] = {
EOT
    my ($setup, $teardown);

    for my $t (@{$groups{$g}{TESTS}}) {
        $setup = $teardown = 'NULL';
        if (defined ${$groups{$g}{FUNCTIONS}}{"setup_$t"}) {
            $setup = "setup_$t";
        } elsif (defined ${$groups{$g}{FUNCTIONS}}{setup_default}) {
            $setup = 'setup_default';
        }
        if (defined ${$groups{$g}{FUNCTIONS}}{"teardown_$t"}) {
            $teardown = "teardown_$t";
        } elsif (defined ${$groups{$g}{FUNCTIONS}}{teardown_default}) {
            $teardown = 'teardown_default';
        }
        print $C "      cmocka_unit_test_setup_teardown(test_${t}, ${setup}, ${teardown}),\n";
    }

    $setup = $teardown = 'NULL';
    if (defined ${$groups{$g}{FUNCTIONS}}{setup}) {
        $setup = 'setup';
    }
    if (defined ${$groups{$g}{FUNCTIONS}}{teardown}) {
        $teardown = 'teardown';
    }

    print $C <<EOT;
    };

    return cmocka_run_group_tests(tests, ${setup}, ${teardown});
}
EOT
    close($C);
}

close($AM);


