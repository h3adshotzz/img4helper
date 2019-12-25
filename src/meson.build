
#
#	Sources
#
sources = ['main.c', 'img4.c', 'devtree.c', 'darwin.c', 'asn1.c', 'lzss.c']


#
#	Dependencies
#
deps = [lzfse_dep, cc_dep]


#
#	Binary
#
img4helper = executable('img4helper', sources, dependencies: deps, link_with: libhelper_static, include_directories : incdir)