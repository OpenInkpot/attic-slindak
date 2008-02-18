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
#include "slindak.h"

int dpkg_deb_b(char *path)
{
	char *argv[] = { "dpkg-deb", "-b", path, NULL };
	int ret;

	ret = spawn(DPKGDEB_BIN_PATH, argv);
	if (ret)
		return GE_ERROR;

	return GE_OK;
}

int dpkg_source_b(char *path)
{
	char *argv[] = { "dpkg-source", "-b", path, NULL };
	int ret;

	ret = spawn(DPKGSRC_BIN_PATH, argv);
	if (ret)
		return GE_ERROR;

	return GE_OK;
}

int slindak(char *path)
{
	char *argv[] = { "slindak", "-r", path, NULL };
	char slindak_path[PATH_MAX];
	char *pwd;
	int ret;

	pwd = get_current_dir_name();
	snprintf(slindak_path, PATH_MAX, "%s/src/slindak", pwd);

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

	ret = spawn(slindak_path, argv);
	if (ret)
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
	fprintf(f, "Source: %s\nSection: %s\nArchitecture: %s\n"
			"Maintainer: John Doe <johndoe@slind.org>\n\n"
			"Package: %s\n"
			"Description: a bogus package for slindak testing\n Cheers!\n",
			name, component, arch, name
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

	ret = dpkg_source_b(debdir);
	if (ret) {
		SHOUT("Failed to generate %s source package.\n", name);
		goto out_rm;
	}

	/*snprintf(binpkg, PATH_MAX, "%s.deb", pkgdir);
	slindak_i("/tmp/slindak-test", binpkg, "clydesdale");*/

out_rm:
	/*ret = rm_rf(tmpdir_r);
	if (ret) {
		SHOUT("Failed to remove %s [%d].\n", tmpdir_r, ret);
		return GE_ERROR;
	}*/

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

	ret = dpkg_deb_b(pkgdir);
	if (ret) {
		SHOUT("Failed to generate %s.deb.\n", name);
		goto out_rm;
	}

	snprintf(binpkg, PATH_MAX, "%s.deb", pkgdir);
	slindak_i("/tmp/slindak-test", binpkg, "clydesdale");

out_rm:
	ret = rm_rf(tmpdir_r);
	if (ret) {
		SHOUT("Failed to remove %s [%d].\n", tmpdir_r, ret);
		return GE_ERROR;
	}

	return GE_OK;
}

void do_pkg_tests()
{
	slindak("/tmp/slindak-test");
	mk_src_package("testpkg1", "0.1", "core", NULL);
	mk_deb_package("testpkg1", "0.1", "core", NULL);
}

