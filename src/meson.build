
#
#	Sources
#
sources = ['main.c', 'img4.c', 'devtree.c', 'asn1.c']


#
#	Dependencies
#
deps = [cc_dep]


#
#	Binary
#
img4helper_inc = include_directories ('../include/')
img4helper = executable('img4helper', sources, dependencies: deps, link_with: libhelper_static, include_directories : [incdir, img4helper_inc])