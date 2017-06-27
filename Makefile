T01:
	clang app.c cgts_demux.c cgts_mux.c structs.c util.c -o cgts
	clang example_analyse.c cgts_demux.c cgts_mux.c structs.c util.c -o cgts_analyse
clean:
	rm -rfv cgts
run:
	./cgts test_samples/h264_aac.ts output.ts
