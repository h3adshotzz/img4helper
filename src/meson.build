
#
#	Sources
#
sources = ['main.c', 'img4.c', 'devtree.c', 'darwin.c', 'asn1.c', 'lzss.c', 'sep.c']


#
#	Dependencies
#
deps = [glib_dep, lzfse_dep, cc_dep]


#
#	Binary
#
img4helper = executable('img4helper', sources, dependencies: deps, link_with: libhelper, include_directories : incdir)