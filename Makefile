T01:
	clang app.c cgts.c structs.c util.c -o cgts
clean:
	rm -rfv cgts
run:
	./cgts demo.ts
