#!/bin/sh

python XPFacer.p.py

mv npscimoz_gen.h gen
mv ISciMoz_gen.idl.fragment gen
mv ISciMoz_lite_gen.idl.fragment gen

echo "Generating ISciMoz.idl..."
perl -x $0
idl_files="ISciDoc.idl ISciMozController.idl ISciMozEvents.idl gen/ISciMoz.idl"

for idl_file in $idl_files; do
	./idl2npapi.sh $idl_file gen
done

exit 0;

#!/usr/bin/perl

open(TEMPLATE, "<ISciMoz.template.idl");
open(ISCIMOZFRAGMENT, "<gen/ISciMoz_gen.idl.fragment");
open(ISCIMOZLITEFRAGMENT, "<gen/ISciMoz_lite_gen.idl.fragment");

my $iscimozfragment = "";
my $iscimozlitefragment = "";
while (<ISCIMOZFRAGMENT>) { $iscimozfragment .= $_; }
while (<ISCIMOZLITEFRAGMENT>) { $iscimozlitefragment .= $_; }

close ISCIMOZFRAGMENT;
close ISCIMOZLITEFRAGMENT;

my $new_contents = "";
while (<TEMPLATE>) {
	if (/__ISCIMOZ_INTERFACE__/) {
		$new_contents .= $iscimozfragment;
	}
	elsif (/__ISCIMOZ_LITE_INTERFACE__/) {
		$new_contents .= $iscimozlitefragment;
	}
	else {
		$new_contents .= $_;
	}
}
close TEMPLATE;

open(ISCIMOZIDL, "+>", "gen/ISciMoz.idl");
print ISCIMOZIDL $new_contents;
close ISCIMOZIDL;
