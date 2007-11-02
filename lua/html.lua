header_tmpl = [[
<html><head>
<title>SlindWeb -- __TITLE__</title>
<link rel="stylesheet" type="text/css" href="/gitweb-style.css"/>
</head><body>
<table><tr>
<td><a class="title" href="?act=list">All records</td>
<td><a class="title" href="?act=new">Add record</td>
</tr></table>
]]

footer_tmpl = [[
<p>__DATE__</p></body></html>
]]

list_table_header_tmpl = [[
<table border="0">
 <tr class="title">
  <td class="list"><b>pkgname</b> <a href="?act=list&order=pkgname">V</a></td>
  <td class="list"><b>version</b> <a href="?act=list&order=version">V</a></td>
  <td class="list"><b>suite</b> <a href="?act=list&order=suite">V</a></td>
  <td class="list"><b>component</b> <a href="?act=list&order=component">V</a></td>
  <td class="list"><b>arch</b> <a href="?act=list&order=arch">V</a></td>
  <td class="list">&nbsp;</td>
 </tr>
]]

__even_row = 0
list_table_row_tmpl = [[
 <tr class="__CSS_CLASS__">
  <td class="link">
   <a href="?act=list&pkg=__PACKAGE__">__PACKAGE__</a>
  </td>
  <td class="link">
   <a href="?act=edit&pkg=__PACKAGE__&ver=__VERSION__&suite=__SUITE__&arch=__ARCH__">__VERSION__</a>
  </td>
  <td class="link">
   <a href="?act=edit&pkg=__PACKAGE__&ver=__VERSION__&suite=__SUITE__&arch=__ARCH__">__SUITE__</a>
  </td>
  <td class="link">
   <a href="?act=edit&pkg=__PACKAGE__&ver=__VERSION__&suite=__SUITE__&arch=__ARCH__">__COMP__</a>
  </td>
  <td class="link">
   <a href="?act=edit&pkg=__PACKAGE__&ver=__VERSION__&suite=__SUITE__&arch=__ARCH__">__ARCH__</a>
  </td>
  <td class="link">
   <a href="?act=del&pkg=__PACKAGE__&ver=__VERSION__&suite=__SUITE__&arch=__ARCH__">delete</a>
  </td>
 </tr>
]]

list_table_footer_tmpl = [[
</table>
]]

edit_form_tmpl = [[
<h1>Edit __PACKAGE__ (__VERSION__)</h1>
<form>
 <input type="hidden" name="pkg" value="__PACKAGE__"/>
 <input type="hidden" name="ver" value="__VERSION__"/>
 <input type="hidden" name="arch" value="__ARCH__"/>
 <input type="hidden" name="suite" value="__SUITE__"/>
 <input type="hidden" name="debug" value="yes"/>
 <input type="hidden" name="act" value="commit"/>
 <table>
  <tr>
   <td>Package name</td>
   <td>__PACKAGE__</td>
  </tr>
  <tr>
   <td>Version</td>
   <td><input name="set_version" type="text" value="__VERSION__"></td>
  </tr>
  <tr>
   <td>Suite</td>
   <td>
    <select name="set_suite">
__SUITES__
    </select>
   </td>
  </tr>
  <tr>
   <td>Component</td>
   <td>
    <select name="set_component">
__COMPS__
    </select>
   </td>
  </tr>
  <tr>
   <td>Architecture</td>
   <td>
    <select name="set_arch">
__ARCHES__
    </select>
   </td>
  </tr>
  <tr><td></td><td><input type="submit"/></td></tr>
 </table>
</form>
]]

new_form_tmpl = [[
<h1>New package record</h1>
<form>
 <input type="hidden" name="debug" value="yes"/>
 <input type="hidden" name="act" value="commit"/>
 <table>
  <tr>
   <td>Package name</td>
   <td><input name="pkg" type="text"/></td>
  </tr>
  <tr>
   <td>Version</td>
   <td><input name="set_version" type="text"></td>
  </tr>
  <tr>
   <td>Suite</td>
   <td>
    <select name="set_suite">
__SUITES__
    </select>
   </td>
  </tr>
  <tr>
   <td>Component</td>
   <td>
    <select name="set_component">
__COMPS__
    </select>
   </td>
  </tr>
  <tr>
   <td>Architecture</td>
   <td>
    <select name="set_arch">
__ARCHES__
    </select>
   </td>
  </tr>
  <tr><td></td><td><input type="submit"/></td></tr>
 </table>
</form>
]]

select_opt_tmpl = [[
     <option value="__OPT__"__SEL__>__OPT__</option>
]]

function HtmlHeader(title)
	return string.gsub(header_tmpl, "__TITLE__", title)
end

function HtmlFooter()
	return string.gsub(footer_tmpl, "__DATE__", BUILD_DATE)
end

function HtmlListTableHeader()
	return list_table_header_tmpl
end

function HtmlListTableRow(pkg, ver, suite, arch, comp)
	local t = list_table_row_tmpl

	if __even_row then
		t = string.gsub(t, "__CSS_CLASS__", "dark")
	else
		t = string.gsub(t, "__CSS_CLASS__", "light")
	end

	__even_row = not __even_row;
	t = string.gsub(t, "__PACKAGE__", pkg)
	t = string.gsub(t, "__VERSION__", ver)
	t = string.gsub(t, "__SUITE__", suite)
	t = string.gsub(t, "__COMP__", comp)
	t = string.gsub(t, "__ARCH__", arch)

	return t
end

function HtmlListTableFooter()
	return list_table_footer_tmpl
end

function HtmlEditPage(pkg, ver, suite, comp, arch)
	local t = edit_form_tmpl
	local st=""
	local ct=""
	local at=""
	local s
	local tab
	local comps={}
	local arches={}
	local c, i

	-- dropdown list with suites
	for s, tab in pairs(Suites) do

		st = st .. string.gsub(select_opt_tmpl, "__OPT__", s)
		if suite == s then
			st = string.gsub(st, "__SEL__", " selected")
		else
			st = string.gsub(st, "__SEL__", "")
		end

		for i, c in ipairs(tab["components"]) do
			if c == comp then
				comps[c] = " selected"
			else
				comps[c] = ""
			end
		end

		if arch == "" then
			arches["ALL"] = " selected"
		else
			arches["ALL"] = ""
		end

		for i, c in ipairs(tab["arches"]) do
			if c == arch then
				arches[c] = " selected"
			else
				arches[c] = ""
			end
		end
	end

	-- dropdown list with components
	for c, i in pairs(comps) do
		ct = ct .. string.gsub(select_opt_tmpl, "__OPT__", c)
		ct = string.gsub(ct, "__SEL__", i)
	end

	-- dropdown list with architectures
	for c, i in pairs(arches) do
		at = at .. string.gsub(select_opt_tmpl, "__OPT__", c)
		at = string.gsub(at, "__SEL__", i)
	end

	t = string.gsub(t, "__PACKAGE__", pkg)
	t = string.gsub(t, "__VERSION__", ver)
	t = string.gsub(t, "__SUITE__", suite)
	t = string.gsub(t, "__ARCH__", arch)
	t = string.gsub(t, "__SUITES__", st)
	t = string.gsub(t, "__COMPS__", ct)
	t = string.gsub(t, "__ARCHES__", at)

	return t
end

function HtmlNewPage()
	local t = new_form_tmpl
	local st=""
	local ct=""
	local at=""
	local s
	local tab
	local comps={}
	local arches={}
	local c, i

	-- dropdown list with suites
	for s, tab in pairs(Suites) do

		st = st .. string.gsub(select_opt_tmpl, "__OPT__", s)
		st = string.gsub(st, "__SEL__", "")

		for i, c in ipairs(tab["components"]) do
			comps[c] = ""
		end

		arches["ALL"] = " selected"

		for i, c in ipairs(tab["arches"]) do
			arches[c] = ""
		end
	end

	-- dropdown list with components
	for c, i in pairs(comps) do
		ct = ct .. string.gsub(select_opt_tmpl, "__OPT__", c)
		ct = string.gsub(ct, "__SEL__", "")
	end

	-- dropdown list with architectures
	for c, i in pairs(arches) do
		at = at .. string.gsub(select_opt_tmpl, "__OPT__", c)
		at = string.gsub(at, "__SEL__", "")
	end

	t = string.gsub(t, "__SUITES__", st)
	t = string.gsub(t, "__COMPS__", ct)
	t = string.gsub(t, "__ARCHES__", at)

	return t
end

