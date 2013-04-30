$info = `svn info tvpwin32.bpr`;

if($info =~ /^Revision:\s*(\d+)/m)
{
	$rev = $1;
	undef $/;
	open FH, "tvpwin32.bpr" or die;
	$bpr = <FH>;
	$bpr =~ s/^FileVersion=(\d+\.\d+\.)\d+\.\d+/FileVersion=$1$rev.0/m;
	$bpr =~ s/^Release=\d+/Release=$rev/m;
	$bpr =~ s/^Build=\d+/Build=0/m;
	
	open FH, ">tvpwin32.bpr" or die;
	print FH $bpr;
	open FH, ">svn_revision.h" or die;
	print FH "#define TVP_SVN_REVISION TJS_W(\"$rev\")\n";
}
else
{
	print "!!!!!! WARNING !!!!!!!\n";
	print "CLI version of subversion client seems not working.\n";
	print "Revision auto-embedding will not work!\n";
}
