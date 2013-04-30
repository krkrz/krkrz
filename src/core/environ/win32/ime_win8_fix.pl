#!/usr/bin/perl


# very nasty way to avoid win8's Win32NLSEnableIME(Handle, TRUE).
# The following codes in VCL;
#  if SysLocale.FarEast and (Win32Platform = VER_PLATFORM_WIN32_NT) then
#    ImeMode := imDisable;
# are to be modified as below by this program;
#  if SysLocale.FarEast and (Win32Platform = VER_PLATFORM_WIN32_NT) then
#    ImeMode := imClose;
# Although Borland C++ comes with its source code, we modify its
# binary code rather than source code, for maintenancebility.
$filename = $ARGV[0];

open FH, $filename  or die;
binmode FH;
read (FH, $binary, (-s $filename ));
close FH;

$re = "
\xa1 ....               (?# mov eax, ptr SysLocale     )
\x80 \x78 \x08 \x00     (?# byte ptr [eax + 8], 0      )
\x74 \x11               (?# jz +11                     )
\xa1 ....               (?# mov eax, ptr Win32Platform )
\x83 \x38 \x02          (?# cmp dword ptr [eax], 2     )
\x75 \x07               (?# jnz +7                     )
\xc6 \x86 ....          (?# mov bytr ptr [esi+ ?], 0x00)
"; # and last one byte follows (which is a value to be assigned to ImeMode)

# sanity check
# above re must match strictly less than twice.
# In stdctrls.pas, above code appears twice, but
# TScrollBar is not linked with normal kirikiri2 binary.

$count = 0;
map {++$count} ($binary =~ /$re\x00/gx);

if($count > 2)
{
	print "====== Win8 ime fix failed! Consult with W.Dee!!! ======\n";
	print "count : $count \n";
	exit(3);
}

# replace the code

$binary =~ s/($re)\x00/\1\x01/gxs; # last \x01 = imClose

open FH, ">$filename"  or die;
binmode FH;
print FH $binary;
close FH;

print $filename ;
print "\n";
print "patched location count : $count ";
