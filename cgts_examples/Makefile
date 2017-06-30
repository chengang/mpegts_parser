CCOMP = clang
CFLAGS = -I ../cgts -I ../cg_crypto
CLIBS = ../cgts/cgts_demux.c ../cgts/cgts_mux.c ../cgts/cgts_structs.c ../cgts/cgts_nal_adts_parse.c ../cgts/cgts_util.c ../cg_crypto/aes128.c

T01:
	$(CCOMP) example_remux.c   $(CLIBS) $(CFLAGS) -o cgts_remux
	$(CCOMP) example_analyse.c $(CLIBS) $(CFLAGS) -o cgts_analyse
	$(CCOMP) example_encrypt.c $(CLIBS) $(CFLAGS) -o cgts_encrypt
	$(CCOMP) example_decrypt.c $(CLIBS) $(CFLAGS) -o cgts_decrypt
clean:
	rm -rfv output.ts cgts_remux cgts_analyse cgts_encrypt cgts_decrypt
remux:
	./cgts_remux ../test_samples/h264_aac.ts output.ts
enc:
	./cgts_encrypt ../test_samples/h264_aac.ts output.ts
