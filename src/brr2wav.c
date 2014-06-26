/**
 * brr2wav.c: decodes snes brr into Microsft WAVE.
 * @author      gocha - http://gocha.s151.xrea.com/
 */

#define APP_NAME    "brr2wav"
#define APP_VER     "(2010-02-23)"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <direct.h>
#include "cbyteio.h"

static char g_baseName[PATH_MAX];
static char g_fileName[PATH_MAX];
static int  g_pitch = 0x1000;

static INLINE int32 absolute (int32 x)
{
    return ((x < 0) ? -x : x);
}

static INLINE int32 sclip15 (int32 x)
{
    return ((x & 16384) ? (x | ~16383) : (x & 16383));
}

static INLINE int32 sclamp8 (int32 x)
{
    return ((x > 127) ? 127 : (x < -128) ? -128 : x);
}

static INLINE int32 sclamp15 (int32 x)
{
    return ((x > 16383) ? 16383 : (x < -16384) ? -16384 : x);
}

static INLINE int32 sclamp16 (int32 x)
{
    return ((x > 32767) ? 32767 : (x < -32768) ? -32768 : x);
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

#define SNES_BRRBLOCK_SIZE      9
#define SNES_BRRBLOCK_SAMPLES   16

bool brr2wav (const char *path)
{
    FILE *brrFile;
    FILE *wavFile = NULL;
    char wavPath[PATH_MAX];
    int32 sampCount;
    int32 prev[2];
    int32 rate;
    size_t brrSize;
    bool addLoopInfo = false;
    int loopStart = 0;
    int loopEnd = 0;

    setFileNameBase(path);

    brrFile = fopen(path, "rb");
    if (!brrFile) {
        fprintf(stderr, "%s: error: unable to open.\n", g_fileName);
        goto fin;
    }
    fseek(brrFile, 0, SEEK_END);
    brrSize = (size_t) ftell(brrFile);
    rewind(brrFile);

    switch (brrSize % 9) {
    case 0:
        break;
    case 2: {
        int loopOffset = fget2l(brrFile);
        int loopPoint = loopOffset / 9 * 16;
        if (loopOffset % 9 == 0)
            printf("%s: addmusicM header detected, loop offset = %04xh (sample #%d).\n", g_fileName, loopOffset, loopPoint);
        else
            fprintf(stderr, "%s: warning: illegal length, skip first %d bytes.\n", g_fileName, brrSize % 9);

        loopStart = loopPoint;
        addLoopInfo = true;
        break;
    }
    default:
        fprintf(stderr, "%s: warning: illegal length, skip first %d bytes.\n", g_fileName, brrSize % 9);
        fseek(brrFile, brrSize % 9, SEEK_CUR);
    }

    sprintf(wavPath, "%s.wav", g_baseName);
    wavFile = fopen(wavPath, "wb");
    if (!wavFile) {
        fprintf(stderr, "%s: error: unable to create wave file.\n", g_fileName);
        goto fin;
    }

    rate = g_pitch * 32000 / 0x1000;

    // output RIFF WAVE header first
    fput1('R', wavFile); fput1('I', wavFile); fput1('F', wavFile); fput1('F', wavFile);
    fput4l(0, wavFile); // 0x04: put actual file size later
    fput1('W', wavFile); fput1('A', wavFile); fput1('V', wavFile); fput1('E', wavFile);
    fput1('f', wavFile); fput1('m', wavFile); fput1('t', wavFile); fput1(' ', wavFile);
    fput4l(16, wavFile);
    fput2l(1, wavFile);
    fput2l(1, wavFile);
    fput4l(rate, wavFile);
    fput4l(rate * 2, wavFile);
    fput2l(2, wavFile);
    fput2l(16, wavFile);
    fput1('d', wavFile); fput1('a', wavFile); fput1('t', wavFile); fput1('a', wavFile);
    fput4l(0, wavFile); // 0x28: put actual sample size later

    // start decoding
    printf("%s: start decoding...\n", g_fileName);
    prev[0] = prev[1] = 0;
    sampCount = 0;
    while (true) {
        uint8   brrBlock[SNES_BRRBLOCK_SIZE];
        uint8   filter;
        bool    lastBlock;
        bool    loop;
        int32   out, S1, S2;
        uint8   shift;
        int8    sample1, sample2;
        bool    validHeader;
        int     i, nybble;
        byte    *compress = brrBlock;

        if (fread(brrBlock, 1, SNES_BRRBLOCK_SIZE, brrFile) != SNES_BRRBLOCK_SIZE) {
            fprintf(stderr, "%s: warning: conversion aborted, missing samples or end flag.\n", g_fileName);
            addLoopInfo = false;
            break;
        }

        filter = (uint8) *compress++;
        lastBlock = filter & 1;
        loop = (filter & 2) != 0;

        shift = filter >> 4;
        validHeader = (shift < 0xD);
        filter &= 0x0C;

        S1 = prev[0];
        S2 = prev[1];

        for (i = 0; i < 8; i++)
        {
            sample1 = *compress++;
            sample2 = sample1 << 4;
            sample1 >>= 4;
            sample2 >>= 4;

            for (nybble = 0; nybble < 2; nybble++)
            {
                out = nybble ? (int32) sample2 : (int32) sample1;
                out = validHeader ? ((out << shift) >> 1) : (out & ~0x7FF);

                switch (filter)
                {
                    case 0x00: // Direct
                        break;

                    case 0x04: // 15/16
                        out += S1 + ((-S1) >> 4);
                        break;

                    case 0x08: // 61/32 - 15/16
                        out += (S1 << 1) + ((-((S1 << 1) + S1)) >> 5) - S2 + (S2 >> 4);
                        break;

                    case 0x0C: // 115/64 - 13/16
                        out += (S1 << 1) + ((-(S1 + (S1 << 2) + (S1 << 3))) >> 6) - S2 + (((S2 << 1) + S2) >> 4);
                        break;
                }

                out = sclip15(sclamp16(out));

                S2 = S1;
                S1 = out;

                fput2l(out << 1, wavFile);
                sampCount++;
            }
        }

        prev[0] = S1;
        prev[1] = S2;

        if (lastBlock) {
            if (!loop) {
                addLoopInfo = false;
            }
            loopEnd = sampCount - 1;
            break;
        }
    }

    // add loop point info (smpl chunk) if needed... details:
    // en: http://www.blitter.com/~russtopia/MIDI/~jglatt/tech/wave.htm
    // ja: http://co-coa.sakura.ne.jp/index.php?Sound%20Programming%2FWave%20File%20Format
    if (addLoopInfo) {
        fput1('s', wavFile); fput1('m', wavFile); fput1('p', wavFile); fput1('l', wavFile); // sampler chunk
        fput4l(60, wavFile); // chunk size
        fput4l(0, wavFile);  // manufacturer
        fput4l(0, wavFile);  // product
        fput4l(1000000000/rate, wavFile); // sample period
        fput4l(60, wavFile); // MIDI uniti note (C5)
        fput4l(0, wavFile);  // MIDI pitch fraction
        fput4l(0, wavFile);  // SMPTE format
        fput4l(0, wavFile);  // SMPTE offset
        fput4l(1, wavFile);  // sample loops
        fput4l(0, wavFile);  // sampler data
        fput4l(0, wavFile);  // cue point ID
        fput4l(0, wavFile);  // type (loop forward)
        fput4l(loopStart, wavFile); // start sample #
        fput4l(loopEnd, wavFile); // end sample #
        fput4l(0, wavFile);  // fraction
        fput4l(0, wavFile);  // playcount
    }

    // finalize
    fseek(wavFile, 0x04, SEEK_SET);
    fput4l(0x24 + sampCount * 2 + (addLoopInfo ? 68 : 0), wavFile); // 0x04: put actual file size later
    fseek(wavFile, 0x28, SEEK_SET);
    fput4l(sampCount * 2, wavFile); // 0x28: put actual sample size later

    fclose(wavFile);
    fclose(brrFile);
    return true;

fin:
    if (wavFile)
        fclose(wavFile);
    if (brrFile)
        fclose(brrFile);
    return false;
}

void usage (void)
{
    printf("%s %s: decodes snes brr into Microsoft WAVE.\n", APP_NAME, APP_VER);
    puts("programmed by gocha - http://gocha.s151.xrea.com/");
    puts("if you want more features, you may want to use snesbrr.");
    puts("");
    puts("Options:");
    puts("  -p  --pitch n     specify pitch (0x1000 = 1.0).");
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
            else if (strcmp(argv[*argi], "--pitch") == 0) {
                if (*argi + 1 < argc) {
                    g_pitch = (int) strtol(argv[*argi + 1], NULL, 0);
                    (*argi)++;
                }
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
                  case 'p':
                    if (i == 1 && argv[*argi][i + 1] == '\0' && *argi + 1 < argc) {
                        g_pitch = (int) strtol(argv[*argi + 1], NULL, 0);
                        (*argi)++;
                        goto skipOpt;
                    }
                    else
                        fprintf(stderr, "error: wrong use of -p.\n");
                    break;
                  default:
                    fprintf(stderr, "warning: unknown option \"%s\".\n", argv[*argi]);
                }
                i++;
            }
skipOpt: ;
        }
    }
}

int main (int argc, char **argv)
{
    int argi;
    char dir[PATH_MAX];

    if (argc == 1) {
        printf("%s %s: decodes snes brr into Microsoft WAVE.\n", APP_NAME, APP_VER);
        puts("run with --help to show details.");
        return EXIT_SUCCESS;
    }

    _getcwd(dir, countof(dir));

    dispatchOpt(argc, argv, &argi);
    for (; argi < argc; argi++) {
        _chdir(dir);
        if (!brr2wav(argv[argi])) {
            return EXIT_FAILURE;
        }
    }
    return EXIT_SUCCESS;
}
