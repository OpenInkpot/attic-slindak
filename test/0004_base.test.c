/*
 * Basic test: adding a source package
 */
int do_test5()
{
	mk_deb_package("testpkg1", "0.2", "core", "mips");
	slindak(repodir);

	return GE_OK;
}
