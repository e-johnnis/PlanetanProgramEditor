#include<ppe.hpp>
#include<cstdio>
#include<cstring>
#include<cmath>
#include<unistd.h>

void showHelp();

int main(int argc, char** argv) {
    ppe::GraphicsConfig config = {};
    int verbose = 0;

    std::sprintf(config.outputFileName, "ppe-result.avi");
    config.width = 0;

    int copt = -1;
    while((copt = getopt(argc, argv, "hvo:f:p:w:")) != -1) {
        switch(copt) {
            case 'h':
                showHelp();
                return 0;
            case 'v':
                verbose = 1;
                break;
            case 'o':
                if(strlen(optarg) >= PPE_CHAR_MAX) {
                    std::fprintf(stderr, "[!] too long output file name. must be less than %d chars.\n", PPE_CHAR_MAX);
                    return -1;
                }else {
                    std::strcpy(config.outputFileName, optarg);
                }
                break;
            case 'f':
                if(!std::sscanf(optarg, "%lf", &config.fps)) {
                    std::fprintf(stderr, "[!] invalid value for video fps: %s\n", optarg);
                    return -1;
                }
                break;
            case 'p':
                if(!(std::sscanf(optarg, "%d", &config.height) && (config.height > 0))) {
                    std::fprintf(stderr, "[!] invalid value for video height: %s\n", optarg);
                    return -1;
                }
                break;
            case 'w':
                if(!(std::sscanf(optarg, "%d", &config.width) && (config.width > 0))) {
                    std::fprintf(stderr, "[!] invalid value for video width: %s\n", optarg);
                    return -1;
                }
                break;
            case '?':
                std::fprintf(stderr, "[!] unknown key '%c'. see help (\"ppe -h\" to show).\n", optopt);
                return -1;
        }
    }

    if(config.width <= 0) config.width = 16 * config.height / 9;

    if(optind >= argc) {
        std::fprintf(stderr, "[!] no argument for input file.\n");
        return -2;
    }else if(strlen(argv[optind]) >= PPE_CHAR_MAX) {
        std::fprintf(stderr, "[!] too long input file name. must be less than %d chars.\n", PPE_CHAR_MAX);
        return -2;
    }

    std::strcpy(config.inputFileName, argv[optind]);

    if(verbose) {
        std::printf("%s\n", PPE_VERSION_STR);
        std::printf("\n");
        std::printf("settings:\n");
        std::printf("  inputFileName  : %s\n", config.inputFileName);
        std::printf("  outputFileName : %s\n", config.outputFileName);
        std::printf("  video size     : %dx%d, %5.2ffps\n", config.width, config.height, config.fps);
        std::printf("\n");
    }

    ppe::Graphics graphics(&config);
    int err = ppe::SUCCESS;
    int nline = 0, nevent = 0;

    if((err = graphics.loadStars())) {
        std::fprintf(stderr, "[!] failed to load star data: %s\n", ppe::errorString(err));
        return -3;
    }
    if((err = graphics.loadConsts())) {
        std::fprintf(stderr, "[!] failed to load constellation line data: %s\n", ppe::errorString(err));
        return -3;
    }
    
    if((err = graphics.loadEvents(&nline, &nevent))) {
        std::fprintf(stderr, "[!] error in line %d: %s\n", nline, ppe::errorString(err));
        return -4;
    }

    if(verbose) {
        std::printf("events loaded: %d\n", nevent);
    }

    int ehour = (int)graphics.endTime / 3600;
    int emin = ((int)graphics.endTime % 3600) / 60;
    int esec = (int)graphics.endTime % 60;
    int emsec = (int)std::round(graphics.endTime * 1000.0) % 1000;

    double frameTime = 0;

    while(graphics.render(&frameTime)) if(verbose) {
        int hour = (int)frameTime / 3600;
        char min = ((int)frameTime % 3600) / 60;
        char sec = (int)frameTime % 60;
        short msec = (int)std::round(frameTime * 1000.0) % 1000;

        std::printf(
            "\rexporting: %02d:%02d:%02d.%03d / %02d:%02d:%02d.%03d",
            hour, min, sec, msec,
            ehour, emin, esec, emsec
        );
        std::fflush(stdout);
    }
    if(verbose) std::printf("\n");

    return 0;
}

void showHelp() {
    std::printf("useage: ppe [options] input_file.ppc\n");
    std::printf("\n");
    std::printf("*** options ***\n");
    std::printf("-o [string] : set output file name (*.avi)\n");
    std::printf("-f [float]  : set video fps (default=30)\n");
    std::printf("-p [int]    : set video height (default=1080)");
    std::printf("-w [int]    : set video width (default=height*16/9)");
    std::printf("-v          : show verbose while exporting\n");
    std::printf("-h          : show help and exit\n");
}