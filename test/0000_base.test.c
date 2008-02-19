/*
 * Basic test: adding a source package
 */
int do_test1()
{
	char ver[DF_ARCHLEN];
	int ret;

	mk_src_package("testpkg1", "0.1", "core", NULL);
	slindak(repodir);

	ret = slindak_q(repodir, "testpkg1", "clydesdale", NULL, ver);
	if (ret != GE_OK)
		return ret;

	if (strcmp(ver, "0.1"))
		return GE_ERROR;

	return GE_OK;
}
