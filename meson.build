project ('img4helper', 'c',
		 version: '0.1',
		 default_options: [
			'werror=true',
			'warning_level=2',
			'c_std=c11'
		])

#
#	Allow unused variables but still show a warning
#
add_project_arguments('-Wno-error=unused-variable', language : 'c')
add_project_arguments('-Wno-error=unused-function', language : 'c')
add_project_arguments('-Wno-error=unused-parameter', language : 'c')
add_project_arguments('-Wno-error=int-conversion', language : 'c')
add_project_arguments('-Wno-error=missing-field-initializers', language : 'c')
add_project_arguments('-Wno-error=sign-compare', language : 'c')
add_project_arguments('-Wno-error=int-conversion', language : 'c')
add_project_arguments('-Wno-error=incompatible-pointer-types', language : 'c')
add_project_arguments('-Wno-error=format', language : 'c')


#
#	Configuration header
#
config_h = configuration_data()
config_h.set_quoted('PACKAGE_VERSION', meson.project_version())
if get_option('buildtype') == 'debug'
config_h.set('DEBUG', 0)
endif
configure_file(
  output: 'config.h',
  configuration: config_h,
)


#
#	Dependencies
#
lzfse_dep = dependency('lzfse')
openssl_dep = dependency('openssl')
cc_dep = dependency('cryptokit')


#
#	Sub directories
#
subdir('libhelper/src')
subdir('src')