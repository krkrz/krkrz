#include <windows.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
	typedef int _stdcall _export (*t_Query_sizeof_OggVorbis_File)();
	t_Query_sizeof_OggVorbis_File Query_sizeof_OggVorbis_File;
	HMODULE mod;

	if(argc < 2) return 3;

	mod = LoadLibrary(argv[1]);
	if(!mod) return 3;

	Query_sizeof_OggVorbis_File = (t_Query_sizeof_OggVorbis_File)
		GetProcAddress(mod, "Query_sizeof_OggVorbis_File");
	if(!Query_sizeof_OggVorbis_File) return 3;

	printf("%d\n", Query_sizeof_OggVorbis_File());

	return 0;
}
