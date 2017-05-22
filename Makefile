T01:
	clang app.c cgts.c -o cgts
clean:
	rm -rfv cgts
run:
	./cgts demo.ts
