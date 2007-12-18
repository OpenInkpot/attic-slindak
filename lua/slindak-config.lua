Config = {
  repo_dir = "/opt/slind/SLIND.bogus",
  devel_suite = "clydesdale",
  attic_suite = "attic",
}

Suites = {
  clydesdale = {
    arches = { "i386", "powerpc", "arm", "mips",
	    "mipsel", "sh4a", "uclibc-mips",
	    "uclibc-mipsel", "uclibc-powerpc",
	    "uclibc-i386", "uclibc-arm",
	    "uclibc-sh4a" },
    components = { "broken", "host-tools", "core",
	    "gui", "security", "debug", "doc", "libdevel" },
  },
  percheron = {
    arches = { "i386", "powerpc", "arm", "mips",
	    "mipsel", "sh4a", "uclibc-mips",
	    "uclibc-mipsel", "uclibc-powerpc",
	    "uclibc-i386", "uclibc-arm",
	    "uclibc-sh4a" },
    components = { "broken", "host-tools", "core",
	    "gui", "security", "debug", "doc", "libdevel" },
  },
  attic = {
    arches = { "i386", "powerpc", "arm", "mips",
	    "mipsel", "sh4a", "uclibc-mips",
	    "uclibc-mipsel", "uclibc-powerpc",
	    "uclibc-i386", "uclibc-arm",
	    "uclibc-sh4a" },
    components = { "broken", "host-tools", "core",
	    "gui", "security", "debug", "doc", "libdevel" },
  }
}

SuiteAdd("clydesdale")
SuiteAdd("percheron")
SuiteAdd("attic")

function RenderListFileName(suite, arch, components)
  return (
    Config["repo_dir"] ..
    "/indices/" .. suite .. "_" .. components .. "_" .. arch .. ".list")
end

function RenderSrcListFileName(suite, components)
  return (
    Config["repo_dir"] ..
    "/indices/" .. suite .. "_" .. components .. ".src.list")
end

