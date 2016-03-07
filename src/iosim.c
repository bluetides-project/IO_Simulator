#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <alloca.h>
#include <string.h>

#include <mpi.h>
#include <unistd.h>

#include <bigfile.h>
#include <bigfile-mpi.h>

int ThisTask;
int NTask;

static void 
usage() 
{
    if(ThisTask != 0) return;
    printf("usage: iosim [-N nfiles] [-n numwriters] [-s items] [-w width] [-p path] [-m (create|update|read)] filename\n");
    printf("Defaults: -N 1 -n NTask -s 1 -w 1 -p <dir> -m create \n");
}

static void 
info(char * fmt, ...) {

    static double t0 = -1.0;

    MPI_Barrier(MPI_COMM_WORLD);

    if(t0 < 0) t0 = MPI_Wtime();

    char * buf = alloca(strlen(fmt) + 100);
    sprintf(buf, "[%010.3f] %s", MPI_Wtime() - t0, fmt );

    if(ThisTask == 0) {
        va_list va;
        va_start(va, fmt);
        vprintf(buf, va);
        va_end(va);
    }
}

#define MODE_CREATE 0
#define MODE_READ   1
#define MODE_UPDATE 2

static void 
sim(int Nfile, int Nwriter, size_t Nitems, char * filename, double * tlog_ranks, int mode) //
{
    info("Writing to `%s`\n", filename);
    info("Physical Files %d\n", Nfile);
    info("Ranks %d\n", NTask);
    info("Writers %d\n", Nwriter);
    info("Bytes Per Rank %td\n", Nitems * 4 / NTask);
    info("Items Per Rank %td\n", Nitems / NTask);

    size_t itemsperrank = 1024;
    itemsperrank = Nitems / NTask;
    
    BigFile bf = {0};
    BigBlock bb = {0};
    BigArray array = {0};
    BigBlockPtr ptr = {0};

    int * fakedata;
    ptrdiff_t i;

    int color = ThisTask * Nwriter / NTask;
    MPI_Comm COMM_SPLIT;
    int GroupRank;
    int GroupSize;
    MPI_Comm_split(MPI_COMM_WORLD, color, ThisTask, &COMM_SPLIT);
    MPI_Comm_size(COMM_SPLIT, &GroupSize); 
    MPI_Comm_rank(COMM_SPLIT, &GroupRank); 
    MPI_Allreduce(MPI_IN_PLACE, &GroupSize, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);

    if(mode == MODE_CREATE) {
        info("Creating BigFile\n");
        big_file_mpi_create(&bf, filename, MPI_COMM_WORLD);
        info("Created BigFile\n");

        info("Creating BigBlock\n");
        big_file_mpi_create_block(&bf, &bb, "TestBlock", "i4", 1, Nfile, Nitems, MPI_COMM_WORLD);
        info("Created BigBlock\n");
    }  else {
        info("Opening BigFile\n");
        big_file_mpi_open(&bf, filename, MPI_COMM_WORLD);
        info("Opened BigFile\n");

        info("Opening BigBlock\n");
        big_file_mpi_open_block(&bf, &bb, "TestBlock", MPI_COMM_WORLD);
        info("Opened BigBlock\n");
        if(bb.size != Nitems) {
            info("Size mismatched, overriding Nitems = %td\n", bb.size);
            Nitems = bb.size;
            itemsperrank = Nitems / NTask;
        }
        if(bb.Nfile != Nfile) {
            info("Nfile mismatched, overriding Nfile = %d\n", bb.Nfile );
            Nfile = bb.Nfile;
        }
    }

    info("Initializing FakeData\n");
    fakedata = malloc(4 * itemsperrank);
    for(i = 0; i < itemsperrank; i ++) {
        fakedata[i] = i;
    }
    big_array_init(&array, fakedata, "i4", 1, &itemsperrank, NULL);
    info("Initialized FakeData\n");

    if(mode == MODE_CREATE || mode == MODE_UPDATE) {
        info("Writing BigBlock\n");
        double twrite = 0;
        for(i = 0; i < GroupSize; i ++) {
            MPI_Barrier(COMM_SPLIT);
            //info("Writing Round %d\n", i+1);
            if (i != GroupRank) continue;

            double t0 = MPI_Wtime();
            big_block_seek(&bb, &ptr, itemsperrank * ThisTask);
            big_block_write(&bb, &ptr, &array);
            double t1 = MPI_Wtime();
            twrite += t1 - t0;
        }
        info("Written BigBlock\n");
        info("Writing took %f seconds\n", twrite);

        //+++++++++++++++++ Preparing Time Log using MPI_Send & MPI_Recv +++++++++++++++++
/*        if(ThisTask != 0) {
            MPI_Send(&twrite, 1, MPI_DOUBLE, 0, ThisTask, MPI_COMM_WORLD);
        } else if (ThisTask == 0) {
            tlog_ranks[ThisTask] = twrite;
            for (i=1; i<NTask; i++) {
            MPI_Status status;
            MPI_Recv(&twrite, 1, MPI_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            tlog_ranks[status.MPI_SOURCE] = twrite;
            }
        }
*/
        //+++++++++++++++++ Now using MPI_Gather +++++++++++++++++

        MPI_Gather(&twrite, 1, MPI_DOUBLE, tlog_ranks, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

        //+++++++++++++++++ END +++++++++++++++++

    }
    else {
        info("Reading BigBlock\n");
        double twrite = 0;
        for(i = 0; i < GroupSize; i ++) {
            MPI_Barrier(COMM_SPLIT);
            //info("Reading Round %d\n", i);
            if (i != GroupRank) continue;

            double t0 = MPI_Wtime();
            big_block_seek(&bb, &ptr, itemsperrank * ThisTask);
            big_block_read(&bb, &ptr, &array);
            double t1 = MPI_Wtime();
            twrite += t1 - t0;
        }
        info("Read BigBlock\n");
        info("Reading took %f seconds\n", twrite);
        
        //+++++++++++++++++ Preparing Time Log variable +++++++++++++++++
        MPI_Gather(&twrite, 1, MPI_DOUBLE, tlog_ranks, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    }
    info("Closing BigBlock\n");
    big_block_mpi_close(&bb, MPI_COMM_WORLD);
    info("Closed BigBlock\n");

    info("Closing BigFile\n");
    big_file_mpi_close(&bf, MPI_COMM_WORLD);
    info("Closed BigFile\n");

    MPI_Comm_free(&COMM_SPLIT);
    free(fakedata);
}

int main(int argc, char * argv[]) {
    MPI_Init(&argc, &argv);
    
    MPI_Comm_rank(MPI_COMM_WORLD, &ThisTask);
    MPI_Comm_size(MPI_COMM_WORLD, &NTask);
    
    int i;
    int ch;
    int width = 1;
    int Nfile = 1;
    int Nwriter = NTask;
    size_t Nitems = 1024;
    char * filename = alloca(1500);
    char * path = "";
    char * postfix = alloca(1500);
    int mode = MODE_CREATE;
    //+++++++++++++++++ Timelog +++++++++++++++++
    char * timelog = alloca(1000);
    double * tlog_ranks;
    tlog_ranks = (double *) malloc(sizeof(double)*NTask);
    FILE *F;

    while(-1 != (ch = getopt(argc, argv, "hN:n:s:w:p:m:"))) {
        switch(ch) {
            case 'm':
                if(0 == strcmp(optarg, "read")) {
                    mode = MODE_READ;
                } else
                if(0 == strcmp(optarg, "create")) {
                    mode = MODE_CREATE;
                } else
                if(0 == strcmp(optarg, "update")) {
                    mode = MODE_UPDATE;
                } else {
                    usage();
                    goto byebye;
                }
                break;
            case 'p':
                path = optarg;
                if( path[0] == '-') {
                    usage();
                    goto byebye;
                }
                break;
            case 'w':
                if(1 != sscanf(optarg, "%d", &width)) {
                    usage();
                    goto byebye;
                }
                break;
            case 'N':
                if(1 != sscanf(optarg, "%d", &Nfile)) {
                    usage();
                    goto byebye;
                }
                break;
            case 'n':
                if(1 != sscanf(optarg, "%d", &Nwriter)) {
                    usage();
                    goto byebye;
                }
                break;
            case 's':
                if(1 != sscanf(optarg, "%td", &Nitems)) {
                    usage();
                    goto byebye;
                }
                break;
            case 'h':
            default:
                usage();
                goto byebye;
        }    
    }
    if(optind == argc) {
        usage();
        goto byebye;
    }

//+++++++++++++++++ Filename and checks of input parameters +++++++++++++++++
    sprintf(postfix, "_files%d_ranks%d_writers%d_items%td_of_width%d",
             Nfile, NTask, Nwriter, Nitems, width);
    sprintf(filename, "%s%s%s", path, argv[optind], postfix);
    Nitems *= width;
    if (Nitems % NTask != 0) {
        Nitems -= Nitems % NTask;
        info("#items not divisible by ranks!\n Overriding total#items = %td\n", Nitems);
    }
    if (Nwriter > NTask) {
        info("\n\n ############## CAUTION: you chose %d ranks and %d writers! ##############\n"
             " #  If you want %d writers, allocate at least %d ranks with <mpirun -n %d> #\n"
             " ################### Can only use %d writers instead! ###################\n\n",
             NTask, Nwriter, Nwriter, Nwriter, Nwriter, NTask);
        Nwriter = NTask;
    }
    
//+++++++++++++++++ Starting Simulation +++++++++++++++++
    sim(Nfile, Nwriter, Nitems, filename, tlog_ranks, mode);

//+++++++++++++++++ Writing Time Log +++++++++++++++++
    sprintf(timelog, "%s/Timelog%s", filename, postfix);
    F = fopen(timelog, "a+");
    if (!F){
        info("iosim.c: Couldn't open file %s for writting!\n", timelog);
    }
    else{
        if (ThisTask == 0){
            for (i=0; i<NTask; i++) {
                fprintf(F, "Task=%d\ttwrite=\t%f\n", i, tlog_ranks[i]);
            }
        }
        fclose(F);
    }

byebye:
    MPI_Finalize();
//    free(filename); free(timelog);
    free(tlog_ranks);
    return 0;
}
