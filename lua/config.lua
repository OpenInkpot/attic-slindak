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
  local arches = ""
  local comps = ""

  print(">> "..i.." : ");
  for k, l in ipairs(j["arches"]) do
    arches = arches .. l;
  end

  for k, l in ipairs(j["components"]) do
    comps = comps .. l;
  end

  SuiteAdd(i, arches, comps)
end

