/*
 * Basic test: adding a source package
 */
int do_test6()
{
	int i;
	char pkgname[40];

	for (i = 0; i < 100; i++) {
		snprintf(pkgname, 40, "test-package-%05d", i);
		mk_src_package(pkgname, "0.1", "debug", NULL);
	}

	slindak(repodir);

	for (i = 0; i < 100; i++) {
		snprintf(pkgname, 40, "test-package-%05d", i);
		mk_deb_package(pkgname, "0.1", "debug", "uclibc-arm");
	}

	slindak(repodir);

	return GE_OK;
}
