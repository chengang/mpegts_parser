T01:
	clang example_remux.c   ../cgts/cgts_demux.c ../cgts/cgts_mux.c ../cgts/cgts_structs.c ../cgts/cgts_util.c -I ../cgts -o cgts_remux
	clang example_analyse.c ../cgts/cgts_demux.c ../cgts/cgts_mux.c ../cgts/cgts_structs.c ../cgts/cgts_util.c -I ../cgts -o cgts_analyse
	clang example_encrypt.c ../cgts/cgts_demux.c ../cgts/cgts_mux.c ../cgts/cgts_structs.c ../cgts/cgts_util.c -I ../cgts -o cgts_encrypt
clean:
	rm -rfv output.ts cgts_remux cgts_analyse
remux:
	./cgts_remux ../test_samples/h264_aac.ts output.ts
enc:
	./cgts_encrypt ../test_samples/h264_aac.ts output.ts
