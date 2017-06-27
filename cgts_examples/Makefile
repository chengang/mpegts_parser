T01:
	clang example_remux.c   ../cgts/cgts_demux.c ../cgts/cgts_mux.c ../cgts/cgts_structs.c ../cgts/cgts_util.c -I ../cgts -o cgts_remux
	clang example_analyse.c ../cgts/cgts_demux.c ../cgts/cgts_mux.c ../cgts/cgts_structs.c ../cgts/cgts_util.c -I ../cgts -o cgts_analyse
clean:
	rm -rfv output.ts cgts_remux cgts_analyse
run:
	./cgts_remux ../test_samples/h264_aac.ts output.ts
