Config = {
  repo_dir = "/opt/slind/SLIND",
  devel_suite = "clydesdale",
}

Suites = {
  clydesdale = {
    arches = { "i386", "powerpc", "arm", "mips",
	    "mipsel", "sh4a", "uclibc-mips",
	    "uclibc-mipsel", "uclibc-powerpc",
	    "uclibc-i386", "uclibc-arm",
	    "uclibc-sh4a" },
    components = { "broken", "host-tools", "core",
	    "gui", "security", "debug" },
  },
  percheron = {
    arches = { "i386", "powerpc", "arm", "mips",
	    "mipsel", "sh4a", "uclibc-mips",
	    "uclibc-mipsel", "uclibc-powerpc",
	    "uclibc-i386", "uclibc-arm",
	    "uclibc-sh4a" },
    components = { "broken", "host-tools", "core",
	    "gui", "security", "debug" },
  },
  attic = {
    arches = { "i386", "powerpc", "arm", "mips",
	    "mipsel", "sh4a", "uclibc-mips",
	    "uclibc-mipsel", "uclibc-powerpc",
	    "uclibc-i386", "uclibc-arm",
	    "uclibc-sh4a" },
    components = { "broken", "host-tools", "core",
	    "gui", "security", "debug" },
  }
}

-- SuiteAdd("suffolk", "i386 arm powerpc", "main");

for i, j in pairs(Suites) do
  local arches = table.concat(j["arches"], " ")
  local comps = table.concat(j["components"], " ")

  SuiteAdd(i, arches, comps)
end

function RenderListFileName(suite, arch, components)
  return (
    Config["repo_dir"] ..
    "/indices/" .. suite .. "_" .. components .. "_" .. arch .. ".list")
end

function RenderSrcListFileName(suite, arch, components)
  return (
    Config["repo_dir"] ..
    "/indices/" .. suite .. "_" .. components .. ".src.list")
end

