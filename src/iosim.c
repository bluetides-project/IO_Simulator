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
sim(int Nfiles, int Nwriters, size_t bytesperrank) 
{
    size_t size = bytesperrank / 4;
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

int main(int argc, char * argv[]) {
    MPI_Init(&argc, &argv);
    
    MPI_Comm_rank(MPI_COMM_WORLD, &ThisTask);
    MPI_Comm_size(MPI_COMM_WORLD, &NTask);

    int ch;
    int Nfiles = 1;
    int Nwriters = NTask;
    size_t bytesperrank = 1024;
    char * filename;

    while(-1 != (ch = getopt(argc, argv, "hN:n:s:"))) {
        switch(ch) {
            case 'N':
                if(1 != sscanf(optarg, "%d", &Nfiles)) {
                    usage();
                    goto byebye;
                }
                break;
            case 'n':
                if(1 != sscanf(optarg, "%d", &Nwriters)) {
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

    info("Writing to `%s`\n", filename);

byebye:
    MPI_Finalize();
    return 0;
    return 0;
}
