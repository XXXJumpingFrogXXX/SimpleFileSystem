OUTDIR=./out

fs:main.o api.o disk.o list.o shell.o time.o
	cc -o fs $(OUTDIR)/main.o $(OUTDIR)/api.o $(OUTDIR)/disk.o $(OUTDIR)/list.o $(OUTDIR)/shell.o $(OUTDIR)/time.o

main.o:main.c ./api/api.h ./shell/shell.h
	cc -c main.c 
	mv main.o $(OUTDIR)/main.o
	
api.o:./api/api.c ./api/api.h
	cc -c ./api/api.c
	mv api.o $(OUTDIR)/api.o

disk.o:./tool/disk.c ./tool/disk.h
	cc -c ./tool/disk.c
	mv disk.o $(OUTDIR)/disk.o

list.o:./tool/list.c ./tool/list.h
	cc -c ./tool/list.c
	mv list.o $(OUTDIR)/list.o

shell.o:./shell/shell.c ./shell/shell.h
	cc -c ./shell/shell.c
	mv shell.o $(OUTDIR)/shell.o

time.o:./tool/time.c ./tool/time.h
	cc -c ./tool/time.c
	mv time.o $(OUTDIR)/time.o
.PHONY:clean
clean:
	-rm $(OUTDIR)/main.o $(OUTDIR)/api.o $(OUTDIR)/disk.o $(OUTDIR)/list.o $(OUTDIR)/shell.o $(OUTDIR)/time.o
