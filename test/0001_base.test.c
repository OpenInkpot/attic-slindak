/*
 * Basic test: updating a source package
 */
int do_test2()
{
	char ver[DF_ARCHLEN];
	int ret;

	mk_src_package("testpkg1", "0.2", "core", NULL);
	slindak(repodir);

	ret = slindak_q(repodir, "testpkg1", "clydesdale", NULL, ver);
	if (ret != GE_OK)
		return ret;

	if (strcmp(ver, "0.2"))
		return GE_ERROR;

	return GE_OK;
}
