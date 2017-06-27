T01:
	clang example_remux.c cgts_demux.c cgts_mux.c cgts_structs.c cgts_util.c -o cgts_remux
	clang example_analyse.c cgts_demux.c cgts_mux.c cgts_structs.c cgts_util.c -o cgts_analyse
clean:
	rm -rfv cgts_remux cgts_analyse
run:
	./cgts_remux test_samples/h264_aac.ts output.ts
