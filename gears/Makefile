# Copyright 2005, Google Inc.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
#  1. Redistributions of source code must retain the above copyright notice,
#     this list of conditions and the following disclaimer.
#  2. Redistributions in binary form must reproduce the above copyright notice,
#     this list of conditions and the following disclaimer in the documentation
#     and/or other materials provided with the distribution.
#  3. Neither the name of Google Inc. nor the names of its contributors may be
#     used to endorse or promote products derived from this software without
#     specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
# WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
# EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
# OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# Delete all default suffixes as we don't need this feature.
# This reduces the time taken for make to parse VPATH files by 80%.
.SUFFIXES:

include tools/version.mk
include tools/config.mk
include tools/sanity.mk


INCLUDES += -I../plugin/titanium_plugin/scintilla -DSCI_NAMESPACE -DSCI_OWNREGEX
LIBS += -L../plugin/titanium_plugin/scintilla -lscintilla
EXEFLAGS += -L../plugin/titanium_plugin/scintilla -lscintilla

MODULE		= $(SHORT_NAME)

#-----------------------------------------------------------------------------
# third_party/breakpad

ifeq ($(OS),win32)

ifeq ($(BROWSER),NPAPI)


# Disable breakpad for Chrome, since it has its own.
BREAKPAD_VPATH += \
		../third_party/breakpad/src/client \
		$(NULL)

BREAKPAD_CPPSRCS += \
		exception_handler_stub.cc \
		$(NULL)

else

BREAKPAD_VPATH	+= \
		../third_party/breakpad/src/client/windows/handler \
		../third_party/breakpad/src/common/windows \
		$(NULL)

BREAKPAD_CPPSRCS	+= \
		exception_handler.cc \
		guid_string.cc \
		$(NULL)


FF3_VPATH	+= \
		base/common \
		$(NULL)

FF3_CPPSRCS	+= \
		exception_handler_win32.cc \
		$(NULL)


IE_VPATH	+= \
		base/common \
		$(NULL)

IE_CPPSRCS	+= \
		exception_handler_win32.cc \
		$(NULL)

endif # BROWSER != NPAPI
endif # OS == win32

#-----------------------------------------------------------------------------
# third_party/breakpad_osx

ifeq ($(OS),osx)
BREAKPAD_VPATH	+= \
		../third_party/breakpad_osx/src/client \
		../third_party/breakpad_osx/src/client/mac/handler \
		../third_party/breakpad_osx/src/common \
		../third_party/breakpad_osx/src/common/mac \
		$(NULL)

BREAKPAD_CPPSRCS	+= \
		dynamic_images.cc \
		exception_handler.cc \
		file_id.cc \
		macho_id.cc \
		macho_utilities.cc \
		macho_walker.cc \
		minidump_file_writer.cc \
		minidump_generator.cc \
		protected_memory_allocator.cc \
		string_conversion.cc \
		string_utilities.cc \
		$(NULL)

SF_VPATH	+= \
		base/common/exception_handler_osx \
		$(NULL)

SF_CPPSRCS	+= \
		exception_handler_osx.mm \
		google_breakpad.mm \
		mach_ipc.mm \
		on_demand_server.mm \
		simple_string_dictionary.mm \
		$(NULL)

endif

#-----------------------------------------------------------------------------
# third_party/convert_utf

THIRD_PARTY_VPATH	+= \
		../third_party/convert_utf \
		$(NULL)

THIRD_PARTY_CSRCS	+= \
		ConvertUTF.c \
		$(NULL)

#-----------------------------------------------------------------------------
# third_party/glint
# Note: Glint is only needed in NOTIFIER_EXE, not in MODULE_DLL,
# so we don't add Glint to THIRD_PARTY_CPPSRCS - that would increase the size
# of of MODULE_DLL unnecessarily.
GLINT_VPATH += \
		../third_party/glint/crossplatform \
		$(NULL)

GLINT_CPPSRCS += \
		allocator.cc \
		animation_timeline.cc \
		base_object.cc \
		bitmap.cc \
		button.cc \
		color.cc \
		column.cc \
		core.cc \
		current_time.cc \
		draw_stack.cc \
		image_node.cc \
		lexer.cc \
		message.cc \
		nine_grid.cc \
		node.cc \
		rectangle.cc \
		root_ui.cc \
		row.cc \
		simple_text.cc \
		size.cc \
		snapshot_stack.cc \
		tile_plane.cc \
		transform.cc \
		$(NULL)

# Glint Win32 files
ifeq ($(OS),win32)
GLINT_VPATH += \
		../third_party/glint/win \
		$(NULL)

GLINT_CPPSRCS += \
		formatted_text.cc \
		win32.cc \
		$(NULL)
endif

# Glint OSX files
ifeq ($(OS),osx)
GLINT_VPATH += \
		../third_party/glint/mac \
		../third_party/glint/posix \
		$(NULL)

GLINT_CPPSRCS += \
		darwin_platform_bitmaps.mm \
		darwin_platform_fonttext.mm \
		darwin_platform_misc.mm \
		darwin_platform_windows.mm \
		glint_bitmap_view.mm \
		glint_window.mm \
		glint_work_item_dispatcher.mm \
		$(NULL)

GLINT_CPPSRCS += \
		posix.cc \
		formatted_text.cc \
		$(NULL)
endif

# Glint Linux files
ifeq ($(OS),linux)
GLINT_VPATH += \
		../third_party/glint/linux \
		../third_party/glint/posix \
		$(NULL)

GLINT_CPPSRCS += \
		linux.cc \
		posix.cc \
		formatted_text.cc \
		$(NULL)
endif

#-----------------------------------------------------------------------------
# third_party/googleurl

THIRD_PARTY_VPATH	+= \
		../third_party/googleurl/src \
		$(NULL)

URL_PARSING_CPPSRCS	+= \
		gurl.cc \
		url_canon_etc.cc \
		url_canon_fileurl.cc \
		url_canon_host.cc \
		url_canon_internal.cc \
		url_canon_ip.cc \
		url_canon_path.cc \
		url_canon_pathurl.cc \
		url_canon_query.cc \
		url_canon_relative.cc \
		url_canon_stdurl.cc \
		url_parse.cc \
		url_parse_file.cc \
		url_util.cc \
		$(NULL)

THIRD_PARTY_CPPSRCS	+= \
		$(URL_PARSING_CPPSRCS) \
		$(NULL)

#-----------------------------------------------------------------------------
# third_party/growl

ifeq ($(OS),osx)
GROWL_VPATH	+= \
		../third_party/growl \
		$(NULL)

GROWL_CPPSRCS	+= \
		CFGrowlAdditions.c \
		CFMutableDictionaryAdditions.c \
		CFURLAdditions.c \
		GrowlApplicationBridge.m \
		GrowlPathUtilities.m \
		$(NULL)

endif

#-----------------------------------------------------------------------------
# third_party/icu38

ifeq ($(USING_ICU),1)
THIRD_PARTY_VPATH	+= \
		../third_party/icu38/source/common \
		$(NULL)

ICU_CPPSRCS	+= \
		utf_impl.cc \
		$(NULL)

THIRD_PARTY_CPPSRCS	+= \
		$(ICU_CPPSRCS) \
		$(NULL)
endif

#-----------------------------------------------------------------------------
# third_party/jsoncpp

THIRD_PARTY_VPATH	+= \
		../third_party/jsoncpp \
		$(NULL)

THIRD_PARTY_CPPSRCS	+= \
		json_reader.cc \
		json_value.cc \
		json_writer.cc \
		$(NULL)

#-----------------------------------------------------------------------------
# third_party/libgd

ifeq ($(USING_LIBGD),1)
ifneq ($(OFFICIAL_BUILD),1)
ifneq ($(OS),wince)
THIRD_PARTY_VPATH	+= \
		../third_party/libgd \
		$(NULL)

LIBGD_CSRCS	+= \
		gd.c \
		gdfx.c \
		gd_security.c \
		gd_gd.c \
		gd_gd2.c \
		gd_io.c \
		gd_io_dp.c \
		gd_gif_in.c \
		gd_gif_out.c \
		gd_io_file.c \
		gd_io_ss.c \
		gd_jpeg.c \
		gd_png.c \
		gd_ss.c \
		gd_topal.c \
		gd_wbmp.c \
		gdcache.c \
		gdfontg.c \
		gdfontl.c \
		gdfontmb.c \
		gdfonts.c \
		gdfontt.c \
		gdft.c \
		gdhelpers.c \
		gdkanji.c \
		gdtables.c \
		gdxpm.c \
		wbmp.c \
		$(NULL)
endif # not wince
endif # not OFFICIAL_BUILD
endif # USING_LIBGD

#-----------------------------------------------------------------------------
# third_party/libjpeg

ifeq ($(USING_LIBJPEG),1)
ifneq ($(OFFICIAL_BUILD),1)
ifneq ($(OS),wince)
THIRD_PARTY_VPATH	+= \
		../third_party/libjpeg \
		$(NULL)

THIRD_PARTY_CSRCS	+= \
		jcapimin.c \
		jcapistd.c \
		jccoefct.c \
		jccolor.c \
		jcdctmgr.c \
		jchuff.c \
		jcinit.c \
		jcmainct.c \
		jcmarker.c \
		jcmaster.c \
		jcomapi.c \
		jcparam.c \
		jcphuff.c \
		jcprepct.c \
		jcsample.c \
		jctrans.c \
		jdapimin.c \
		jdapistd.c \
		jdatadst.c \
		jdatasrc.c \
		jdcoefct.c \
		jdcolor.c \
		jddctmgr.c \
		jdhuff.c \
		jdinput.c \
		jdmainct.c \
		jdmarker.c \
		jdmaster.c \
		jdmerge.c \
		jdphuff.c \
		jdpostct.c \
		jdsample.c \
		jdtrans.c \
		jerror.c \
		jfdctflt.c \
		jfdctfst.c \
		jfdctint.c \
		jidctflt.c \
		jidctfst.c \
		jidctint.c \
		jidctred.c \
		jquant1.c \
		jquant2.c \
		jutils.c \
		jmemmgr.c \
		jmemnobs.c \
		$(NULL)
endif # not wince
endif # not OFFICIAL_BUILD
endif # USING_LIBJPEG

#-----------------------------------------------------------------------------
# third_party/libpng

ifeq ($(USING_LIBPNG),1)
THIRD_PARTY_VPATH	+= \
		../third_party/libpng \
		$(NULL)

# Common Files
THIRD_PARTY_CSRCS	+= \
		png.c \
		pngerror.c \
		pngget.c \
		pngmem.c \
		pngset.c \
		pngtrans.c \
		$(NULL)
# Reading PNGs
THIRD_PARTY_CSRCS	+= \
		pngpread.c \
		pngread.c \
		pngrio.c \
		pngrtran.c \
		pngrutil.c \
		$(NULL)

ifeq ($(OFFICIAL_BUILD),1)
  # The following files are not yet needed; exclude them to save size.
else
# Writing PNGs
THIRD_PARTY_CSRCS	+= \
		pngwio.c \
		pngwrite.c \
		pngwtran.c \
		pngwutil.c \
		$(NULL)
endif
endif # USING_LIBPNG

#-----------------------------------------------------------------------------
# third_party/modp_b64

THIRD_PARTY_VPATH	+= \
		../third_party/modp_b64 \
		$(NULL)

THIRD_PARTY_CPPSRCS	+= \
		modp_b64.cc \
		$(NULL)

#-----------------------------------------------------------------------------
# third_party/spidermonkey

ifeq ($(BROWSER),SF)
THIRD_PARTY_VPATH	+= \
		../third_party/spidermonkey/js/src \
		../third_party/spidermonkey/nspr/pr/src/io \
		../third_party/spidermonkey/nspr/pr/src/linking \
		../third_party/spidermonkey/nspr/pr/src/malloc \
		../third_party/spidermonkey/nspr/pr/src/md \
		../third_party/spidermonkey/nspr/pr/src/md/unix \
		../third_party/spidermonkey/nspr/pr/src/memory \
		../third_party/spidermonkey/nspr/pr/src/misc \
		../third_party/spidermonkey/nspr/pr/src/pthreads \
		../third_party/spidermonkey/nspr/pr/src/threads \
		../third_party/spidermonkey/xpcom/glue \
		$(NULL)

MOZJS_CSRCS	+= \
		darwin.c \
		jsapi.c \
		jsarena.c \
		jsarray.c \
		jsatom.c \
		jsbool.c \
		jscntxt.c \
		jsdate.c \
		jsdbgapi.c \
		jsdhash.c \
		jsdtoa.c \
		jsemit.c \
		jsexn.c \
		jsfun.c \
		jsgc.c \
		jshash.c \
		jsinterp.c \
		jsinvoke.c \
		jsiter.c \
		jskwgen.c \
		jslock.c \
		jslog2.c \
		jslong.c \
		jsmath.c \
		jsnum.c \
		jsobj.c \
		jsopcode.c \
		jsparse.c \
		jsprf.c \
		jsregexp.c \
		jsscan.c \
		jsscope.c \
		jsscript.c \
		jsstr.c \
		jsutil.c \
		jsxdrapi.c \
		jsxml.c \
		pldhash.c \
		pratom.c \
		prcmon.c \
		prdtoa.c \
		prenv.c \
		prerr.c \
		prerror.c \
		prerrortable.c \
		prfdcach.c \
		prinit.c \
		prinrval.c \
		priometh.c \
		pripv6.c \
		prlayer.c \
		prlink.c \
		prlog.c \
		prmalloc.c \
		prmapopt.c \
		prmem.c \
		prmjtime.c \
		prmmap.c \
		prmwait.c \
		prnetdb.c \
		prosdep.c \
		prprf.c \
		prrwlock.c \
		prscanf.c \
		prseg.c \
		prstdio.c \
		prtime.c \
		prtpd.c \
		ptio.c \
		ptmisc.c \
		ptsynch.c \
		ptthread.c \
		unix.c \
		unix_errors.c \
		uxproces.c \
		os_Darwin_ppc.s \
		os_Darwin_x86.s \
		$(NULL)

$(BROWSER)_VPATH	+= \
		../third_party/spidermonkey/gears_npapi_bindings \
		$(NULL)

$(BROWSER)_CPPSRCS	+= \
		mozjs_npapi_storage.cc \
		mozjs_npruntime.cc \
		mozjs_npruntime_utils.cc \
		$(NULL)
endif # BROWSER_SF

#-----------------------------------------------------------------------------
# third_party/sqlite_google

ifeq ($(USING_SQLITE),1)
THIRD_PARTY_VPATH	+= \
		../third_party/sqlite_google/src \
		../third_party/sqlite_google/preprocessed \
		../third_party/sqlite_google/ext/fts1 \
		../third_party/sqlite_google/ext/fts2 \
		$(NULL)

SQLITE_CSRCS	+= \
		alter.c \
		analyze.c \
		attach.c \
		auth.c \
		btree.c \
		build.c \
		callback.c \
		complete.c \
		date.c \
		delete.c \
		expr.c \
		func.c \
		hash.c \
		insert.c \
		legacy.c \
		loadext.c \
		main.c \
		malloc.c \
		opcodes.c \
		os.c \
		os_os2.c \
		os_unix.c \
		os_win.c \
		pager.c \
		parse.c \
		pragma.c \
		prepare.c \
		printf.c \
		random.c \
		select.c \
		table.c \
		tokenize.c \
		trigger.c \
		update.c \
		utf.c \
		util.c \
		vacuum.c \
		vdbe.c \
		vdbeapi.c \
		vdbeaux.c \
		vdbeblob.c \
		vdbefifo.c \
		vdbemem.c \
		vtab.c \
		where.c \
		fts1.c \
		fts1_hash.c \
		fts1_porter.c \
		fts1_tokenizer1.c \
		fts2.c \
		fts2_hash.c \
		fts2_porter.c \
		fts2_tokenizer1.c \
		$(NULL)

ifeq ($(OS),symbian)
SQLITE_CSRCS	+= \
		os_symbian.cc \
		$(NULL)
endif # symbian
endif # USING_SQLITE

#-----------------------------------------------------------------------------
# third_party/v8/bindings

CHROME_VPATH	+= \
		../third_party/v8/bindings_local \
		$(NULL)

CHROME_CPPSRCS	+= \
		npruntime.cc \
		np_v8object.cc \
		v8_helpers.cc \
		v8_np_utils.cc \
		v8_npobject.cc \
 		$(NULL)

V8_DIR = ../third_party/v8/bin-$(MODE)
CHROME_LINK_EXTRAS += $(V8_DIR)/libv8core.lib $(V8_DIR)/libjscre.lib $(V8_DIR)/no_snapshotv8.lib 



#-----------------------------------------------------------------------------
# third_party/zlib

ifeq ($(USING_ZLIB),1)
THIRD_PARTY_VPATH	+= \
		../third_party/zlib \
		$(NULL)

# Common Files
THIRD_PARTY_CSRCS	+= \
		adler32.c \
		zutil.c \
		$(NULL)
# Inflate Algorithm (use inflate or infback, but not both)
THIRD_PARTY_CSRCS	+= \
		inflate.c \
		inffast.c \
		inftrees.c \
		$(NULL)

ifeq ($(OFFICIAL_BUILD),1)
  # The following files are not yet needed; exclude them to save size.
else
# Other Algorithms
THIRD_PARTY_CSRCS	+= \
		compress.c \
		deflate.c \
		uncompr.c \
		$(NULL)
# Other Common Files
THIRD_PARTY_CSRCS	+= \
		crc32.c \
		gzio.c \
		trees.c \
		$(NULL)
endif
endif # USING_ZLIB

#-----------------------------------------------------------------------------
# third_party/portaudio
ifeq ($(USING_PORTAUDIO),1)
ifeq ($(OFFICIAL_BUILD),1)
# PortAudio is for the Audio API which has not been finalized for official
# builds.
else
ifeq ($(USING_PORTAUDIO),1)

THIRD_PARTY_VPATH	+= \
		../third_party/portaudio/src/common \
		$(NULL)

ifeq ($(IS_WIN32_OR_WINCE),1)
THIRD_PARTY_VPATH	+= \
		../third_party/portaudio/src/hostapi/wmme \
		../third_party/portaudio/src/os/win \
		$(NULL)
endif
ifeq ($(OS),linux)
THIRD_PARTY_VPATH	+= \
		../third_party/portaudio/src/hostapi/oss \
		../third_party/portaudio/src/os/unix \
		$(NULL)
endif
ifeq ($(OS),osx)
THIRD_PARTY_VPATH	+= \
		../third_party/portaudio/src/hostapi/coreaudio \
		../third_party/portaudio/src/os/mac_osx \
		../third_party/portaudio/src/os/unix \
		$(NULL)
endif

PORTAUDIO_CSRCS	+= \
		pa_allocation.c \
		pa_converters.c \
		pa_cpuload.c \
		pa_debugprint.c \
		pa_dither.c \
		pa_front.c \
		pa_process.c \
		pa_skeleton.c \
		pa_stream.c \
		pa_trace.c \
		$(NULL)

ifeq ($(IS_WIN32_OR_WINCE),1)
PORTAUDIO_CSRCS	+= \
		pa_win_wmme.c \
		pa_win_hostapis.c \
		pa_win_util.c \
		pa_win_waveformat.c \
		$(NULL)
endif
ifeq ($(OS),linux)
PORTAUDIO_CSRCS	+= \
		pa_unix_oss.c \
		pa_unix_hostapis.c \
		pa_unix_util.c \
		$(NULL)
endif
ifeq ($(OS),osx)
PORTAUDIO_CSRCS	+= \
		pa_ringbuffer.c \
		pa_mac_core.c \
		pa_mac_core_blocking.c \
		pa_mac_core_utilities.c \
		pa_mac_hostapis.c \
		pa_unix_util.c \
		$(NULL)
endif

endif # USING_PORTAUDIO
endif # not OFFICIAL_BUILD
endif # USING_PORTAUDIO
#-----------------------------------------------------------------------------
# third_party/libspeex
ifeq ($(USING_LIBSPEEX),1)
ifneq ($(OFFICIAL_BUILD),1)
THIRD_PARTY_VPATH	+= \
		../third_party/speex/libspeex \
		$(NULL)

LIBSPEEX_CSRCS	+= \
		bits.c \
		cb_search.c \
		exc_10_16_table.c \
		exc_10_32_table.c \
		exc_20_32_table.c \
		exc_5_256_table.c \
		exc_5_64_table.c \
		exc_8_128_table.c \
		filters.c \
		gain_table.c \
		gain_table_lbr.c \
		hexc_10_32_table.c \
		hexc_table.c \
		high_lsp_tables.c \
		lpc.c \
		lsp.c \
		lsp_tables_nb.c \
		ltp.c \
		modes.c \
		modes_wb.c \
		nb_celp.c \
		quant_lsp.c \
		sb_celp.c \
		speex.c \
		speex_callbacks.c \
		speex_header.c \
		stereo.c \
		vbr.c \
		vq.c \
		window_spx.c \
		$(NULL)

endif # not OFFICIAL_BUILD
endif # USING_LIBSPEEX
#-----------------------------------------------------------------------------
# third_party/libtremor
ifeq ($(USING_LIBTREMOR),1)
ifneq ($(OFFICIAL_BUILD),1)
THIRD_PARTY_VPATH	+= \
		../third_party/tremor \
		$(NULL)

LIBTREMOR_CSRCS	+= \
		bitwise.c \
		block.c \
		codebook.c \
		floor0.c \
		floor1.c \
		framing.c \
		info.c \
		mapping0.c \
		mdct.c \
		registry.c \
		res012.c \
		sharedbook.c \
		synthesis.c \
		window_tremor.c \
		$(NULL)

endif # not OFFICIAL_BUILD
endif # USING_LIBTREMOR
#-----------------------------------------------------------------------------
# base/common

COMMON_VPATH	+= \
		base/common \
		installer \
		$(NULL)

COMMON_M4SRCS	+= \
		npapi_msi.wxs.m4 \
		product_constants.h.m4 \
		win32_msi.wxs.m4 \
		wince_cab.inf.m4 \
		$(NULL)

COMMON_CPPSRCS	+= \
		$(NULL)
# WARNING: Think carefully before listing any non-third-party C/C++ files in
# COMMON_*SRCS. Such files only get built once per platform (not per browser),
# and BROWSER_* #defines are not obeyed.  Instead, use $(BROWSER)_CPPSRCS
# when possible.
# One important exception: unit-tested code is only built once per platform, so
# it should be listed in CPPSRCS. Thus it must not use any BROWSER_* #defines,
# directly or indirectly.
#
# TODO(cprince): investigate OS-specific sources (e.g. WIN32_CPPSRCS)


#-----------------------------------------------------------------------------
# base/common (built for all browsers)

$(BROWSER)_VPATH	+= \
		base/common \
		$(NULL)

$(BROWSER)_M4SRCS	+= \
		product_version.rc.m4 \
		$(NULL)

$(BROWSER)_CPPSRCS	+= \
		async_router.cc \
		base_class.cc \
		base64.cc \
		byte_store.cc \
		byte_store_test.cc \
		circular_buffer_test.cc \
		database_name_table.cc \
		event.cc \
		event_test.cc \
		file.cc \
		file_test.cc \
		html_event_monitor.cc \
		http_utils.cc \
		js_dom_element.cc \
		js_marshal.cc \
		js_runner_utils.cc \
		js_types.cc \
		leak_counter.cc \
		memory_buffer.cc \
		memory_buffer_test.cc \
		message_queue.cc \
		message_service.cc \
		message_service_test.cc \
		mime_detect.cc \
		mutex.cc \
		mutex_posix.cc \
		mutex_test.cc \
		mutex_win32.cc \
		name_value_table.cc \
		name_value_table_test.cc \
		paths.cc \
		permissions_db.cc \
		permissions_db_test.cc \
		permissions_manager.cc \
		position_table.cc \
		process_utils_win32.cc \
		png_utils.cc \
		scoped_refptr_test.cc \
		security_model.cc \
		security_model_test.cc \
		serialization.cc \
		serialization_test.cc \
		shortcut_table.cc \
		sqlite_wrapper.cc \
		sqlite_wrapper_test.cc \
		stopwatch.cc \
		stopwatch_posix.cc \
		stopwatch_win32.cc \
		string16.cc \
		string_utils.cc \
		string_utils_osx.cc \
		string_utils_test.cc \
		thread.cc \
		thread_locals.cc \
		thread_posix.cc \
		thread_win32.cc \
		timed_call.cc \
		timed_call_test.cc \
		url_utils.cc \
		url_utils_test.cc \
		user_config.cc \
		$(NULL)

ifeq ($(OS),osx)
$(BROWSER)_CPPSRCS	+= \
		common_osx.mm \
		notifier_process_osx.mm \
		nsstring_utils.mm \
		$(NULL)
endif

#-----------------------------------------------------------------------------
# base/firefox

FF3_VPATH	+= \
		base/common \
		base/firefox \
		$(NULL)

FF3_M4SRCS	+= \
		install.rdf.m4 \
		$(NULL)

FF3_IDLSRCS	+= \
		interfaces.idl \
		$(NULL)

FF3_CPPSRCS	+= \
		dom_utils.cc \
		file_posix.cc \
		file_win32.cc \
		html_event_monitor_ff.cc \
		ipc_message_queue_linux.cc \
		ipc_message_queue_null.cc \
		ipc_message_queue_test.cc \
		ipc_message_queue_test_linux.cc \
		ipc_message_queue_test_win32.cc \
		ipc_message_queue_win32.cc \
		js_runner_ff.cc \
		js_runner_ff_marshaling.cc \
		message_queue_ff.cc \
		module.cc \
		ns_file_utils.cc \
		paths_ff.cc \
		time_utils_win32.cc \
		xpcom_dynamic_load.cc \
		$(NULL)

ifeq ($(OS),osx)
FF3_CPPSRCS	+= \
		ipc_message_queue_test_osx.mm \
		$(NULL)
endif

#-----------------------------------------------------------------------------
# base/ie

IE_VPATH	+= \
		base/common \
		base/ie \
		$(NULL)

IE_M4SRCS	+= \
		bho.rgs.m4 \
		interfaces.idl.m4 \
		module.rgs.m4 \
		$(NULL)

IE_IDLSRCS	+= \
		interfaces.idl \
		$(NULL)

IE_CPPSRCS	+= \
		activex_utils.cc \
		bho.cc \
		detect_version_collision_win32.cc \
		dispatcher_to_idispatch.cc \
		ipc_message_queue_null.cc \
		ipc_message_queue_test.cc \
		ipc_message_queue_test_win32.cc \
		ipc_message_queue_win32.cc \
		js_runner_ie.cc \
		message_queue_ie.cc \
		module.cc \
		module_wrapper.cc \
		paths_ie.cc \
		time_utils_win32.cc \
		vista_utils.cc \
		$(NULL)
ifeq ($(OS),win32)
IE_CPPSRCS	+= \
		file_win32.cc \
		html_event_monitor_ie.cc \
		ie_version.cc \
		$(NULL)
endif
ifeq ($(OS),wince)
IE_CPPSRCS	+= \
		common_ie.cc \
		file_wince.cc \
		wince_compatibility.cc \
		$(NULL)
endif

IE_LINK_EXTRAS	+= \
		$(IE_OUTDIR)/module.res \
		$(NULL)

#-----------------------------------------------------------------------------
# base/npapi

NPAPI_VPATH	+= \
		base/common \
		base/npapi \
		base/ie \
		$(NULL)

NPAPI_M4SRCS	+= \
		$(NULL)

NPAPI_IDLSRCS	+= \
		$(NULL)

NPAPI_CPPSRCS	+= \
		browser_utils.cc \
		html_event_monitor_np.cc \
		ipc_message_queue_linux.cc \
		ipc_message_queue_null.cc \
		ipc_message_queue_test.cc \
		ipc_message_queue_test_linux.cc \
		ipc_message_queue_test_win32.cc \
		ipc_message_queue_win32.cc \
		js_runner_np.cc \
		module.cc \
		np_utils.cc \
		npn_bindings.cc \
		npp_bindings.cc \
		plugin.cc \
		$(NULL)

CHROME_VPATH	+= \
		base/chrome \
		$(NULL)

CHROME_CPPSRCS	+= \
		js_runner_cr.cc \
		module_cr.cc \
		paths_cr.cc \
		$(NULL)

ifeq ($(OS),win32)
NPAPI_CPPSRCS	+= \
		detect_version_collision_win32.cc \
		file_win32.cc \
		ie_version.cc \
		message_queue_ie.cc \
		time_utils_win32.cc \
		vista_utils.cc \
		$(NULL)
# TODO(mpcomplete): use WIN32_LINK_EXTRAS here.
NPAPI_LINK_EXTRAS  += \
                $(NPAPI_OUTDIR)/module.res \
                $(NULL)
else
ifeq ($(OS),symbian)
NPAPI_M4SRCS	+= \
		gears_armv5.pkg.m4 \
		$(NULL)

NPAPI_CPPSRCS	+= \
		file_symbian.cc \
		stopwatch_symbian.cc \
		paths_np.cc \
		plugin_symbian.cc \
		uid_symbian.cc \
		$(NULL)
else
NPAPI_CPPSRCS	+= \
		file_posix.cc \
		$(NULL)
endif # SYMBIAN
endif # WIN32

#-----------------------------------------------------------------------------
# base/safari

SF_VPATH	+= \
		base/common \
		base/npapi \
		base/safari \
		$(NULL)

SF_CPPSRCS	+= \
		browser_load_hook.mm \
		browser_utils.cc \
		browser_utils_sf.cc \
		common_sf.mm \
		curl_downloader.mm \
		detect_version_collision_osx.mm \
		file_posix.cc \
		html_event_monitor_np.cc \
		http_handler.mm \
		ipc_message_queue_linux.cc \
		ipc_message_queue_test.cc \
		ipc_message_queue_test_osx.mm \
		js_runner_np.cc \
		js_standalone_engine_mozjs.cc \
		messagebox.mm \
		message_queue_sf.cc \
		module.cc \
		np_utils.cc \
		npn_bindings.cc \
		npp_bindings.cc \
		paths_sf.mm \
		paths_sf_more.mm \
		plugin.cc \
		safari_workarounds.m \
		$(NULL)

SF_LINK_EXTRAS	+= \
		$(SF_OUTDIR)/resources.res \
		$(NULL)

#-----------------------------------------------------------------------------
# vista_broker

IE_VPATH += \
		base/common \
		desktop \
		vista_broker \
		../third_party/convert_utf \
		$(NULL)

VISTA_BROKER_CPPSRCS += \
		process_utils_win32.cc \
		shortcut_utils_win32.cc \
		string16.cc \
		string_utils.cc \
		vista_broker.cc \
		$(NULL)

VISTA_BROKER_CSRCS += \
		ConvertUTF.c \
		$(NULL)

#-----------------------------------------------------------------------------
# crash_inspector

ifeq ($(OS),osx)
COMMON_VPATH += \
		$(BREAKPAD_VPATH) \
		base/common/exception_handler_osx \
		$(NULL)

OSX_CRASH_INSPECTOR_CPPSRCS += \
		$(BREAKPAD_CPPSRCS) \
		convert_UTF.c \
		inspector.mm \
		mach_ipc.mm \
		on_demand_server.mm \
		simple_string_dictionary.mm \
		$(NULL)
endif

#-----------------------------------------------------------------------------
# launch_url_with_browser

ifeq ($(OS),osx)
COMMON_VPATH += \
		base/safari \
		$(NULL)

OSX_LAUNCHURL_CPPSRCS += \
		launch_url_in_browser.cc \
		$(NULL)
endif

#-----------------------------------------------------------------------------
# Safari InputManager

SF_VPATH += \
		base/safari \
		$(NULL)

SF_INPUTMANAGER_CPPSRCS += \
		enabler.m \
		loader.mm \
		$(NULL)

#-----------------------------------------------------------------------------
# Safari Proxy DLL

SF_PROXY_DLL_CPPSRCS += \
		plugin_proxy.mm \
		$(NULL)

#-----------------------------------------------------------------------------
# tools/perf

COMMON_VPATH += \
		tools/perf \
		$(NULL)

PERF_TOOL_CPPSRCS += \
		measure_startup.cc \
		$(NULL)


#-----------------------------------------------------------------------------
# console

$(BROWSER)_VPATH	+= \
		console \
		$(NULL)

$(BROWSER)_CPPSRCS	+= \
		console.cc \
		js_callback_logging_backend.cc \
		$(NULL)

#-----------------------------------------------------------------------------
# crash_sender

ifeq ($(OS),win32)
COMMON_VPATH += \
		crash_sender \
		../third_party/breakpad/src/client/windows/sender \
		../third_party/breakpad/src/common/windows \
		$(NULL)

CRASH_SENDER_CPPSRCS  += \
		crash_report_sender.cc \
		crash_sender_win32.cc \
		http_upload.cc \
		$(NULL)
endif

ifeq ($(OS),osx)
COMMON_VPATH += \
		crash_sender \
		gears/base/common/exception_handler_osx \
		../third_party/breakpad_osx/src/common/mac \
		$(NULL)

CRASH_SENDER_CPPSRCS += \
		crash_sender_osx.m \
		HTTPMultipartUpload.m \
		nshost_macaddress.m \
		$(NULL)
endif


#-----------------------------------------------------------------------------
# canvas

ifeq ($(OFFICIAL_BUILD),1)
# The Canvas API is not yet enabled in official builds.
else
ifneq ($(findstring $(OS), win32|osx),)
$(BROWSER)_VPATH += \
		canvas \
		$(NULL)

$(BROWSER)_CPPSRCS += \
		blob_backed_skia_input_stream.cc \
		blob_backed_skia_output_stream.cc \
		canvas.cc \
		canvas_rendering_context_2d.cc \
		$(NULL)
endif  # win32 | osx
endif  # OFFICIAL_BUILD

#-----------------------------------------------------------------------------
# database

$(BROWSER)_VPATH	+= \
		database \
		$(NULL)

$(BROWSER)_CPPSRCS	+= \
		database.cc \
		database_utils.cc \
		database_utils_test.cc \
		result_set.cc \
		$(NULL)

#-----------------------------------------------------------------------------
# database2

$(BROWSER)_VPATH += \
                database2 \
                $(NULL)

$(BROWSER)_CPPSRCS += \
                connection.cc \
                commands.cc \
                database2.cc \
                database2_common.cc \
                database2_metadata.cc \
                interpreter.cc \
                manager.cc \
                result_set2.cc \
                statement.cc \
                transaction.cc \
                $(NULL)

#-----------------------------------------------------------------------------
# desktop

$(BROWSER)_VPATH += \
		desktop \
		$(NULL)

$(BROWSER)_CPPSRCS += \
		desktop.cc \
		desktop_linux.cc \
		desktop_osx.cc \
		desktop_test.cc \
		desktop_win32.cc \
		dll_data_wince.cc \
		drag_and_drop_registry.cc \
		notification_message_orderer.cc \
		shortcut_utils_win32.cc \
		$(NULL)

$(BROWSER)_CPPSRCS += \
		file_dialog.cc \
		file_dialog_gtk.cc \
		file_dialog_osx.cc \
		file_dialog_win32.cc \
		$(NULL)

# The browser module also needs these files, to communicate with the notifier.
$(BROWSER)_VPATH += \
		notifier \
		$(NULL)

$(BROWSER)_CPPSRCS += \
		const_notifier.cc \
		notifier_process_linux.cc \
		notifier_process_posix.cc \
		notifier_process_win32.cc \
		notifier_proxy.cc \
		notifier_utils_win32.cc \
		notification.cc \
		$(NULL)

FF3_CPPSRCS += \
		drop_target_ff.cc \
		$(NULL)

IE_CPPSRCS += \
		drop_target_ie.cc \
		$(NULL)

CHROME_CPPSRCS	+= \
		desktop_cr.cc \
		$(NULL)

#-----------------------------------------------------------------------------
# gtest

ifeq ($(OFFICIAL_BUILD),1)
# Don't include gtest in official builds.
else
GTEST_VPATH	+= \
		../third_party/gtest/src \
		$(NULL)

GTEST_CPPSRCS	+= \
		gtest.cc \
		gtest-death-test.cc \
		gtest-filepath.cc \
		gtest-port.cc \
		$(NULL)
endif # not OFFICIAL_BUILD

#-----------------------------------------------------------------------------
# notifier

COMMON_VPATH	+= \
		notifier \
		../third_party/convert_utf \
		$(GLINT_VPATH) \
		$(NULL)

NOTIFIER_SHELL_CPPSRCS += \
		const_notifier.cc \
		notifier_shell_win32.cc \
		notifier_utils_win32.cc \
		$(NULL)

NOTIFIER_CORE	+= \
		$(ICU_CPPSRCS) \
		$(URL_PARSING_CPPSRCS) \
		balloons.cc \
		const_notifier.cc \
		event.cc \
		file.cc \
		file_posix.cc \
		file_win32.cc \
		http_constants.cc \
		http_utils.cc \
		ipc_message_queue_linux.cc \
		ipc_message_queue_null.cc \
		ipc_message_queue_win32.cc \
		mutex.cc \
		mutex_posix.cc \
		mutex_win32.cc \
		notification_manager.cc \
		notifier_process_linux.cc \
		notifier_process_posix.cc \
		notifier_process_win32.cc \
		notifier_utils_win32.cc \
		notification.cc \
		paths_none.cc \
		security_model.cc \
		serialization.cc \
		stopwatch.cc \
		stopwatch_posix.cc \
		stopwatch_win32.cc \
		string16.cc \
		string_utils.cc \
		string_utils_osx.cc \
		system_linux.cc \
		system_win32.cc \
		thread.cc \
		thread_posix.cc \
		thread_win32.cc \
		time_utils_win32.cc \
		timed_call.cc \
		timed_call_test.cc \
		user_activity.cc \
		user_activity_linux.cc \
		user_activity_win32.cc \
		$(NULL)

NOTIFIER_CPPSRCS += \
		$(GLINT_CPPSRCS) \
		$(NOTIFIER_CORE) \
		notifier.cc \
		notifier_linux.cc \
		notifier_win32.cc \
		$(NULL)
		
ifeq ($(OS),win32)		
NOTIFIER_CPPSRCS += \
		exception_handler_win32.cc \
		$(NULL)
endif # win32

ifeq ($(OS),osx)
COMMON_VPATH    += \
		base/common/exception_handler_osx \
		$(NULL)

NOTIFIER_CPPSRCS += \
		exception_handler_osx.mm \
		google_breakpad.mm \
		mach_ipc.mm \
		on_demand_server.mm \
		simple_string_dictionary.mm \
		$(NULL)
endif # osx

NOTIFIER_RESOURCES += \
		notifier/resources/background.png \
		notifier/resources/button_strip.png \
		notifier/resources/menu_button_strip.png \
		$(NULL)

ifeq ($(OS),osx)
NOTIFIER_CPPSRCS += \
		$(GROWL_CPPSRCS) \
		common_osx.mm \
		growl_collection.mm \
		notifier_osx.mm \
		notifier_pref_common.cc \
		notifier_pref_listener.cc \
		notifier_process_osx.mm \
		nsstring_string16.mm \
		system_osx.mm \
		user_activity_osx.mm \
		$(NULL)

COMMON_VPATH	+= \
		tools/osx \
		$(GROWL_VPATH) \
		$(NULL)

COMMON_M4SRCS += \
		Notifier-Info.plist.m4 \
		$(NULL)

NOTIFIER_RESOURCES += \
		notifier/resources/close_button_strip_osx.png \
		$(NULL)
else
NOTIFIER_RESOURCES += \
		notifier/resources/close_button_strip.png \
		$(NULL)
endif  # osx

NOTIFIER_CSRCS += \
		ConvertUTF.c \
		$(NULL)

ifeq ($(OS),linux)
COMMON_VPATH	+= \
		../third_party/google_perftools/src/base \
		$(NULL)

NOTIFIER_CORE	+= \
		sysinfo.cc \
		$(NULL)

NOTIFIER_LIBS 	+= \
		-lX11 \
		-lgtk-x11-2.0 \
		-L../third_party/gtk/linux/lib \
		$(NULL)
endif

ifeq ($(OS),win32)
COMMON_VPATH	+= \
		notifier/resources \
		$(NULL)

NOTIFIER_SHELL_LINK_EXTRAS += \
		$(COMMON_OUTDIR)/notifier_shell.res \
		$(NULL)

NOTIFIER_LINK_EXTRAS += \
		$(COMMON_OUTDIR)/notifier.res \
		$(NULL)

NOTIFIER_LIBS 	+= \
		gdiplus.lib \
		$(NULL)

endif

#-----------------------------------------------------------------------------
# notifier_prefpane

ifeq ($(OS),osx)
COMMON_VPATH += \
		notifier/prefpane \
		$(GROWL_VPATH) \
		$(NULL)

NOTIFIER_PREFPANE_CPPSRCS += \
		$(GROWL_CPPSRCS) \
		notifier_pref.mm \
		notifier_pref_common.cc \
		$(NULL)

NOTIFIER_PREFPANE_NIB += \
		notifier/prefpane/English.lproj/notifier_pref.nib \
		$(NULL)

NOTIFIER_PREFPANE_STRINGS += \
		notifier/prefpane/English.lproj/InfoPlist.strings \
		notifier/prefpane/English.lproj/Localizable.strings \
		$(NULL)

NOTIFIER_PREFPANE_RESOURCES += \
		notifier/prefpane/notifier_pref_icon.tiff \
		$(NULL)

COMMON_M4SRCS += \
		Prefpane-Info.plist.m4 \
		$(NULL)
endif

#-----------------------------------------------------------------------------
# notifier_test
COMMON_VPATH	+= \
		$(GTEST_VPATH) \
		$(NULL)

NOTIFIER_TEST_CPPSRCS += \
		$(GLINT_CPPSRCS) \
		$(GTEST_CPPSRCS) \
		$(NOTIFIER_CORE) \
		balloon_collection_test.cc \
		notification_manager_test.cc \
		unit_test.cc \
		$(NULL)

ifeq ($(OS),osx)
NOTIFIER_TEST_CPPSRCS += \
		$(GROWL_CPPSRCS) \
		common_osx.mm \
		growl_collection.mm \
		notifier_pref_common.cc \
		notifier_pref_listener.cc \
		nsstring_string16.mm \
		system_osx.mm \
		unit_test_osx.mm \
		$(NULL)

COMMON_VPATH += \
		$(GROWL_VPATH) \
		$(NULL)
endif

NOTIFIER_TEST_CSRCS += \
		ConvertUTF.c \
		$(NULL)

#-----------------------------------------------------------------------------
# ipc_test

IPC_TEST_CPPSRCS += \
		event.cc \
		ipc_message_queue_linux.cc \
		ipc_message_queue_null.cc \
		ipc_message_queue_test.cc \
		ipc_message_queue_test_linux.cc \
		ipc_message_queue_test_win32.cc \
		ipc_message_queue_win32.cc \
		mutex.cc \
		mutex_posix.cc \
		mutex_win32.cc \
		serialization.cc \
		stopwatch.cc \
		stopwatch_posix.cc \
		stopwatch_win32.cc \
		string16.cc \
		string_utils.cc \
		string_utils_osx.cc \
		thread.cc \
		thread_posix.cc \
		thread_win32.cc \
		time_utils_win32.cc \
		$(NULL)

ifeq ($(OS),osx)
IPC_TEST_CPPSRCS += \
		common_osx.mm \
		ipc_message_queue_test_osx.mm \
		$(NULL)
endif

IPC_TEST_CSRCS += \
		ConvertUTF.c \
		$(NULL)

#-----------------------------------------------------------------------------
# dummy_module

$(BROWSER)_VPATH += \
		dummy \
		$(NULL)

$(BROWSER)_CPPSRCS += \
		dummy_module.cc \
		$(NULL)


#-----------------------------------------------------------------------------
# test

$(BROWSER)_VPATH += \
		cctests \
		$(NULL)

$(BROWSER)_CPPSRCS += \
		test.cc \
		$(NULL)

#-----------------------------------------------------------------------------
# ui/generated
#
# Anything with the _I18N suffix will be expanded for each language in
# I18N_LANGS

I18N_LANGS	+= \
		en-US \
		ar \
		bg \
		ca \
		cs \
		da \
		de \
		el \
		en-GB \
		es \
		et \
		fa \
		fi \
		fil \
		fr \
		he \
		hi \
		hr \
		hu \
		id \
		is \
		it \
		ja \
		ko \
		lt \
		lv \
		ms \
		nl \
		no \
		pl \
		pt-BR \
		pt-PT \
		ro \
		ru \
		sk \
		sl \
		sr \
		sv \
		th \
		tr \
		uk \
		ur \
		vi \
		zh-CN \
		zh-TW \
		ml \
		te \
		gu \
		kn \
		or \
		bn \
		ta \
		mr \
		$(NULL)

# ui/generated is not specified here, because it's included in the generated
# name for _I18N source files.
$(BROWSER)_VPATH += \
		$(NULL)

$(BROWSER)_HTML_M4SRCS += \
		permissions_dialog.html_m4 \
		settings_dialog.html_m4 \
		shortcuts_dialog.html_m4 \
		$(NULL)

FF3_VPATH	+= \
		$(NULL)

FF3_M4SRCS_I18N	+= \
		i18n.dtd.m4 \
		$(NULL)

NPAPI_VPATH	+= \
		ui/generated \
		$(NULL)

ifeq ($(OS),android)
NPAPI_BASE64SRCS += \
	local_data.png.base64 \
	location_data.png.base64 \
	icon_32x32.png.base64 \
	$(NULL)
endif

# Some files should be omitted on Mac and Linux...
ifeq ($(OS),osx)
else
ifeq ($(OS),linux)
else
# ...but included on Win32 and WinCE, for all targets.
$(BROWSER)_VPATH += \
		ui/generated \
		$(NULL)

IE_LINK_EXTRAS += \
		$(IE_OUTDIR)/string_table.res \
		$(NULL)

VISTA_BROKER_LINK_EXTRAS += \
		$(VISTA_BROKER_OUTDIR)/string_table.res \
		$(NULL)
endif
endif

#-----------------------------------------------------------------------------
# ui/common (built for all browsers)

$(BROWSER)_VPATH += \
		ui/common \
		$(NULL)

$(BROWSER)_CPPSRCS += \
		html_dialog.cc \
		i18n_strings.cc \
		permissions_dialog.cc \
		settings_dialog.cc \
		window_utils.cc \
		$(NULL)

$(BROWSER)_STABSRCS += \
		permissions_dialog.js.stab \
		settings_dialog.js.stab \
		shortcuts_dialog.js.stab \
		$(NULL)

#-----------------------------------------------------------------------------
# ui/firefox

FF3_VPATH	+= \
		ui/firefox \
		$(NULL)

FF3_M4SRCS	+= \
		browser-overlay.js.m4 \
		browser-overlay.xul.m4 \
		chrome.manifest.m4 \
		$(NULL)

FF3_IDLSRCS	+= \
		ui_utils.idl \
		$(NULL)

FF3_CPPSRCS	+= \
		html_dialog_ff.cc \
		ui_utils.cc \
		$(NULL)

#-----------------------------------------------------------------------------
# ui/ie

IE_VPATH	+= \
		ui/ie \
		$(NULL)

IE_M4SRCS	+= \
		tools_menu_item.rgs.m4 \
		ui_resources.rc.m4 \
		$(NULL)

IE_IDLSRCS	+= \
		html_dialog_host.idl \
		$(NULL)

IE_CPPSRCS	+= \
		html_dialog_ie.cc \
		tools_menu_item.cc \
		$(NULL)

IE_LINK_EXTRAS	+= \
		$(IE_OUTDIR)/ui_resources.res \
		$(NULL)

IE_STABSRCS += \
		string_table.rc.stab \
		$(NULL)

# Additional files specific to Win32 or WinCE.
ifeq ($(OS),win32)
IE_CPPSRCS	+= \
		html_dialog_host.cc \
		$(NULL)
endif

ifeq ($(OS),wince)
IE_M4SRCS	+= \
		html_dialog_bridge_iemobile.rgs.m4 \
		$(NULL)

IE_IDLSRCS	+= \
		html_dialog_host_iemobile.idl \
		html_dialog_bridge_iemobile.idl \
		$(NULL)

IE_CPPSRCS	+= \
		html_dialog_host_iemobile.cc \
		html_dialog_bridge_iemobile.cc \
		$(NULL)
endif

#-----------------------------------------------------------------------------
# ui/npapi

NPAPI_VPATH	+= \
		ui/ie \
		$(NULL)

NPAPI_M4SRCS	+= \
		ui_resources.rc.m4 \
		$(NULL)

NPAPI_IDLSRCS	+= \
		$(NULL)

NPAPI_CPPSRCS	+= \
		$(NULL)

NPAPI_LINK_EXTRAS += \
		$(NPAPI_OUTDIR)/ui_resources.res \
		$(NULL)

CHROME_VPATH	+= \
		ui/chrome \
		$(NULL)

CHROME_CPPSRCS	+= \
		settings_dialog_cr.cc \
		html_dialog_cr.cc \
		$(NULL)


#-----------------------------------------------------------------------------
# ui/safari

SF_VPATH	+= \
		ui/safari \
		$(NULL)

SF_CPPSRCS	+= \
		html_dialog_sf.mm \
		html_modeless_dialog_sf.mm \
		native_dialogs_osx.mm \
		settings_menu.mm \
		$(NULL)

#-----------------------------------------------------------------------------
# workerpool/firefox

FF3_VPATH	+= \
		workerpool \
		workerpool/common \
		workerpool/firefox \
		$(NULL)

FF3_CPPSRCS	+= \
		pool_threads_manager.cc \
		workerpool.cc \
		workerpool_utils.cc \
		$(NULL)

#-----------------------------------------------------------------------------
# workerpool/ie

IE_VPATH	+= \
		workerpool \
		workerpool/common \
		workerpool/ie \
		$(NULL)

IE_CPPSRCS	+= \
		pool_threads_manager.cc \
		workerpool.cc \
		workerpool_utils.cc \
		$(NULL)

#-----------------------------------------------------------------------------
# workerpool/npapi

ifeq ($(USING_NPAPI),1)
$(BROWSER)_VPATH	+= \
		workerpool \
		workerpool/common \
		workerpool/npapi \
		$(NULL)

$(BROWSER)_CPPSRCS	+= \
		pool_threads_manager.cc \
		workerpool.cc \
		workerpool_utils.cc \
		$(NULL)
endif

#-----------------------------------------------------------------------------
# timer

$(BROWSER)_VPATH += \
		timer \
		$(NULL)

$(BROWSER)_CPPSRCS += \
		timer.cc \
		$(NULL)

#-----------------------------------------------------------------------------
# appcelerator

$(BROWSER)_VPATH += \
		appcelerator \
		$(NULL)

$(BROWSER)_CPPSRCS += \
		appcelerator.cc \
		$(NULL)


#-----------------------------------------------------------------------------
# localserver/common (built for all browsers)

$(BROWSER)_VPATH += \
		localserver \
		localserver/common \
		$(NULL)

$(BROWSER)_CPPSRCS += \
		blob_store.cc \
		capture_task.cc \
		file_store.cc \
		file_submitter.cc \
		http_constants.cc \
		http_cookies.cc \
		localserver.cc \
		localserver_db.cc \
		localserver_module.cc \
		localserver_perf_test.cc \
		managed_resource_store.cc \
		managed_resource_store_module.cc \
		manifest.cc \
		resource_store.cc \
		resource_store_module.cc \
		update_task.cc \
		update_task_single_process.cc \
		$(NULL)

#-----------------------------------------------------------------------------
# localserver/firefox

FF3_VPATH	+= \
		localserver/firefox \
		$(NULL)

FF3_CPPSRCS	+= \
		async_task_ff.cc \
		cache_intercept.cc \
		http_request_ff.cc \
		progress_event.cc \
		progress_input_stream.cc \
		safe_http_request.cc \
		$(NULL)

#-----------------------------------------------------------------------------
# localserver/ie

IE_VPATH	+= \
		localserver/ie \
		$(NULL)

IE_CPPSRCS	+= \
		async_task_ie.cc \
		file_submit_behavior.cc \
		http_handler_ie.cc \
		http_request_ie.cc \
		progress_event.cc \
		progress_input_stream.cc \
		update_task_ie.cc \
		urlmon_utils.cc \
		$(NULL)

#-----------------------------------------------------------------------------
# localserver/npapi

NPAPI_VPATH	+= \
		localserver/npapi \
		$(NULL)

NPAPI_IDLSRCS	+= \
		$(NULL)

NPAPI_CPPSRCS	+= \
		async_task_np.cc \
		update_task_np.cc \
		$(NULL)

CHROME_VPATH	+= \
		localserver/chrome \
		$(NULL)

CHROME_CPPSRCS	+= \
		gears_protocol_handler.cc \
		network_intercept_cr.cc \
		http_cookies_cr.cc \
		http_request_cr.cc \
		safe_http_request.cc \
		update_task_cr.cc \
		$(NULL)

#-----------------------------------------------------------------------------
# localserver/safari

SF_VPATH	+= \
		localserver/npapi \
		localserver/safari \
		$(NULL)

SF_CPPSRCS	+= \
		async_task_sf.mm \
		http_cookies_sf.mm \
		http_request_delegate.mm \
		http_request_sf.mm \
		localserver_db_proxy.mm \
		progress_event.cc \
		progress_input_stream.mm \
		safe_http_request.cc \
		ui_thread.cc \
		$(NULL)

#-----------------------------------------------------------------------------
# httprequest

$(BROWSER)_VPATH += \
		httprequest \
		$(NULL)

$(BROWSER)_CPPSRCS += \
		httprequest.cc \
		httprequest_upload.cc \
		$(NULL)

#-----------------------------------------------------------------------------
# blob

# (built for all browsers)
$(BROWSER)_VPATH += \
		blob \
		$(NULL)

$(BROWSER)_CPPSRCS += \
		blob.cc \
		blob_builder.cc \
		blob_interface.cc \
		blob_test.cc \
		blob_utils.cc \
		buffer_blob.cc \
		fail_blob.cc \
		file_blob.cc \
		join_blob.cc \
		slice_blob.cc \
		$(NULL)

# TODO(bpm): Make this cross-browser, not Firefox- or Safari-specific.
FF3_CPPSRCS	+= \
		blob_input_stream_ff.cc \
		blob_input_stream_ff_test.cc \
		$(NULL)

SF_CPPSRCS 	+= \
		blob_input_stream_sf.mm \
		blob_input_stream_sf_test.mm \
		$(NULL)

#-----------------------------------------------------------------------------
# inspector

ifeq ($(OFFICIAL_BUILD),1)
# Inspector is not yet enabled in official builds.
else
# $(BROWSER)_VPATH += \
# 		inspector \
# 		$(NULL)
# $(BROWSER)_CPPSRCS += \
# 		inspector_resources.cc \
# 		$(NULL)
# 
# COMMON_VPATH += \
# 		inspector \
# 		$(NULL)
# COMMON_GEN_CPPSRCS += \
# 		console.html.from_bin.cc \
# 		database.html.from_bin.cc \
# 		index.html.from_bin.cc \
# 		localserver.html.from_bin.cc \
# 		$(NULL)
# 
# COMMON_VPATH += \
# 		inspector/common \
# 		$(NULL)
# COMMON_GEN_CPPSRCS += \
# 		alert-35.png.from_bin.cc \
# 		database.gif.from_bin.cc \
# 		error-35.png.from_bin.cc \
# 		ie6hacks.css.from_bin.cc \
# 		inspector_links.js.from_bin.cc \
# 		lightbulb-35.png.from_bin.cc \
# 		localserver.gif.from_bin.cc \
# 		question-35.png.from_bin.cc \
# 		styles.css.from_bin.cc \
# 		workerpool.gif.from_bin.cc \
# 		$(NULL)
# 
# COMMON_VPATH += \
# 		sdk \
# 		ui/common \
# 		$(NULL)
# COMMON_GEN_CPPSRCS += \
# 		base.js.from_bin.cc \
# 		dom.js.from_bin.cc \
# 		gears_init.js.from_bin.cc \
# 		icon_32x32.png.from_bin.cc \
# 		$(NULL)

endif # not OFFICIAL_BUILD

#-----------------------------------------------------------------------------
# image

ifneq ($(OFFICIAL_BUILD),1)
ifneq ($(OS),wince)
$(BROWSER)_VPATH += \
		image \
		$(NULL)

$(BROWSER)_CPPSRCS += \
		backing_image.cc \
		image.cc \
		image_loader.cc \
		$(NULL)
endif # not wince
endif # not OFFICIAL_BUILD

#-----------------------------------------------------------------------------
# factory/firefox

FF3_VPATH	+= \
		factory \
		$(NULL)

FF3_CPPSRCS	+= \
		factory_ff.cc \
		factory_impl.cc \
		factory_utils.cc \
		$(NULL)

#-----------------------------------------------------------------------------
# factory/ie

IE_VPATH	+= \
		factory \
		$(NULL)

IE_M4SRCS	+= \
		factory_ie.rgs.m4 \
		$(NULL)

IE_CPPSRCS	+= \
		factory_ie.cc \
		factory_impl.cc \
		factory_utils.cc \
		$(NULL)

#-----------------------------------------------------------------------------
# factory/npapi

ifeq ($(USING_NPAPI),1)
$(BROWSER)_VPATH	+= \
		factory \
		$(NULL)

$(BROWSER)_IDLSRCS	+= \
		$(NULL)

$(BROWSER)_CPPSRCS	+= \
		factory_impl.cc \
		factory_np.cc \
		factory_utils.cc \
		$(NULL)
endif

#-----------------------------------------------------------------------------
# installer/iemobile

ifeq ($(OS),wince)
IE_VPATH	+= \
		installer/iemobile \
		$(NULL)

IE_WINCESETUP_CPPSRCS += \
		process_restarter.cc \
		setup.cc \
		$(NULL)

IE_WINCESETUP_LINK_EXTRAS += \
		$(IE_OUTDIR)/setup.res \
		$(NULL)

# List these setup-related items in IE_FOO just so they get built before used.
IE_M4SRCS	+= \
		setup.rc.m4 \
		$(NULL)

IE_LINK_EXTRAS += \
		$(IE_OUTDIR)/setup.res \
		$(NULL)

# TODO(cprince): move this to another source path; IE_MODULE_DLL links it.
IE_CPPSRCS += \
		cab_updater.cc \
		$(NULL)
endif # wince

#-----------------------------------------------------------------------------
# installer/safari

SF_VPATH	+= \
		installer/safari \
		tools/osx \
		$(NULL)

SF_M4SRCS	+= \
		installer.packproj.m4 \
		Info.plist.m4 \
		$(NULL)

#-----------------------------------------------------------------------------
# installer/android

ifeq ($(OS),android)
NPAPI_VPATH	+= \
		installer/android \
		$(NULL)

NPAPI_CPPSRCS	+= \
		lib_updater.cc \
		version_check_task.cc \
		zip_download_task.cc \
		$(NULL)

NPAPI_JAVASRCS	+= \
		VersionExtractor.java \
		ZipInflater.java \
		$(NULL)
endif # android

#-----------------------------------------------------------------------------
# geolocation

$(BROWSER)_VPATH	+= \
		geolocation \
		$(NULL)

$(BROWSER)_IDLSRCS	+= \
		$(NULL)

$(BROWSER)_CPPSRCS	+= \
		empty_device_data_provider.cc \
		geolocation.cc \
		geolocation_db.cc \
		geolocation_db_test.cc \
		geolocation_test.cc \
		gps_location_provider_wince.cc \
		location_provider.cc \
		location_provider_pool.cc \
		network_location_provider.cc \
		network_location_request.cc \
		radio_data_provider_wince.cc \
		timed_callback.cc \
		wifi_data_provider_android.cc \
		wifi_data_provider_common.cc \
		wifi_data_provider_linux.cc \
		wifi_data_provider_osx.cc \
		wifi_data_provider_win32.cc \
		wifi_data_provider_wince.cc \
		wifi_data_provider_windows_common.cc \
		$(NULL)

#-----------------------------------------------------------------------------
# media

ifeq ($(OFFICIAL_BUILD),1)
# The Audio API has not been finalized for official builds.
else
$(BROWSER)_VPATH += \
		media \
		$(NULL)

$(BROWSER)_CPPSRCS += \
		audio.cc \
		audio_recorder.cc \
		audio_recorder_test.cc \
		base_audio_recorder.cc \
		media.cc \
		media_data.cc \
		mock_audio_recorder.cc \
		pa_audio_recorder.cc \
		time_ranges.cc \
		$(NULL)

endif # not OFFICIAL_BUILD

#-----------------------------------------------------------------------------
# Add any hard-coded "FF3_" sources to the FF2_ sources.

FF2_VPATH += $(FF3_VPATH)
FF2_M4SRCS += $(FF3_M4SRCS)
FF2_M4SRCS_I18N += $(FF3_M4SRCS_I18N)
FF2_IDLSRCS += $(FF3_IDLSRCS)
FF2_CPPSRCS += $(FF3_CPPSRCS)


# Chrome-specific hackery.  NPAPI+win32 == Chrome.
ifeq ($(BROWSER),NPAPI)
ifeq ($(OS),win32)

# Add Chrome-only sources to the NPAPI build.
NPAPI_CPPSRCS += $(CHROME_CPPSRCS)
NPAPI_CPPFLAGS += $(CHROME_CPPFLAGS)
NPAPI_LINK_EXTRAS += $(CHROME_LINK_EXTRAS)
NPAPI_VPATH += $(CHROME_VPATH)
NPAPI_DLLFLAGS += $(CHROME_DLLFLAGS)

# Remove some files that we don't want to compile for Chrome.
NPAPI_CPPSRCS := $(subst http_cookies.cc,,$(NPAPI_CPPSRCS))
NPAPI_CPPSRCS := $(subst http_request_ie.cc,,$(NPAPI_CPPSRCS))
NPAPI_CPPSRCS := $(subst http_handler_ie.cc,,$(NPAPI_CPPSRCS))
NPAPI_CPPSRCS := $(subst js_runner_np.cc,,$(NPAPI_CPPSRCS))
NPAPI_CPPSRCS := $(subst urlmon_utils.cc,,$(NPAPI_CPPSRCS))
NPAPI_CPPSRCS := $(subst update_task_np.cc,,$(NPAPI_CPPSRCS))
NPAPI_CPPSRCS := $(subst paths_np.cc,,$(NPAPI_CPPSRCS))
NPAPI_CPPSRCS := $(subst settings_dialog.cc,,$(NPAPI_CPPSRCS))
endif
endif


#-----------------------------------------------------------------------------
# Include site-specific build rules, evaluated before our built-in build rules.
ifneq ($(wildcard site-prerules.mk),)  # if file exists
include site-prerules.mk
endif

#-----------------------------------------------------------------------------

include tools/rules.mk

#-----------------------------------------------------------------------------
# Include site-specific build rules, evaluated after our built-in build rules.
ifneq ($(wildcard site-postrules.mk),)  # if site-postrules.mk exists.
include site-postrules.mk
endif
