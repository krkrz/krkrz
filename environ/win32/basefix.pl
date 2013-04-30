
open FH, "tvpwin32.bpr";
undef $/;
$lines = <FH>;

if($lines !~ /LFLAGS value=\"-B:0x400000/)
{
	$lines =~ s/LFLAGS value=\"/LFLAGS value=\"-B:0x400000 /s;
}

open FH, ">tvpwin32.bpr";
print FH $lines;


