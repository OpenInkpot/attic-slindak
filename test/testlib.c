/*
 * vi: sw=4 ts=4 noexpandtab
 */
#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <sqlite3.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "slindak.h"
#include "debfile.h"

char repodir[PATH_MAX];

int slindak(char *path)
{
	char *argv[] = { "slindak", "-r", path, NULL };
	char slindak_path[PATH_MAX];
	char *pwd;
	int ret;

	pwd = get_current_dir_name();
	snprintf(slindak_path, PATH_MAX, "%s/src/slindak", pwd);
	free(pwd);

	ret = spawn(slindak_path, argv);
	if (ret)
		return GE_ERROR;

	return GE_OK;
}

int slindak_i(char *path, char *debpath, char *suite)
{
	char *argv[] = { "slindak", "-r", path, "-i", debpath, "-s", suite, NULL };
	char slindak_path[PATH_MAX];
	char *pwd;
	int ret;

	pwd = get_current_dir_name();
	snprintf(slindak_path, PATH_MAX, "%s/src/slindak", pwd);
	free(pwd);

	ret = spawn(slindak_path, argv);
	if (ret)
		return GE_ERROR;

	return GE_OK;
}

int slindak_q(char *path, char *pkgname, char *suite, char *arch, char *ver)
{
	FILE *p;
	char slindak_cmd[PATH_MAX];
	char *pwd;
	int ret;

	pwd = get_current_dir_name();
	if (arch)
		snprintf(slindak_cmd, PATH_MAX,
				"%s/src/slindak -r %s -q %s -s %s -a %s -Q %%v",
				pwd, path, pkgname, suite, arch);
	else
		snprintf(slindak_cmd, PATH_MAX,
				"%s/src/slindak -r %s -q %s -s %s -Q %%v",
				pwd, path, pkgname, suite);
	free(pwd);

	p = popen(slindak_cmd, "r");
	if (!p)
		return GE_ERROR;

	ret = fscanf(p, "%s", ver);
	pclose(p);

	if (ret <= 0)
		return GE_ERROR;

	return GE_OK;
}

int mk_src_package(
		const char *name,
		const char *version,
		const char *component,
		const char *arch
		)
{
	char *tmpdir, *pwd;
	char tmpdir_r[PATH_MAX];
	char pkgdir[PATH_MAX];
	char debdir[PATH_MAX];
	char srcpkg[PATH_MAX];
	char control[PATH_MAX];
	FILE *f;
	int ret;

	pwd = get_current_dir_name();
	snprintf(tmpdir_r, PATH_MAX, "%s/slindak-test-suite_%s_XXXXXX",
			pwd, name);
	free(pwd);

	tmpdir = mkdtemp(tmpdir_r);
	if (!tmpdir) {
		SHOUT("Failed to create temporary directory.\n", tmpdir_r);
		return GE_ERROR;
	}

	ret = chmod(tmpdir_r, 0755);
	if (ret) {
		SHOUT("Failed to chmod 0755 %s.\n", tmpdir_r);
		goto out_rm;
	}

	snprintf(pkgdir, PATH_MAX, "%s/%s-%s", tmpdir, name, version);
	snprintf(debdir, PATH_MAX, "%s/debian", pkgdir);
	ret = mkdir_p(debdir, 0755);
	if (ret) {
		SHOUT("Failed to create %s [%d].\n", debdir, ret);
		goto out_rm;
	}

	snprintf(control, PATH_MAX, "%s/control", debdir);
	f = fopen(control, "w");
	if (!f) {
		SHOUT("Failed to create debian/control file.\n");
		goto out_rm;
	}

	if (!arch) arch = "all";
	fprintf(f, "Source: %s\nSection: %s\n"
			"Maintainer: John Doe <johndoe@slind.org>\n"
			"Standards-Version: 3.6.2\n\n"
			"Package: %s\nArchitecture: %s\n"
			"Description: a bogus package for slindak testing\n Cheers!\n",
			name, component, name, arch
		   );
	fclose(f);

	snprintf(control, PATH_MAX, "%s/changelog", debdir);
	f = fopen(control, "w");
	if (!f) {
		SHOUT("Failed to create debian/changelog file.\n");
		goto out_rm;
	}

	if (!arch) arch = "all";
	fprintf(f, "%s (%s) unstable; urgency=low\n\n  * Test.\n\n"
			" -- John Doe <johndoe@slind.org>  Fri, 15 Feb 2008 18:02:18 +0300\n",
			name, version, component, arch
		   );
	fclose(f);

	mkdir_p("/tmp/slindak-test/pool/core/t/testpkg1", 0755);
	ret = dpkg_source(pkgdir, "/tmp/slindak-test/pool/core/t/testpkg1");
	if (ret) {
		SHOUT("Failed to generate %s source package.\n", name);
		goto out_rm;
	}

out_rm:
	ret = rm_rf(tmpdir_r);
	if (ret) {
		SHOUT("Failed to remove %s [%d].\n", tmpdir_r, ret);
		return GE_ERROR;
	}

	return GE_OK;
}

int mk_deb_package(
		const char *name,
		const char *version,
		const char *component,
		const char *arch
		)
{
	char *tmpdir, *pwd;
	char tmpdir_r[PATH_MAX];
	char pkgdir[PATH_MAX];
	char debdir[PATH_MAX];
	char binpkg[PATH_MAX];
	char control[PATH_MAX];
	FILE *f;
	int ret;

	pwd = get_current_dir_name();
	snprintf(tmpdir_r, PATH_MAX, "%s/slindak-test-suite_%s_XXXXXX",
			pwd, name);
	free(pwd);

	tmpdir = mkdtemp(tmpdir_r);
	if (!tmpdir) {
		SHOUT("Failed to create temporary directory.\n", tmpdir_r);
		return GE_ERROR;
	}

	ret = chmod(tmpdir_r, 0755);
	if (ret) {
		SHOUT("Failed to chmod 0755 %s.\n", tmpdir_r);
		goto out_rm;
	}

	snprintf(pkgdir, PATH_MAX, "%s/%s", tmpdir, name);
	snprintf(debdir, PATH_MAX, "%s/DEBIAN", pkgdir);
	ret = mkdir_p(debdir, 0755);
	if (ret) {
		SHOUT("Failed to create %s [%d].\n", debdir, ret);
		goto out_rm;
	}

	snprintf(control, PATH_MAX, "%s/control", debdir);
	f = fopen(control, "w");
	if (!f) {
		SHOUT("Failed to create DEBIAN/control file.\n");
		goto out_rm;
	}

	if (!arch) arch = "all";
	fprintf(f, "Package: %s\nVersion: %s\nSection: %s\nArchitecture: %s\n"
			"Maintainer: John Doe <johndoe@slind.org>\n"
			"Description: a bogus package for slindak testing\n Cheers!\n",
			name, version, component, arch
		   );
	fclose(f);

	ret = dpkg_deb(pkgdir);
	if (ret) {
		SHOUT("Failed to generate %s.deb.\n", name);
		goto out_rm;
	}

	snprintf(binpkg, PATH_MAX, "%s.deb", pkgdir);
	slindak_i(repodir, binpkg, "clydesdale");

out_rm:
	ret = rm_rf(tmpdir_r);
	if (ret) {
		SHOUT("Failed to remove %s [%d].\n", tmpdir_r, ret);
		return GE_ERROR;
	}

	return GE_OK;
}

#include "0000_base.test.c"
#include "0001_base.test.c"
#include "0002_base.test.c"
#include "0003_base.test.c"

void do_pkg_tests()
{
	strcpy(repodir, "/tmp/slindak-test");
	rm_rf(repodir);
	slindak(repodir);

	printf("TEST1: %s\n", do_test1() == GE_OK ? "OK" : "FAILED");
	printf("TEST2: %s\n", do_test2() == GE_OK ? "OK" : "FAILED");
	printf("TEST3: %s\n", do_test3() == GE_OK ? "OK" : "FAILED");
	printf("TEST4: %s\n", do_test4() == GE_OK ? "OK" : "FAILED");
}

