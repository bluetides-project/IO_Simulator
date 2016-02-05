#include <stdio.h>
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
    printf("usage: iosim -N nfiles -n numwriters -s bytesperrank filename\n");
}

static void 
info(char * fmt, ...) {

    static double t0 = -1.0;

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

static void 
sim(int Nfile, int Nwriter, size_t bytesperrank, char * filename) 
{
    size_t size = bytesperrank / 4;

    info("Writing to `%s`\n", filename);
    info("Physical Files %d\n", Nfile);
    info("Ranks %d\n", NTask);
    info("Writers %d\n", Nwriter);
    info("Bytes Per Rank %td\n", size * 4);

    BigFile bf = {0};
    BigBlock bb = {0};
    big_file_mpi_create(&bf, filename, MPI_COMM_WORLD);
    big_file_mpi_create_block(&bf, &bb, "TestBlock", "i4", 1, Nfile, size * NTask, MPI_COMM_WORLD);

    big_block_mpi_close(&bb, MPI_COMM_WORLD);
    big_file_mpi_close(&bf, MPI_COMM_WORLD);
}


int main(int argc, char * argv[]) {
    MPI_Init(&argc, &argv);
    
    MPI_Comm_rank(MPI_COMM_WORLD, &ThisTask);
    MPI_Comm_size(MPI_COMM_WORLD, &NTask);

    int ch;
    int Nfile = 1;
    int Nwriter = NTask;
    size_t bytesperrank = 1024;
    char * filename;

    while(-1 != (ch = getopt(argc, argv, "hN:n:s:"))) {
        switch(ch) {
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
                if(1 != sscanf(optarg, "%td", &bytesperrank)) {
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
    filename = argv[optind];
    sim(Nfile, Nwriter, bytesperrank, filename);

byebye:
    MPI_Finalize();
    return 0;
    return 0;
}
