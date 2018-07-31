# -*- Mode: makefile-gmake -*-

RUN_MAKE = $(if $(V),$(MAKE) $1, @$(MAKE) --no-print-directory $1)

all:
%:
	$(call RUN_MAKE,-C test_base32 $*)
	$(call RUN_MAKE,-C test_base64 $*)
	$(call RUN_MAKE,-C test_bitset $*)
	$(call RUN_MAKE,-C test_buf $*)
	$(call RUN_MAKE,-C test_fmem $*)
	$(call RUN_MAKE,-C test_hash $*)
	$(call RUN_MAKE,-C test_itr $*)
	$(call RUN_MAKE,-C test_math $*)
	$(call RUN_MAKE,-C test_md $*)
	$(call RUN_MAKE,-C test_mfp $*)
	$(call RUN_MAKE,-C test_mpm $*)
	$(call RUN_MAKE,-C test_parse $*)
	$(call RUN_MAKE,-C test_ring $*)
	$(call RUN_MAKE,-C test_stack $*)
	$(call RUN_MAKE,-C test_str $*)
	$(call RUN_MAKE,-C test_strbuf $*)
	$(call RUN_MAKE,-C test_utf8 $*)
	$(call RUN_MAKE,-C test_queue $*)
	$(call RUN_MAKE,-C test_vector $*)
