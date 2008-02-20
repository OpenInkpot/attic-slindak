/*
 * Basic test: adding a source package
 */
int do_test4()
{
	char ver[DF_ARCHLEN];
	int ret;

	mk_src_package("testpkg2", "0.2.0.2", "core", "i386 arm powerpc");
	slindak(repodir);

	ret = slindak_q(repodir, "testpkg2", "clydesdale", "arm", ver);
	if (ret != GE_OK)
		return ret;

	if (strcmp(ver, "0.1.0.1"))
		return GE_ERROR;

	return GE_OK;
}
