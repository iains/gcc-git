## Process this file with automake to produce Makefile.in.

AUTOMAKE_OPTIONS = foreign dejagnu

# May be used by various substitution variables.
gcc_version := $(shell @get_gcc_base_ver@ $(top_srcdir)/../gcc/BASE-VER)

EXPECT = $(shell if test -f $(top_builddir)/../expect/expect; then \
	   echo $(top_builddir)/../expect/expect; else echo expect; fi)

_RUNTEST = $(shell if test -f $(top_srcdir)/../dejagnu/runtest; then \
	     echo $(top_srcdir)/../dejagnu/runtest; else echo runtest; fi)
RUNTEST = $(_RUNTEST) $(AM_RUNTESTFLAGS)

cxxabi_name.exp: stamp-cxxabi-name

stamp-cxxabi-name:
	@if [ "X$(CXX_ABI_LIB_NAME)" != "X" ] ; then \
	  echo "set cxx_abi_lib_name \"$(CXX_ABI_LIB_NAME)\"" >cxxabi_name.exp; \
	else \
	  echo "#  cxx_abi_lib_name is not set" >cxxabi_name.exp; \
	fi
	echo `date` > stamp-cxxabi-name

EXTRA_DEJAGNU_SITE_CONFIG = cxxabi_name.exp
