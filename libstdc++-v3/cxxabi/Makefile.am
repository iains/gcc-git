## Makefile to allow use of c++abi as the support library.

include $(top_srcdir)/fragment.am

# A shim library to support the use of an external libc++abi DSO.
# We do not support incorporating the c++abi sources into libstdc++ (it
# does not seem to be useful and the sources are licensed differently).

noinst_LTLIBRARIES = libc++abiconvenience.la

std_HEADERS = \
	compare cxxabi.h exception initializer_list new typeinfo

bits_HEADERS = \
	atomic_lockfree_defines.h cxxabi_forced.h cxxabi_init_exception.h \
	hash_bytes.h exception.h exception_ptr.h nested_exception.h \
	exception_defines.h

headers = $(std_HEADERS) $(bits_HEADERS)

sources = \
	shim.cc \
	exception_ptr.cc \
	hash_bytes.cc \
	missing-fundamental-typeinfo.S

libc__abiconvenience_la_SOURCES = $(sources)

# AM_CXXFLAGS needs to be in each subdirectory so that it can be
# modified in a per-library or per-sub-library way.  Need to manually
# set this option because CONFIG_CXXFLAGS has to be after
# OPTIMIZE_CXXFLAGS on the compile line so that -O2 can be overridden
# as the occasion call for it.
AM_CXXFLAGS = \
	$(glibcxx_lt_pic_flag) $(glibcxx_compiler_shared_flag) \
	$(XTEMPLATE_FLAGS) $(FREESTANDING_FLAGS) $(EH_POOL_FLAGS) \
	$(WARN_CXXFLAGS) $(OPTIMIZE_CXXFLAGS)  $(CONFIG_CXXFLAGS)

AM_MAKEFLAGS = \
	"gxx_include_dir=$(gxx_include_dir)"


# Use special rules for pulling things out of libiberty.  These
# objects should be compiled with the "C" compiler, not the C++
# compiler, and also should not use the C++ includes.
C_INCLUDES = -I.. -I$(toplevel_srcdir)/libiberty -I$(toplevel_srcdir)/include
C_COMPILE = \
	$(CC) $(DEFS) $(C_INCLUDES) \
	$(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CFLAGS) $(CFLAGS) $(EXTRA_CFLAGS)

# LTCOMPILE is copied from LTCXXCOMPILE below.
LTCOMPILE = $(LIBTOOL) --tag CC $(LIBTOOLFLAGS) --mode=compile \
	    $(CC) $(DEFS) $(C_INCLUDES) \
	    $(glibcxx_lt_pic_flag) $(glibcxx_compiler_shared_flag) \
	    $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CFLAGS) $(CFLAGS) $(EXTRA_CFLAGS)


LTCXXCOMPILE = \
	$(LIBTOOL) --tag CXX \
	$(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) \
	--mode=compile $(CXX) $(TOPLEVEL_INCLUDES) \
	$(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS) $(EXTRA_CXX_FLAGS)

LTLDFLAGS = $(shell $(SHELL) $(top_srcdir)/../libtool-ldflags $(LDFLAGS))

# 3) We'd have a problem when building the shared libstdc++ object if
# the rules automake generates would be used.  We cannot allow g++ to
# be used since this would add -lstdc++ to the link line which of
# course is problematic at this point.  So, we get the top-level
# directory to configure libstdc++-v3 to use gcc as the C++
# compilation driver.
CXXLINK = \
	$(LIBTOOL) --tag CXX \
	$(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) \
	--mode=link $(CXX) \
	$(OPT_LDFLAGS) $(SECTION_LDFLAGS) $(AM_CXXFLAGS) $(LTLDFLAGS) -o $@

# Install notes
# We have to have rules modified from the default to counteract SUN make
# prepending each of $(*_HEADERS) with VPATH below.
stddir = $(gxx_include_dir)
bitsdir = $(gxx_include_dir)/bits

shim.lo: shim.cc
	$(LTCXXCOMPILE) -std=gnu++11 -c $<
shim.o: shim.cc
	$(CXXCOMPILE) -std=gnu++11 -c $<

hash_bytes.lo: hash_bytes.cc
	$(LTCXXCOMPILE) -std=gnu++11 -c $<
hash_bytes.o: hash_bytes.cc
	$(CXXCOMPILE) -std=gnu++11 -c $<

missing-fundamental-typeinfo.S: missing-fti-arm64.h missing-fti-x86_64.h

missing-fundamental-typeinfo.lo: missing-fundamental-typeinfo.S
	$(LTCXXCOMPILE) -c -xassembler-with-cpp $<
missing-fundamental-typeinfo.o: missing-fundamental-typeinfo.S
	$(CXXCOMPILE) -c -xassembler-with-cpp $<

install-stdHEADERS: $(std_HEADERS)
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(DESTDIR)$(stddir)
	@list='$(std_HEADERS)'; for p in $$list; do \
	  q=`echo $$p | sed -e 's,.*/,,'`; \
	  if test -f "$$p"; then d= ; else d="$(srcdir)/"; fi; \
	  echo " $(INSTALL_DATA) $$d$$p $(DESTDIR)$(stddir)/$$q"; \
	  $(INSTALL_DATA) $$d$$p $(DESTDIR)$(stddir)/$$q; \
	done

install-bitsHEADERS: $(bits_HEADERS)
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(DESTDIR)$(bitsdir)
	@list='$(bits_HEADERS)'; for p in $$list; do \
	  q=`echo $$p | sed -e 's,.*/,,'`; \
	  if test -f "$$p"; then d= ; else d="$(srcdir)/"; fi; \
	  echo " $(INSTALL_DATA) $$d$$p $(DESTDIR)$(bitsdir)/$$q"; \
	  $(INSTALL_DATA) $$d$$p $(DESTDIR)$(bitsdir)/$$q; \
	done

uninstall-stdHEADERS:
	@$(NORMAL_UNINSTALL)
	list='$(std_HEADERS)'; for p in $$list; do \
	  q=`echo $$p | sed -e 's,.*/,,'`; \
	  rm -f $(DESTDIR)$(stddir)/$$q; \
	done

uninstall-bitsHEADERS:
	@$(NORMAL_UNINSTALL)
	list='$(bits_HEADERS)'; for p in $$list; do \
	  q=`echo $$p | sed -e 's,.*/,,'`; \
	  rm -f $(DESTDIR)$(bitsdir)/$$q; \
	done

# By adding these files here, automake will remove them for 'make clean'
CLEANFILES = stamp-*

