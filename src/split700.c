/**
 * split700.c: extracts brr samples from spc dump
 * @author      gocha - http://gocha.s151.xrea.com/
 * @version     2009-08-26
 */

#define APP_NAME    "split700"
#define APP_VER     "(2009-08-26)"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <direct.h>
#include "cbyteio.h"

static char g_baseName[PATH_MAX];
static char g_fileName[PATH_MAX];
static bool g_force = false;
static int  g_leastBlockNum = 0;
static bool g_ignoreSampAtZero = true;
static bool g_dumpSamples = true;
static int  g_dumpFrom = 0;
static int  g_dumpTo = 255;
static bool g_FilenameWithloopInfo = false;
static bool g_WriteHeaderForAddmusicM = false;

bool dumpPartOfFile (const char *path, int startPos, int endPos, FILE *file)
{
    FILE *outFile;
    int pos;
    int c;

    if (startPos > endPos)
        return false;

    outFile = fopen(path, "wb");
    if (!outFile)
        return false;

    fseek(file, startPos, SEEK_SET);
    for (pos = startPos; pos <= endPos; pos++) {
        c = fgetc(file);
        if (c == EOF) {
            fclose(outFile);
            return false;
        }
        fputc(c, outFile);
    }

    fclose(outFile);
    return true;
}

bool dumpBrrFile (const char *path, int startPos, int endPos, int loopAddrRel, FILE *file)
{
    FILE *outFile;
    int pos;
    int c;

    if (startPos > endPos)
        return false;

    outFile = fopen(path, "wb");
    if (!outFile)
        return false;

    if (g_WriteHeaderForAddmusicM) {
        fput2l(loopAddrRel, outFile);
    }

    fseek(file, startPos, SEEK_SET);
    for (pos = startPos; pos <= endPos; pos++) {
        c = fgetc(file);
        if (c == EOF) {
            fclose(outFile);
            return false;
        }
        fputc(c, outFile);
    }

    fclose(outFile);
    return true;
}

void setFileNameBase (const char *path)
{
    char drive[PATH_MAX];
    char dir[PATH_MAX];
    char fname[PATH_MAX];
    char ext[PATH_MAX];

    _splitpath(path, drive, dir, fname, ext);
    _makepath(g_baseName, drive, dir, fname, NULL);
    _makepath(g_fileName, NULL, NULL, fname, ext);
}

bool isSpcFile (FILE *file)
{
    size_t size;
    const byte validSig[0x1b] = {
        'S','N','E','S','-','S','P','C','7','0','0',' ','S','o','u','n',
        'd',' ','F','i','l','e',' ','D','a','t','a',
    };
    byte sig[0x23];

    fseek(file, 0, SEEK_END);
    size = (size_t) ftell(file);
    rewind(file);

    if (size < 0x10200)
        return false;

    rewind(file);
    if (!fread(sig, 0x23, 1, file))
        return false;
    if (memcmp(sig, validSig, 0x1b) != 0)
        return false;

    rewind(file);
    return true;
}

#define SPC_ARAM_OFFSET         0x100
#define SPC_ARAM_SIZE           0x10000
#define SNES_BRRBLOCK_SIZE      9
#define SNES_BRRBLOCK_SAMPLES   16

bool spcGetSampInfo (FILE *file, int sampStartAddr, int sampLoopAddr, int *sampBlockNum, bool *sampLooped)
{
    bool validSample = true;
    byte brrBlock[SNES_BRRBLOCK_SIZE];
    int blockAddr = sampStartAddr;
    int sampLastAddr;
    bool loopFlag;
    //bool lastLoopFlag;

    while ((blockAddr + SNES_BRRBLOCK_SIZE) <= SPC_ARAM_SIZE) {
        int shift;

        fseek(file, SPC_ARAM_OFFSET + blockAddr, SEEK_SET);
        fread(brrBlock, SNES_BRRBLOCK_SIZE, 1, file);

        shift = brrBlock[0] >> 4;
        if (shift > 0x0C) {  // out of range
            validSample = false;
        }

        loopFlag = (bool) (brrBlock[0] & 2);
        //if (blockAddr != sampStartAddr && loopFlag != lastLoopFlag)
        //    validSample = false;
        //lastLoopFlag = loopFlag;

        if (brrBlock[0] & 1) // END
            break;

        blockAddr += SNES_BRRBLOCK_SIZE;
    }
    if ((blockAddr + SNES_BRRBLOCK_SIZE) > SPC_ARAM_SIZE) {
        if (!g_force)
            return false;   // out of ARAM, totally impossible
        else {
            while ((blockAddr + SNES_BRRBLOCK_SIZE) > SPC_ARAM_SIZE)
                blockAddr -= SNES_BRRBLOCK_SIZE;
        }
    }
    if (!g_force) {
        if (loopFlag && (sampStartAddr > sampLoopAddr || sampLoopAddr > blockAddr))
            validSample = false;
        if ((sampLoopAddr - sampStartAddr) % SNES_BRRBLOCK_SIZE != 0)
            validSample = false;
    }

    sampLastAddr = blockAddr + SNES_BRRBLOCK_SIZE;
    *sampBlockNum = (sampLastAddr - sampStartAddr) / SNES_BRRBLOCK_SIZE;
    *sampLooped = loopFlag;
    return !g_force ? validSample : true;
}

bool split700 (const char *path)
{
    FILE *spcFile;
    byte dspRegs[128];
    int loopPoint;
    int sampDir;
    int samp;
    static bool firstTime = true;
    char pathTemp[PATH_MAX];

    if (firstTime) {
        firstTime = false;
        printf("\n");
    }
    setFileNameBase(path);

    spcFile = fopen(path, "rb");
    if (!spcFile) {
        fprintf(stderr, "%s: error: unable to open.\n", g_fileName);
        goto fin;
    }

    if (!isSpcFile(spcFile)) {
        fprintf(stderr, "%s: error: corrupt spc file.\n", g_fileName);
        goto fin;
    }

    fseek(spcFile, 0x10100, SEEK_SET);
    if (!fread(dspRegs, 128, 1, spcFile)) {
        fprintf(stderr, "%s: error: unable to read dsp regs.\n", g_fileName);
        goto fin;
    }

    sampDir = mget2l(&dspRegs[0x5d]) * 0x100;
    printf("%s: sample dir address = $%04x\n", g_fileName, sampDir);

    for (samp = g_dumpFrom; samp <= g_dumpTo; samp++)
    {
        int sampEntryAddr = sampDir + (samp * 4);
        int sampStartAddr, sampLoopAddr;
        int sampBlockNum;
        bool sampLooped;
        bool validSample;
        char brrFilePath[PATH_MAX];

        fseek(spcFile, SPC_ARAM_OFFSET + sampEntryAddr, SEEK_SET);
        sampStartAddr = fget2l(spcFile);
        sampLoopAddr = fget2l(spcFile);
        loopPoint = (sampLoopAddr - sampStartAddr) / SNES_BRRBLOCK_SIZE * SNES_BRRBLOCK_SAMPLES;

        validSample = spcGetSampInfo(spcFile, sampStartAddr, sampLoopAddr, &sampBlockNum, &sampLooped);
        if (!validSample)
            continue;
        if (sampBlockNum < g_leastBlockNum)
            continue;
        if (!g_force && g_ignoreSampAtZero && sampStartAddr == 0)
            continue;

        sprintf(brrFilePath, "%s_%02x", g_baseName, samp);
        if (g_FilenameWithloopInfo) {
            if (sampLooped) {
                sprintf(pathTemp, "-loop%d", loopPoint);
                strcat(brrFilePath, pathTemp);
            }
            else
                strcat(brrFilePath, "-noloop");
        }
        strcat(brrFilePath, ".brr");
        if (g_dumpSamples) {
            dumpBrrFile(brrFilePath, SPC_ARAM_OFFSET + sampStartAddr,
                SPC_ARAM_OFFSET + sampStartAddr + (sampBlockNum * SNES_BRRBLOCK_SIZE) - 1,
                sampLoopAddr - sampStartAddr, spcFile);
        }

        printf("%s: voice $%02x: SA/LSA %04X/%04X, %4d blocks", g_fileName, samp, sampStartAddr, sampLoopAddr, sampBlockNum);
        if (sampLooped) {
            printf(", loop = sample #%d", loopPoint);
        }
        printf("\n");
    }

    fclose(spcFile);
    return true;

fin:
    if (spcFile)
        fclose(spcFile);
    return false;
}

void usage (void)
{
    printf("%s %s: extracts brr samples from spc dump.\n", APP_NAME, APP_VER);
    puts("programmed by gocha - http://gocha.s151.xrea.com/");
    puts("if you want to decode brr, try snesbrr. it's one of nice brr decoders.");
    puts("");
    puts("Options:");
    puts("  -f  --force       force output corrupt samples.");
    puts("  -l  --list        display voice list, but do not dump any brrs.");
    puts("      --least n     assume a brr sample has at least n blocks.");
    puts("  -L                add loop point info to output filename of the sample.");
    puts("  -M                add a simple header for addmusicM.");
    puts("  -z  --allow-zero  ignore samples which starts from $0000.");
    puts("  -?  --help        display this help.");
}

void dispatchOpt (int argc, char **argv, int *argi)
{
    for (*argi = 1; *argi < argc; (*argi)++) {
        if (argv[*argi][0] != '-')
            break;

        if (argv[*argi][1] == '-') {
            if (strcmp(argv[*argi], "--help") == 0) {
                usage();
            }
            else if (strcmp(argv[*argi], "--force") == 0) {
                g_force = true;
            }
            else if (strcmp(argv[*argi], "--list") == 0) {
                g_dumpSamples = false;
            }
            else if (strcmp(argv[*argi], "--least") == 0) {
                if (*argi + 1 < argc) {
                    g_leastBlockNum = atoi(argv[*argi + 1]);
                    (*argi)++;
                }
            }
            else if (strcmp(argv[*argi], "--allow-zero") == 0) {
                g_ignoreSampAtZero = false;
            }
            else {
                fprintf(stderr, "warning: unknown option \"%s\".\n", argv[*argi]);
            }
        }
        else {
            int i = 1;
            while (argv[*argi][i] != '\0') {
                char c = argv[*argi][i];

                switch (c) {
                  case '?':
                    usage();
                    break;
                  case 'f':
                    g_force = true;
                    break;
                  case 'l':
                    g_dumpSamples = false;
                    break;
                  case 'L':
                    g_FilenameWithloopInfo = true;
                    break;
                  case 'M':
                    g_WriteHeaderForAddmusicM = true;
                    break;
                  case 'z':
                    g_ignoreSampAtZero = false;
                    break;
                  default:
                    fprintf(stderr, "warning: unknown option \"%s\".\n", argv[*argi]);
                }
                i++;
            }
        }
    }
}

int main (int argc, char **argv)
{
    int argi;
    char dir[PATH_MAX];

    if (argc == 1) {
        printf("%s %s: extracts brr samples from spc dump.\n", APP_NAME, APP_VER);
        puts("run with --help to show details.");
        return EXIT_SUCCESS;
    }

    _getcwd(dir, countof(dir));

    dispatchOpt(argc, argv, &argi);
    for (; argi < argc; argi++) {
        _chdir(dir);
        if (!split700(argv[argi])) {
            return EXIT_FAILURE;
        }
    }
    return EXIT_SUCCESS;
}
