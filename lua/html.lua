header_tmpl = [[
<html><head>
<title>SlindWeb -- __TITLE__</title>
<link rel="stylesheet" type="text/css" href="/gitweb-style.css"/>
</head><body background="banner.png">
<table><tr>
<td><a class="title" href="?act=list">All records</td>
<td><a class="title" href="?act=new">Add record</td>
<td><a class="title" href="?act=deblist">Binary package matrix</td>
<td>
 <form method="post">
  <input type="hidden" name="act" value="searchres"/>
  <input type="text" name="pkg"/>
 </form>
</td>
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
   <a href="?act=list&pkg=__UPACKAGE__">__PACKAGE__</a>
  </td>
  <td class="link">
   <a href="?act=edit&pkg=__UPACKAGE__&ver=__UVERSION__&suite=__USUITE__&arch=__UARCH__">__VERSION__</a>
  </td>
  <td class="link">
   <a href="?act=edit&pkg=__UPACKAGE__&ver=__UVERSION__&suite=__USUITE__&arch=__UARCH__">__SUITE__</a>
  </td>
  <td class="link">
   <a href="?act=edit&pkg=__UPACKAGE__&ver=__UVERSION__&suite=__USUITE__&arch=__UARCH__">__COMP__</a>
  </td>
  <td class="link">
   <a href="?act=edit&pkg=__UPACKAGE__&ver=__UVERSION__&suite=__USUITE__&arch=__UARCH__">__ARCH__</a>
  </td>
  <td class="link">
   <a href="?act=del&pkg=__UPACKAGE__&ver=__UVERSION__&suite=__USUITE__&arch=__UARCH__">delete</a>
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

error_table = {
	auth = "Authorisation required to proceed",
	inval = "Invalid parameters",
	intern = "Internal slindweb error",
	oops = "Whoops, something's wrong with this thing"
}

bail_page_tmpl = [[
  <h1>Error</h1>
  <p>__MSG__</p>
]]

function HtmlBailPage(err)
	local msg = error_table[err]
	local t = bail_page_tmpl

	t = string.gsub(t, "__MSG__", msg)

	return t
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
	t = string.gsub(t, "__UPACKAGE__", CgiEscapeSpecialChars(pkg))
	t = string.gsub(t, "__UVERSION__", CgiEscapeSpecialChars(ver))
	t = string.gsub(t, "__USUITE__", CgiEscapeSpecialChars(suite))
	t = string.gsub(t, "__UARCH__", CgiEscapeSpecialChars(arch))

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

deblist_table_header_start_tmpl = [[
<table border="0">
 <tr class="title">
  <td class="list">&nbsp;</td>
]]

deblist_table_header_cell_tmpl = [[
  <td class="list"><b>__ARCH__</b></td>
]]

deblist_table_header_end_tmpl = [[
  <td class="list">&nbsp;</td>
 </tr>
]]

deblist_table_footer_start_tmpl = [[
 <tr class="title">
  <td class="list">&nbsp;</td>
]]

deblist_table_footer_end_tmpl = [[
  <td class="list">&nbsp;</td>
 </tr>
</table>
]]

deblist_table_body_row_start_tmpl = [[
 <tr class="__CSS_CLASS__">
  <td class="link">
   <a href="?act=srcview&pkg=__UPKG__&suite=__USUITE__">__PKG__</a>
  </td>
]]

deblist_table_body_row_cell_tmpl = [[
  <td class="link">
   <a href="?act=debsrcview&pkg=__UPACKAGE__&suite=__USUITE__&arch=__UARCH__">
    <b>__VER__</b>
   </a>
  </td>
]]

deblist_table_body_row_end_tmpl = [[
 </tr>
]]

deb_list = {}

function HtmlDebListTablePkg(pkg, ver, suite, arch)
	if deb_list[pkg] == nil then
		deb_list[pkg] = {} -- = { arch = ver }
	end
	deb_list[pkg][arch] = ver

	return ver
end

function HtmlDebListTableHeader(suite)
	local arch
	local tab
	local t
	local ret

	ret = deblist_table_header_start_tmpl
	for tab, arch in ipairs(Suites[suite]["arches"]) do
		t = deblist_table_header_cell_tmpl
		ret = ret .. string.gsub(t, "__ARCH__", arch)
	end
	ret = ret .. deblist_table_header_end_tmpl

	return ret
end

function HtmlDebListTableBody(suite)
	local arch
	local pkg
	local a
	local b
	local t
	local ret = ""

	for pkg, a in pairs(deb_list) do
		t = deblist_table_body_row_start_tmpl
		if __even_row then
			t = string.gsub(t, "__CSS_CLASS__", "dark")
		else
			t = string.gsub(t, "__CSS_CLASS__", "light")
		end

		__even_row = not __even_row;
		t = string.gsub(t, "__SUITE__", suite)
		t = string.gsub(t, "__USUITE__", CgiEscapeSpecialChars(suite))
		t = string.gsub(t, "__UPKG__", CgiEscapeSpecialChars(pkg))
		ret = ret .. string.gsub(t, "__PKG__", pkg)

		for b, arch in ipairs(Suites[suite]["arches"]) do
			if deb_list[pkg][arch] == nil then
				deb_list[pkg][arch] = ""
			end

			t = deblist_table_body_row_cell_tmpl
			t = string.gsub(t, "__PACKAGE__", pkg)
			t = string.gsub(t, "__SUITE__", suite)
			t = string.gsub(t, "__ARCH__", arch)
			t = string.gsub(t, "__UPACKAGE__", CgiEscapeSpecialChars(pkg))
			t = string.gsub(t, "__USUITE__", CgiEscapeSpecialChars(suite))
			t = string.gsub(t, "__UARCH__", CgiEscapeSpecialChars(arch))
			ret = ret .. string.gsub(t, "__VER__", deb_list[pkg][arch])
		end

		ret = ret .. deblist_table_body_row_end_tmpl
	end

	return ret
end

function HtmlDebListTableFooter(suite)
	local arch
	local tab
	local t
	local ret

	ret = deblist_table_footer_start_tmpl
	for tab, arch in ipairs(Suites[suite]["arches"]) do
		t = deblist_table_header_cell_tmpl
		ret = ret .. string.gsub(t, "__ARCH__", "")
	end
	ret = ret .. deblist_table_footer_end_tmpl

	return ret
end

debsrc_header_tmpl = [[
<table>
<tr>
<td colspan="5">
<h1>Source: __PACKAGE__, suite: __SUITE__</h1>
<h2>Version: __VERSION__</h2>
<h2>Architecture: __ARCH__</h2>
<a href="http://git.slind.org/git/?p=__UPACKAGE__.git;a=summary">GitWeb</a>
</td>
</tr>
<tr><td colspan="5"><hr/></td></tr>
<tr>
<td class="title"><b>Package</b></td>
<td class="title"><b>Section</b></td>
<td class="title"><b>Size</b></td>
<td class="title"><b>MD5</b></td>
<td class="title"><b>Path</b></td>
</tr>
]]

function HtmlDebSrcHeader(pkg, version, suite, arch)
	local t = debsrc_header_tmpl

	t = string.gsub(t, "__PACKAGE__", pkg)
	t = string.gsub(t, "__UPACKAGE__", CgiEscapeSpecialChars(pkg))
	t = string.gsub(t, "__VERSION__", version)
	t = string.gsub(t, "__ARCH__", arch)
	t = string.gsub(t, "__SUITE__", suite)
	return t
end

debsrc_list_item_tmpl = [[
<tr class="__CSS_CLASS__">
<td class="link"><b><a href="?act=debcontrol&h=__DEBMD5__">__DEBNAME__</a></b></td>
<td class="link"><b>__DEBSEC__</b></td>
<td class="link"><b>__DEBSIZE__</b></td>
<td class="link"><b>__DEBMD5__</b></td>
<td class="link"><b><a href="__URL__">__DEBPATH__</a></b></td>
</tr>
]]

function HtmlDebSrcListItem(debname, debver, debpath, debsec, debsize, debmd5)
	local t = debsrc_list_item_tmpl

	if __even_row then
		t = string.gsub(t, "__CSS_CLASS__", "dark")
	else
		t = string.gsub(t, "__CSS_CLASS__", "light")
	end

	__even_row = not __even_row;
	t = string.gsub(t, "__DEBNAME__", debname)
	t = string.gsub(t, "__DEBVER__", debver)
	t = string.gsub(t, "__DEBPATH__", debpath)
	t = string.gsub(t, "__DEBSEC__", debsec)
	t = string.gsub(t, "__DEBSIZE__", debsize)
	t = string.gsub(t, "__DEBMD5__", debmd5)
	t = string.gsub(t, "__URL__", "http://ftp.slind.org/pub/SLIND/" .. debpath)

	return t
end

debsrc_footer_tmpl = [[
<tr><td colspan="5"><hr/></td></tr>
</table>
]]

function HtmlDebSrcFooter(pkg, version, suite, arch)
	local t = debsrc_footer_tmpl

	t = string.gsub(t, "__PACKAGE__", pkg)
	t = string.gsub(t, "__VERSION__", version)
	t = string.gsub(t, "__ARCH__", arch)
	t = string.gsub(t, "__SUITE__", suite)
	return t
end

function HtmlDebControlPage(control)
	return "<pre>" .. control .. "</pre>"
end

src_header_tmpl = [[
<table>
<tr>
<td colspan="5">
<h1>Source: __PACKAGE__, suite: __SUITE__</h1>
<a href="http://git.slind.org/git/?p=__PACKAGE__.git;a=summary">GitWeb</a>
</td>
</tr>
<tr>
<td><b>Architecture</b></td>
<td colspan="3"><b>Version</b></td>
<td><b>Binaries</b></td>
</tr>
]]

function HtmlSrcHeader(pkg, suite)
	local t = src_header_tmpl

	t = string.gsub(t, "__PACKAGE__", pkg)
	t = string.gsub(t, "__SUITE__", suite)
	return t
end

src_header_item_tmpl = [[
 <tr>
  <td>__ARCH__</td>
  <td colspan="3"><b>__VERSION__</td>
  <td>
   <a href="?act=debsrcview&pkg=__UPACKAGE__&suite=__USUITE__&arch=__UARCH__">__NBINS__</a>
  </td>
 </tr>
]]

function HtmlSrcItem(pkg, suite, arch, version, nbins)
	local t = src_header_item_tmpl

	t = string.gsub(t, "__PACKAGE__", pkg)
	t = string.gsub(t, "__SUITE__", suite)
	t = string.gsub(t, "__ARCH__", arch)
	t = string.gsub(t, "__UPACKAGE__", CgiEscapeSpecialChars(pkg))
	t = string.gsub(t, "__USUITE__", CgiEscapeSpecialChars(suite))
	t = string.gsub(t, "__UARCH__", CgiEscapeSpecialChars(arch))
	t = string.gsub(t, "__VERSION__", version)
	t = string.gsub(t, "__NBINS__", nbins)
	return t
end

src_footer_tmpl = [[
</table>
]]

function HtmlSrcFooter()
	local t = src_footer_tmpl

	return t
end

search_res_header_tmpl = [[
<table>
 <tr>
  <td colspan="5">
   <h1>Search results for __REQ__</h1>
  </td>
 </tr>
]]

function HtmlSearchResHeader(pkg)
	local t = search_res_header_tmpl
	t = string.gsub(t, "__REQ__", pkg)
	return t
end

search_res_item_tmpl = [[
 <tr class="__CSS_CLASS__">
  <td class="list">
   <a href="?act=srcview&pkg=__UPACKAGE__&suite=__USUITE__&version=__UVERSION__">
    __PACKAGE__
   </a>
  </td>
  <td class="list">
   __VERSION__
  </td>
  <td class="list">
   __SUITE__
  </td>
  <td class="list">
   __ARCH__
  </td>
  <td class="list">
   __COMP__
  </td>
 </tr>
]]

function HtmlSearchResItem(pkg, version, suite, arch, comp)
	local t = search_res_item_tmpl

	if __even_row then
		t = string.gsub(t, "__CSS_CLASS__", "dark")
	else
		t = string.gsub(t, "__CSS_CLASS__", "light")
	end

	__even_row = not __even_row;
	t = string.gsub(t, "__PACKAGE__", pkg)
	t = string.gsub(t, "__VERSION__", version)
	t = string.gsub(t, "__SUITE__", suite)
	t = string.gsub(t, "__UPACKAGE__", CgiEscapeSpecialChars(pkg))
	t = string.gsub(t, "__UVERSION__", CgiEscapeSpecialChars(version))
	t = string.gsub(t, "__USUITE__", CgiEscapeSpecialChars(suite))
	t = string.gsub(t, "__ARCH__", arch)
	t = string.gsub(t, "__COMP__", comp)
	return t
end

search_res_footer_tmpl = [[
</table>
]]

function HtmlSearchResFooter()
	local t = search_res_footer_tmpl
	return t
end

