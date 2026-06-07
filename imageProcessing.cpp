// ============================================================
//  Parallel Image Processor — OpenMP Demo
//  CSC 426 Parallel Programming | Group 1
//  Filters: Grayscale, Brightness, Sepia, Sharpen,
//           Box Blur, Gaussian Blur, Edge Detection, Emboss,
//           Negative, Pixelate, Vignette, Channel Swap,
//           Threshold, Warm Tone, Cool Tone
// ============================================================

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define UNICODE
#define _UNICODE

#include "stb_image.h"
#include "stb_image_write.h"
#include <omp.h>
#include <windows.h>
#include <commdlg.h>
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>
#include <functional>
#include <sstream>
#include <iomanip>
#include <thread>

// ============================================================
//  CONTROL IDs
// ============================================================
#define ID_BTN_BROWSE         101
#define ID_BTN_RUN            102
#define ID_BTN_OPEN_FOLDER    103
#define ID_EDIT_PATH          104
#define ID_RESULTS            105
#define ID_EDIT_THREADS       106
#define ID_LBL_TITLE      401
#define ID_LBL_IMAGE      402
#define ID_LBL_THREADS    403
#define ID_LBL_FILTERS    404
#define ID_LBL_RESULTS    405

// Filter checkboxes
#define ID_CHK_GRAYSCALE      201
#define ID_CHK_BRIGHTNESS     202
#define ID_CHK_SEPIA          203
#define ID_CHK_SHARPEN        204
#define ID_CHK_BOXBLUR        205
#define ID_CHK_GAUSSIAN       206
#define ID_CHK_EDGE           207
#define ID_CHK_EMBOSS         208
#define ID_CHK_NEGATIVE       209
#define ID_CHK_PIXELATE       210
#define ID_CHK_VIGNETTE       211
#define ID_CHK_CHANSWAP       212
#define ID_CHK_THRESHOLD      213
#define ID_CHK_WARM           214
#define ID_CHK_COOL           215
#define ID_CHK_ALL            216

// Intensity dropdowns
#define ID_CMB_SHARPEN        301
#define ID_CMB_BOXBLUR        302
#define ID_CMB_GAUSSIAN       303
#define ID_CMB_EMBOSS         304
#define ID_CMB_VIGNETTE       305
#define ID_CMB_PIXELATE       306

// ============================================================
//  GLOBALS
// ============================================================
HWND hWnd, hEditPath, hResults, hBtnRun, hBtnOpenFolder;
HWND hEditThreads;

HWND hChkGray, hChkBright, hChkSepia, hChkSharpen;
HWND hChkBox,  hChkGauss,  hChkEdge,  hChkEmboss;
HWND hChkNeg,  hChkPix,    hChkVig,   hChkChan;
HWND hChkThresh, hChkWarm, hChkCool,  hChkAll;

HWND hCmbSharpen, hCmbBoxBlur, hCmbGaussian;
HWND hCmbEmboss,  hCmbVignette, hCmbPixelate;

std::wstring g_imagePath = L"";
std::wstring g_outputDir = L"";

// ============================================================
//  IMAGE STRUCT
// ============================================================
inline unsigned char clamp(int v) {
    return (unsigned char)std::max(0, std::min(255, v));
}

struct Image {
    int width, height, channels;
    std::vector<unsigned char> data;

    unsigned char get(int row, int col, int ch) const {
        return data[(row * width + col) * channels + ch];
    }
    void set(int row, int col, int ch, unsigned char val) {
        data[(row * width + col) * channels + ch] = val;
    }
};

bool loadImage(const std::wstring& path, Image& img) {
    int size = WideCharToMultiByte(CP_UTF8, 0, path.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string narrow(size - 1, 0);
    WideCharToMultiByte(CP_UTF8, 0, path.c_str(), -1, &narrow[0], size, nullptr, nullptr);
    int w, h, c;
    unsigned char* raw = stbi_load(narrow.c_str(), &w, &h, &c, 3);
    if (!raw) return false;
    img.width = w; img.height = h; img.channels = 3;
    img.data.assign(raw, raw + w * h * 3);
    stbi_image_free(raw);
    return true;
}

bool saveImage(const std::wstring& path, const Image& img) {
    int size = WideCharToMultiByte(CP_UTF8, 0, path.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string narrow(size - 1, 0);
    WideCharToMultiByte(CP_UTF8, 0, path.c_str(), -1, &narrow[0], size, nullptr, nullptr);
    return stbi_write_png(narrow.c_str(), img.width, img.height,
        img.channels, img.data.data(), img.width * img.channels) != 0;
}

// ============================================================
//  FILTERS
// ============================================================

// 1. Grayscale
void grayscale(const Image& src, Image& dst, bool parallel) {
    dst = src;
    #pragma omp parallel for schedule(static) if(parallel)
    for (int i = 0; i < src.height; i++)
        for (int j = 0; j < src.width; j++) {
            unsigned char r = src.get(i,j,0), g = src.get(i,j,1), b = src.get(i,j,2);
            unsigned char gray = clamp((int)(0.299f*r + 0.587f*g + 0.114f*b));
            dst.set(i,j,0,gray); dst.set(i,j,1,gray); dst.set(i,j,2,gray);
        }
}

// 2. Brightness
void brightness(const Image& src, Image& dst, bool parallel) {
    dst = src;
    #pragma omp parallel for schedule(static) if(parallel)
    for (int i = 0; i < src.height; i++)
        for (int j = 0; j < src.width; j++) {
            dst.set(i,j,0, clamp(src.get(i,j,0) + 60));
            dst.set(i,j,1, clamp(src.get(i,j,1) + 60));
            dst.set(i,j,2, clamp(src.get(i,j,2) + 60));
        }
}

// 3. Sepia
void sepia(const Image& src, Image& dst, bool parallel) {
    dst = src;
    #pragma omp parallel for schedule(static) if(parallel)
    for (int i = 0; i < src.height; i++)
        for (int j = 0; j < src.width; j++) {
            int r=src.get(i,j,0), g=src.get(i,j,1), b=src.get(i,j,2);
            dst.set(i,j,0, clamp((int)(0.393f*r+0.769f*g+0.189f*b)));
            dst.set(i,j,1, clamp((int)(0.349f*r+0.686f*g+0.168f*b)));
            dst.set(i,j,2, clamp((int)(0.272f*r+0.534f*g+0.131f*b)));
        }
}

// 4. Negative
void negative(const Image& src, Image& dst, bool parallel) {
    dst = src;
    #pragma omp parallel for schedule(static) if(parallel)
    for (int i = 0; i < src.height; i++)
        for (int j = 0; j < src.width; j++) {
            dst.set(i,j,0, 255 - src.get(i,j,0));
            dst.set(i,j,1, 255 - src.get(i,j,1));
            dst.set(i,j,2, 255 - src.get(i,j,2));
        }
}

// 5. Threshold (binary black & white)
void threshold(const Image& src, Image& dst, bool parallel) {
    dst = src;
    #pragma omp parallel for schedule(static) if(parallel)
    for (int i = 0; i < src.height; i++)
        for (int j = 0; j < src.width; j++) {
            int r=src.get(i,j,0), g=src.get(i,j,1), b=src.get(i,j,2);
            unsigned char gray = (unsigned char)(0.299f*r + 0.587f*g + 0.114f*b);
            unsigned char val  = gray > 128 ? 255 : 0;
            dst.set(i,j,0,val); dst.set(i,j,1,val); dst.set(i,j,2,val);
        }
}

// 6. Channel Swap (R->G->B->R)
void channelSwap(const Image& src, Image& dst, bool parallel) {
    dst = src;
    #pragma omp parallel for schedule(static) if(parallel)
    for (int i = 0; i < src.height; i++)
        for (int j = 0; j < src.width; j++) {
            unsigned char r=src.get(i,j,0), g=src.get(i,j,1), b=src.get(i,j,2);
            dst.set(i,j,0,g); dst.set(i,j,1,b); dst.set(i,j,2,r);
        }
}

// 7. Warm Tone
void warmTone(const Image& src, Image& dst, bool parallel) {
    dst = src;
    #pragma omp parallel for schedule(static) if(parallel)
    for (int i = 0; i < src.height; i++)
        for (int j = 0; j < src.width; j++) {
            dst.set(i,j,0, clamp(src.get(i,j,0) + 30));
            dst.set(i,j,1, clamp(src.get(i,j,1) + 10));
            dst.set(i,j,2, clamp(src.get(i,j,2) - 20));
        }
}

// 8. Cool Tone
void coolTone(const Image& src, Image& dst, bool parallel) {
    dst = src;
    #pragma omp parallel for schedule(static) if(parallel)
    for (int i = 0; i < src.height; i++)
        for (int j = 0; j < src.width; j++) {
            dst.set(i,j,0, clamp(src.get(i,j,0) - 20));
            dst.set(i,j,1, clamp(src.get(i,j,1) + 10));
            dst.set(i,j,2, clamp(src.get(i,j,2) + 30));
        }
}

// 9. Pixelate
void pixelate(const Image& src, Image& dst, int blockSize, bool parallel) {
    dst = src;
    #pragma omp parallel for schedule(static) if(parallel)
    for (int i = 0; i < src.height; i += blockSize)
        for (int j = 0; j < src.width; j += blockSize) {
            // Average colour of this block
            int sumR=0, sumG=0, sumB=0, count=0;
            for (int bi = i; bi < std::min(i+blockSize, src.height); bi++)
                for (int bj = j; bj < std::min(j+blockSize, src.width); bj++) {
                    sumR += src.get(bi,bj,0);
                    sumG += src.get(bi,bj,1);
                    sumB += src.get(bi,bj,2);
                    count++;
                }
            unsigned char avgR = sumR/count, avgG = sumG/count, avgB = sumB/count;
            // Fill entire block with average colour
            for (int bi = i; bi < std::min(i+blockSize, src.height); bi++)
                for (int bj = j; bj < std::min(j+blockSize, src.width); bj++) {
                    dst.set(bi,bj,0,avgR);
                    dst.set(bi,bj,1,avgG);
                    dst.set(bi,bj,2,avgB);
                }
        }
}

// 10. Vignette
void vignette(const Image& src, Image& dst, float strength, bool parallel) {
    dst = src;
    float cx = src.width  / 2.0f;
    float cy = src.height / 2.0f;
    float maxDist = std::sqrt(cx*cx + cy*cy);

    #pragma omp parallel for schedule(static) if(parallel)
    for (int i = 0; i < src.height; i++)
        for (int j = 0; j < src.width; j++) {
            float dx = j - cx, dy = i - cy;
            float dist = std::sqrt(dx*dx + dy*dy) / maxDist;
            float factor = 1.0f - strength * dist * dist;
            factor = std::max(0.0f, factor);
            dst.set(i,j,0, clamp((int)(src.get(i,j,0) * factor)));
            dst.set(i,j,1, clamp((int)(src.get(i,j,1) * factor)));
            dst.set(i,j,2, clamp((int)(src.get(i,j,2) * factor)));
        }
}

// Generic kernel convolution
void applyKernel(const Image& src, Image& dst,
                 const float kernel[3][3],
                 float divisor, int bias, bool parallel) {
    dst = src;
    #pragma omp parallel for schedule(static) if(parallel)
    for (int i = 1; i < src.height-1; i++)
        for (int j = 1; j < src.width-1; j++)
            for (int ch = 0; ch < 3; ch++) {
                float sum = 0.0f;
                for (int ki=-1; ki<=1; ki++)
                    for (int kj=-1; kj<=1; kj++)
                        sum += kernel[ki+1][kj+1] * src.get(i+ki,j+kj,ch);
                dst.set(i,j,ch, clamp((int)(sum/divisor)+bias));
            }
}

// 11. Sharpen (with passes)
void sharpen(const Image& src, Image& dst, int passes, bool parallel) {
    const float k[3][3] = {{0,-1,0},{-1,5,-1},{0,-1,0}};
    Image temp = src;
    for (int p = 0; p < passes; p++) {
        applyKernel(temp, dst, k, 1.0f, 0, parallel);
        temp = dst;
    }
}

// 12. Box Blur (with passes)
void boxBlur(const Image& src, Image& dst, int passes, bool parallel) {
    const float k[3][3] = {{1,1,1},{1,1,1},{1,1,1}};
    Image temp = src;
    for (int p = 0; p < passes; p++) {
        applyKernel(temp, dst, k, 9.0f, 0, parallel);
        temp = dst;
    }
}

// 13. Gaussian Blur (with passes)
void gaussianBlur(const Image& src, Image& dst, int passes, bool parallel) {
    const float k[3][3] = {{1,2,1},{2,4,2},{1,2,1}};
    Image temp = src;
    for (int p = 0; p < passes; p++) {
        applyKernel(temp, dst, k, 16.0f, 0, parallel);
        temp = dst;
    }
}

// 14. Edge Detection
void edgeDetection(const Image& src, Image& dst, bool parallel) {
    dst = src;
    const float Gx[3][3] = {{-1,0,1},{-2,0,2},{-1,0,1}};
    const float Gy[3][3] = {{-1,-2,-1},{0,0,0},{1,2,1}};
    #pragma omp parallel for schedule(static) if(parallel)
    for (int i=1; i<src.height-1; i++)
        for (int j=1; j<src.width-1; j++)
            for (int ch=0; ch<3; ch++) {
                float gx=0, gy=0;
                for (int ki=-1; ki<=1; ki++)
                    for (int kj=-1; kj<=1; kj++) {
                        float p = src.get(i+ki,j+kj,ch);
                        gx += Gx[ki+1][kj+1]*p;
                        gy += Gy[ki+1][kj+1]*p;
                    }
                dst.set(i,j,ch, clamp((int)std::sqrt(gx*gx+gy*gy)));
            }
}

// 15. Emboss (with passes)
void emboss(const Image& src, Image& dst, int passes, bool parallel) {
    const float k[3][3] = {{-2,-1,0},{-1,1,1},{0,1,2}};
    Image temp = src;
    for (int p = 0; p < passes; p++) {
        applyKernel(temp, dst, k, 1.0f, 128, parallel);
        temp = dst;
    }
}

// ============================================================
//  GUI HELPERS
// ============================================================
void appendResult(const std::wstring& text) {
    int len = GetWindowTextLength(hResults);
    SendMessage(hResults, EM_SETSEL, len, len);
    SendMessage(hResults, EM_REPLACESEL, FALSE, (LPARAM)text.c_str());
}

void clearResults() { SetWindowText(hResults, L""); }

int getComboInt(HWND hCombo) {
    wchar_t buf[16];
    int idx = SendMessage(hCombo, CB_GETCURSEL, 0, 0);
    SendMessage(hCombo, CB_GETLBTEXT, idx, (LPARAM)buf);
    // Extract leading integer from string like "3x" or "10px" or "Medium"
    return _wtoi(buf);
}

// ============================================================
//  RUN PARAMS
// ============================================================
struct RunParams {
    std::wstring imagePath, outputDir;
    int threads;
    bool doGray, doBright, doSepia, doNeg, doThresh, doChan, doWarm, doCool;
    bool doSharpen, doBox, doGauss, doEdge, doEmboss, doPix, doVig;
    int  passesSharpen, passesBox, passesGauss, passesEmboss;
    int  vigStrengthIdx, pixBlockSize;
};

// ============================================================
//  PROCESSING THREAD
// ============================================================
void processingThread(RunParams p) {
    EnableWindow(hBtnRun, FALSE);

    appendResult(L"Loading image...\r\n");
    Image src;
    if (!loadImage(p.imagePath, src)) {
        appendResult(L"ERROR: Could not load image.\r\n");
        EnableWindow(hBtnRun, TRUE);
        return;
    }

    std::wostringstream info;
    info << L"Image: " << src.width << L" x " << src.height << L" pixels\r\n";
    info << L"Threads: " << p.threads << L"\r\n";
    info << L"Output: "  << p.outputDir << L"\r\n\r\n";
    appendResult(info.str());

    double totalSerial = 0, totalParallel = 0;
    int    filterCount = 0;

    // Helper: run one filter, time it, save it, report it
    auto runOne = [&](
        const std::wstring& name,
        std::function<void(const Image&, Image&, bool)> fn)
    {
        appendResult(L"─────────────────────────────\r\n");
        appendResult(L"Filter: " + name + L"\r\n");

        Image serialOut, parallelOut;

        omp_set_num_threads(1);
        double t1 = omp_get_wtime();
        fn(src, serialOut, false);
        double st = omp_get_wtime() - t1;

        omp_set_num_threads(p.threads);
        double t2 = omp_get_wtime();
        fn(src, parallelOut, true);
        double pt = omp_get_wtime() - t2;

        saveImage(p.outputDir + L"\\" + name + L".png", parallelOut);

        double speedup    = st / pt;
        double efficiency = (speedup / p.threads) * 100.0;

        std::wostringstream r;
        r << std::fixed << std::setprecision(4);
        r << L"  Serial    : " << st << L" s\r\n";
        r << L"  Parallel  : " << pt << L" s\r\n";
        r << std::setprecision(2);
        r << L"  Speedup   : " << speedup    << L"x\r\n";
        r << L"  Efficiency: " << efficiency << L"%\r\n";
        appendResult(r.str());

        totalSerial   += st;
        totalParallel += pt;
        filterCount++;
    };

    // Vignette strength map
    float vigStrengths[] = { 0.5f, 0.8f, 1.2f, 1.8f };
    float vigStr = vigStrengths[std::max(0, std::min(3, p.vigStrengthIdx))];

    // Run selected filters
    if (p.doGray)
        runOne(L"grayscale",
            [](const Image& s, Image& d, bool par){ grayscale(s,d,par); });

    if (p.doBright)
        runOne(L"brightness",
            [](const Image& s, Image& d, bool par){ brightness(s,d,par); });

    if (p.doSepia)
        runOne(L"sepia",
            [](const Image& s, Image& d, bool par){ sepia(s,d,par); });

    if (p.doNeg)
        runOne(L"negative",
            [](const Image& s, Image& d, bool par){ negative(s,d,par); });

    if (p.doThresh)
        runOne(L"threshold",
            [](const Image& s, Image& d, bool par){ threshold(s,d,par); });

    if (p.doChan)
        runOne(L"channelswap",
            [](const Image& s, Image& d, bool par){ channelSwap(s,d,par); });

    if (p.doWarm)
        runOne(L"warmtone",
            [](const Image& s, Image& d, bool par){ warmTone(s,d,par); });

    if (p.doCool)
        runOne(L"cooltone",
            [](const Image& s, Image& d, bool par){ coolTone(s,d,par); });

    if (p.doSharpen) {
        int passes = p.passesSharpen;
        runOne(L"sharpen_" + std::to_wstring(passes) + L"x",
            [passes](const Image& s, Image& d, bool par){ sharpen(s,d,passes,par); });
    }

    if (p.doBox) {
        int passes = p.passesBox;
        runOne(L"boxblur_" + std::to_wstring(passes) + L"x",
            [passes](const Image& s, Image& d, bool par){ boxBlur(s,d,passes,par); });
    }

    if (p.doGauss) {
        int passes = p.passesGauss;
        runOne(L"gaussianblur_" + std::to_wstring(passes) + L"x",
            [passes](const Image& s, Image& d, bool par){ gaussianBlur(s,d,passes,par); });
    }

    if (p.doEdge)
        runOne(L"edgedetection",
            [](const Image& s, Image& d, bool par){ edgeDetection(s,d,par); });

    if (p.doEmboss) {
        int passes = p.passesEmboss;
        runOne(L"emboss_" + std::to_wstring(passes) + L"x",
            [passes](const Image& s, Image& d, bool par){ emboss(s,d,passes,par); });
    }

    if (p.doPix) {
        int bs = p.pixBlockSize;
        runOne(L"pixelate_" + std::to_wstring(bs) + L"px",
            [bs](const Image& s, Image& d, bool par){ pixelate(s,d,bs,par); });
    }

    if (p.doVig) {
        float vs = vigStr;
        runOne(L"vignette",
            [vs](const Image& s, Image& d, bool par){ vignette(s,d,vs,par); });
    }

    // ── TOTAL SUMMARY ──────────────────────────────────────
    if (filterCount > 0) {
        double overallSpeedup    = totalSerial / totalParallel;
        double avgEfficiency     = (overallSpeedup / p.threads) * 100.0;

        std::wostringstream sum;
        sum << L"\r\n═════════════════════════════\r\n";
        sum << L"         TOTAL SUMMARY\r\n";
        sum << L"═════════════════════════════\r\n";
        sum << std::fixed << std::setprecision(4);
        sum << L"  Filters run     : " << filterCount      << L"\r\n";
        sum << L"  Threads used    : " << p.threads        << L"\r\n";
        sum << L"  Total serial    : " << totalSerial      << L" s\r\n";
        sum << L"  Total parallel  : " << totalParallel    << L" s\r\n";
        sum << std::setprecision(2);
        sum << L"  Overall speedup : " << overallSpeedup   << L"x\r\n";
        sum << L"  Avg efficiency  : " << avgEfficiency    << L"%\r\n";
        sum << L"═════════════════════════════\r\n";
        appendResult(sum.str());
    }

    appendResult(L"\r\nDone! Click 'Open Output Folder' to see results.\r\n");
    EnableWindow(hBtnRun,        TRUE);
    EnableWindow(hBtnOpenFolder, TRUE);
}

// ============================================================
//  HELPER: Create a labelled combobox
// ============================================================
HWND makeCombo(HWND parent, int x, int y, int w, int id,
               const wchar_t** items, int count, HFONT font) {
    HWND hCombo = CreateWindow(L"COMBOBOX", nullptr,
        WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
        x, y, w, 120, parent, (HMENU)(UINT_PTR)id, nullptr, nullptr);
    SendMessage(hCombo, WM_SETFONT, (WPARAM)font, TRUE);
    for (int i = 0; i < count; i++)
        SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)items[i]);
    SendMessage(hCombo, CB_SETCURSEL, 0, 0);
    return hCombo;
}

// ============================================================
//  WINDOW PROCEDURE
// ============================================================
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {

    case WM_CREATE: {
        HFONT hFont = CreateFont(15, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
        HFONT hBold = CreateFont(15, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
        HFONT hTitle = CreateFont(18, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
        HFONT hMono = CreateFont(13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, FIXED_PITCH, L"Consolas");

        // ── Title ───────────────────────────────────────────
        HWND hT = CreateWindow(L"STATIC",
    L"Parallel Image Processor  |  CSC 426  |  Group 1",
    WS_CHILD|WS_VISIBLE|SS_CENTER,
    0, 8, 620, 24, hwnd, (HMENU)ID_LBL_TITLE, nullptr, nullptr);
        SendMessage(hT, WM_SETFONT, (WPARAM)hTitle, TRUE);

        // ── Image path ──────────────────────────────────────
        HWND hLI = CreateWindow(L"STATIC", L"Image:",
    WS_CHILD|WS_VISIBLE, 12, 42, 50, 22, hwnd, (HMENU)ID_LBL_IMAGE, nullptr, nullptr);
        SendMessage(hLI, WM_SETFONT, (WPARAM)hFont, TRUE);

        hEditPath = CreateWindow(L"EDIT", L"No image selected",
            WS_CHILD|WS_VISIBLE|WS_BORDER|ES_READONLY,
            62, 40, 400, 24, hwnd, (HMENU)ID_EDIT_PATH, nullptr, nullptr);
        SendMessage(hEditPath, WM_SETFONT, (WPARAM)hFont, TRUE);

        HWND hBrowse = CreateWindow(L"BUTTON", L"Browse...",
            WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,
            468, 40, 140, 26, hwnd, (HMENU)ID_BTN_BROWSE, nullptr, nullptr);
        SendMessage(hBrowse, WM_SETFONT, (WPARAM)hFont, TRUE);

        // ── Threads ─────────────────────────────────────────
        HWND hLT = CreateWindow(L"STATIC", L"Threads (1-64):",
    WS_CHILD|WS_VISIBLE, 12, 76, 120, 22, hwnd, (HMENU)ID_LBL_THREADS, nullptr, nullptr);
        SendMessage(hLT, WM_SETFONT, (WPARAM)hFont, TRUE);

        hEditThreads = CreateWindow(L"EDIT", L"4",
            WS_CHILD|WS_VISIBLE|WS_BORDER|ES_NUMBER,
            134, 74, 60, 24, hwnd, (HMENU)ID_EDIT_THREADS, nullptr, nullptr);
        SendMessage(hEditThreads, WM_SETFONT, (WPARAM)hFont, TRUE);


        // ── Filters label ────────────────────────────────────
        HWND hLF = CreateWindow(L"STATIC", L"Filters:",
    WS_CHILD|WS_VISIBLE, 12, 108, 60, 20, hwnd, (HMENU)ID_LBL_FILTERS, nullptr, nullptr);
        SendMessage(hLF, WM_SETFONT, (WPARAM)hBold, TRUE);

        // Intensity dropdown options
        const wchar_t* passes[]    = {L"1x", L"2x", L"3x", L"4x", L"5x"};
        const wchar_t* vigOpts[]   = {L"Light", L"Medium", L"Strong", L"Extreme"};
        const wchar_t* pixOpts[]   = {L"5px", L"10px", L"15px", L"20px", L"30px"};

        // ── Left column checkboxes (y starts at 130) ─────────
        // Row 1
        hChkGray = CreateWindow(L"BUTTON", L"Grayscale",
            WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX,
            12, 130, 120, 22, hwnd, (HMENU)ID_CHK_GRAYSCALE, nullptr, nullptr);
        SendMessage(hChkGray, WM_SETFONT, (WPARAM)hFont, TRUE);

        hChkBright = CreateWindow(L"BUTTON", L"Brightness",
            WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX,
            12, 158, 120, 22, hwnd, (HMENU)ID_CHK_BRIGHTNESS, nullptr, nullptr);
        SendMessage(hChkBright, WM_SETFONT, (WPARAM)hFont, TRUE);

        hChkSepia = CreateWindow(L"BUTTON", L"Sepia",
            WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX,
            12, 186, 120, 22, hwnd, (HMENU)ID_CHK_SEPIA, nullptr, nullptr);
        SendMessage(hChkSepia, WM_SETFONT, (WPARAM)hFont, TRUE);

        hChkSharpen = CreateWindow(L"BUTTON", L"Sharpen",
            WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX,
            12, 214, 100, 22, hwnd, (HMENU)ID_CHK_SHARPEN, nullptr, nullptr);
        SendMessage(hChkSharpen, WM_SETFONT, (WPARAM)hFont, TRUE);
        hCmbSharpen = makeCombo(hwnd, 114, 213, 70, ID_CMB_SHARPEN, passes, 5, hFont);

        hChkBox = CreateWindow(L"BUTTON", L"Box Blur",
            WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX,
            12, 242, 100, 22, hwnd, (HMENU)ID_CHK_BOXBLUR, nullptr, nullptr);
        SendMessage(hChkBox, WM_SETFONT, (WPARAM)hFont, TRUE);
        hCmbBoxBlur = makeCombo(hwnd, 114, 241, 70, ID_CMB_BOXBLUR, passes, 5, hFont);

        hChkGauss = CreateWindow(L"BUTTON", L"Gaussian Blur",
            WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX,
            12, 270, 120, 22, hwnd, (HMENU)ID_CHK_GAUSSIAN, nullptr, nullptr);
        SendMessage(hChkGauss, WM_SETFONT, (WPARAM)hFont, TRUE);
        hCmbGaussian = makeCombo(hwnd, 134, 269, 70, ID_CMB_GAUSSIAN, passes, 5, hFont);

        hChkEdge = CreateWindow(L"BUTTON", L"Edge Detection",
            WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX,
            12, 298, 130, 22, hwnd, (HMENU)ID_CHK_EDGE, nullptr, nullptr);
        SendMessage(hChkEdge, WM_SETFONT, (WPARAM)hFont, TRUE);

        hChkEmboss = CreateWindow(L"BUTTON", L"Emboss",
            WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX,
            12, 326, 100, 22, hwnd, (HMENU)ID_CHK_EMBOSS, nullptr, nullptr);
        SendMessage(hChkEmboss, WM_SETFONT, (WPARAM)hFont, TRUE);
        hCmbEmboss = makeCombo(hwnd, 114, 325, 70, ID_CMB_EMBOSS, passes, 5, hFont);

        // ── Right column checkboxes ──────────────────────────
        hChkNeg = CreateWindow(L"BUTTON", L"Negative",
            WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX,
            320, 130, 120, 22, hwnd, (HMENU)ID_CHK_NEGATIVE, nullptr, nullptr);
        SendMessage(hChkNeg, WM_SETFONT, (WPARAM)hFont, TRUE);

        hChkThresh = CreateWindow(L"BUTTON", L"Threshold",
            WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX,
            320, 158, 120, 22, hwnd, (HMENU)ID_CHK_THRESHOLD, nullptr, nullptr);
        SendMessage(hChkThresh, WM_SETFONT, (WPARAM)hFont, TRUE);

        hChkChan = CreateWindow(L"BUTTON", L"Channel Swap",
            WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX,
            320, 186, 130, 22, hwnd, (HMENU)ID_CHK_CHANSWAP, nullptr, nullptr);
        SendMessage(hChkChan, WM_SETFONT, (WPARAM)hFont, TRUE);

        hChkWarm = CreateWindow(L"BUTTON", L"Warm Tone",
            WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX,
            320, 214, 120, 22, hwnd, (HMENU)ID_CHK_WARM, nullptr, nullptr);
        SendMessage(hChkWarm, WM_SETFONT, (WPARAM)hFont, TRUE);

        hChkCool = CreateWindow(L"BUTTON", L"Cool Tone",
            WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX,
            320, 242, 120, 22, hwnd, (HMENU)ID_CHK_COOL, nullptr, nullptr);
        SendMessage(hChkCool, WM_SETFONT, (WPARAM)hFont, TRUE);

        hChkPix = CreateWindow(L"BUTTON", L"Pixelate",
            WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX,
            320, 270, 90, 22, hwnd, (HMENU)ID_CHK_PIXELATE, nullptr, nullptr);
        SendMessage(hChkPix, WM_SETFONT, (WPARAM)hFont, TRUE);
        hCmbPixelate = makeCombo(hwnd, 412, 269, 80, ID_CMB_PIXELATE, pixOpts, 5, hFont);

        hChkVig = CreateWindow(L"BUTTON", L"Vignette",
            WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX,
            320, 298, 90, 22, hwnd, (HMENU)ID_CHK_VIGNETTE, nullptr, nullptr);
        SendMessage(hChkVig, WM_SETFONT, (WPARAM)hFont, TRUE);
        hCmbVignette = makeCombo(hwnd, 412, 297, 90, ID_CMB_VIGNETTE, vigOpts, 4, hFont);

        // ALL FILTERS
        hChkAll = CreateWindow(L"BUTTON", L"ALL FILTERS",
            WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX,
            320, 326, 120, 22, hwnd, (HMENU)ID_CHK_ALL, nullptr, nullptr);
        SendMessage(hChkAll, WM_SETFONT, (WPARAM)hBold, TRUE);
        SendMessage(hChkAll, BM_SETCHECK, BST_CHECKED, 0);

        // Check all by default
        HWND allChks[] = {hChkGray,hChkBright,hChkSepia,hChkSharpen,
                          hChkBox,hChkGauss,hChkEdge,hChkEmboss,
                          hChkNeg,hChkPix,hChkVig,hChkChan,
                          hChkThresh,hChkWarm,hChkCool};
        for (auto h : allChks)
            SendMessage(h, BM_SETCHECK, BST_CHECKED, 0);

        // ── Run button ───────────────────────────────────────
        hBtnRun = CreateWindow(L"BUTTON", L"▶   Run Filters",
            WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,
            12, 364, 596, 36, hwnd, (HMENU)ID_BTN_RUN, nullptr, nullptr);
        HFONT hBtnFont = CreateFont(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
        SendMessage(hBtnRun, WM_SETFONT, (WPARAM)hBtnFont, TRUE);

        // ── Results ──────────────────────────────────────────
        HWND hLR = CreateWindow(L"STATIC", L"Results:",
    WS_CHILD|WS_VISIBLE, 12, 410, 80, 20, hwnd, (HMENU)ID_LBL_RESULTS, nullptr, nullptr);
        SendMessage(hLR, WM_SETFONT, (WPARAM)hBold, TRUE);

        hResults = CreateWindow(L"EDIT", L"",
            WS_CHILD|WS_VISIBLE|WS_BORDER|WS_VSCROLL|
            ES_MULTILINE|ES_READONLY|ES_AUTOVSCROLL,
            12, 432, 596, 280, hwnd, (HMENU)ID_RESULTS, nullptr, nullptr);
        SendMessage(hResults, WM_SETFONT, (WPARAM)hMono, TRUE);

        // ── Open folder button ───────────────────────────────
        hBtnOpenFolder = CreateWindow(L"BUTTON", L"Open Output Folder",
            WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON|WS_DISABLED,
            12, 724, 596, 28, hwnd, (HMENU)ID_BTN_OPEN_FOLDER, nullptr, nullptr);
        SendMessage(hBtnOpenFolder, WM_SETFONT, (WPARAM)hFont, TRUE);

        break;
    }

    case WM_COMMAND: {
        int id = LOWORD(wParam);

        // Browse
        if (id == ID_BTN_BROWSE) {
            OPENFILENAME ofn = {};
            wchar_t szFile[MAX_PATH] = {};
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner   = hwnd;
            ofn.lpstrFile   = szFile;
            ofn.nMaxFile    = MAX_PATH;
            ofn.lpstrFilter = L"Image Files\0*.jpg;*.jpeg;*.png;*.bmp\0All Files\0*.*\0";
            ofn.Flags       = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
            if (GetOpenFileName(&ofn)) {
                g_imagePath = szFile;
                SetWindowText(hEditPath, szFile);
                std::wstring dir = g_imagePath.substr(0, g_imagePath.find_last_of(L"\\/"));
                g_outputDir = dir + L"\\output";
                CreateDirectory(g_outputDir.c_str(), nullptr);
            }
        }

        // ALL FILTERS toggle
        if (id == ID_CHK_ALL) {
            bool chk = SendMessage(hChkAll, BM_GETCHECK, 0, 0) == BST_CHECKED;
            WPARAM st = chk ? BST_CHECKED : BST_UNCHECKED;
            HWND allChks[] = {hChkGray,hChkBright,hChkSepia,hChkSharpen,
                              hChkBox,hChkGauss,hChkEdge,hChkEmboss,
                              hChkNeg,hChkPix,hChkVig,hChkChan,
                              hChkThresh,hChkWarm,hChkCool};
            for (auto h : allChks)
                SendMessage(h, BM_SETCHECK, st, 0);
        }

        // Run
        if (id == ID_BTN_RUN) {
            if (g_imagePath.empty()) {
                MessageBox(hwnd, L"Please select an image first.",
                    L"No Image", MB_OK|MB_ICONWARNING);
                break;
            }

            // Get thread count from text box
            wchar_t tbuf[16];
            GetWindowText(hEditThreads, tbuf, 16);
            int threads = _wtoi(tbuf);
            if (threads < 1 || threads > 64) {
                MessageBox(hwnd,
                    L"Please enter a thread count between 1 and 64.",
                    L"Invalid Threads", MB_OK|MB_ICONWARNING);
                break;
            }

            // Check at least one filter selected
            HWND allChks[] = {hChkGray,hChkBright,hChkSepia,hChkSharpen,
                              hChkBox,hChkGauss,hChkEdge,hChkEmboss,
                              hChkNeg,hChkPix,hChkVig,hChkChan,
                              hChkThresh,hChkWarm,hChkCool};
            bool any = false;
            for (auto h : allChks)
                if (SendMessage(h, BM_GETCHECK, 0, 0) == BST_CHECKED) { any=true; break; }
            if (!any) {
                MessageBox(hwnd, L"Please select at least one filter.",
                    L"No Filter", MB_OK|MB_ICONWARNING);
                break;
            }

            // Get pixelate block size from combo
            wchar_t pixBuf[16];
            int pixIdx = SendMessage(hCmbPixelate, CB_GETCURSEL, 0, 0);
            SendMessage(hCmbPixelate, CB_GETLBTEXT, pixIdx, (LPARAM)pixBuf);
            int pixBlock = _wtoi(pixBuf);

            // Build params
            RunParams p;
            p.imagePath      = g_imagePath;
            p.outputDir      = g_outputDir;
            p.threads        = threads;
            p.doGray         = SendMessage(hChkGray,   BM_GETCHECK,0,0)==BST_CHECKED;
            p.doBright       = SendMessage(hChkBright, BM_GETCHECK,0,0)==BST_CHECKED;
            p.doSepia        = SendMessage(hChkSepia,  BM_GETCHECK,0,0)==BST_CHECKED;
            p.doNeg          = SendMessage(hChkNeg,    BM_GETCHECK,0,0)==BST_CHECKED;
            p.doThresh       = SendMessage(hChkThresh, BM_GETCHECK,0,0)==BST_CHECKED;
            p.doChan         = SendMessage(hChkChan,   BM_GETCHECK,0,0)==BST_CHECKED;
            p.doWarm         = SendMessage(hChkWarm,   BM_GETCHECK,0,0)==BST_CHECKED;
            p.doCool         = SendMessage(hChkCool,   BM_GETCHECK,0,0)==BST_CHECKED;
            p.doSharpen      = SendMessage(hChkSharpen,BM_GETCHECK,0,0)==BST_CHECKED;
            p.doBox          = SendMessage(hChkBox,    BM_GETCHECK,0,0)==BST_CHECKED;
            p.doGauss        = SendMessage(hChkGauss,  BM_GETCHECK,0,0)==BST_CHECKED;
            p.doEdge         = SendMessage(hChkEdge,   BM_GETCHECK,0,0)==BST_CHECKED;
            p.doEmboss       = SendMessage(hChkEmboss, BM_GETCHECK,0,0)==BST_CHECKED;
            p.doPix          = SendMessage(hChkPix,    BM_GETCHECK,0,0)==BST_CHECKED;
            p.doVig          = SendMessage(hChkVig,    BM_GETCHECK,0,0)==BST_CHECKED;
            p.passesSharpen  = getComboInt(hCmbSharpen);
            p.passesBox      = getComboInt(hCmbBoxBlur);
            p.passesGauss    = getComboInt(hCmbGaussian);
            p.passesEmboss   = getComboInt(hCmbEmboss);
            p.vigStrengthIdx = SendMessage(hCmbVignette, CB_GETCURSEL, 0, 0);
            p.pixBlockSize   = pixBlock;

            clearResults();
            EnableWindow(hBtnOpenFolder, FALSE);
            std::thread worker(processingThread, p);
            worker.detach();
        }

        // Open output folder
        if (id == ID_BTN_OPEN_FOLDER && !g_outputDir.empty())
            ShellExecute(nullptr, L"open", g_outputDir.c_str(),
                nullptr, nullptr, SW_SHOWNORMAL);

        break;
    }
    
    
    
    
    case WM_SIZE: {
    int W = LOWORD(lParam);
    int H = HIWORD(lParam);

    int margin   = 12;
    int fullW    = W - margin * 2;
    int col2X    = W / 2 + 4;
    int resultsH = H - 480;
    if (resultsH < 100) resultsH = 100;

    // Title — full width centered
    SetWindowPos(GetDlgItem(hwnd, ID_LBL_TITLE), nullptr,
    0, 8, W, 24, SWP_NOZORDER | SWP_NOREDRAW);
    InvalidateRect(GetDlgItem(hwnd, ID_LBL_TITLE), nullptr, TRUE);

    // Image row
    SetWindowPos(GetDlgItem(hwnd, ID_LBL_IMAGE), nullptr,
        margin, 42, 50, 22, SWP_NOZORDER);
    SetWindowPos(hEditPath, nullptr,
        62, 40, W - 230, 24, SWP_NOZORDER);
    SetWindowPos(GetDlgItem(hwnd, ID_BTN_BROWSE), nullptr,
        W - 160, 40, 148, 26, SWP_NOZORDER);

    // Threads row
    SetWindowPos(GetDlgItem(hwnd, ID_LBL_THREADS), nullptr,
        margin, 76, 120, 22, SWP_NOZORDER);
    SetWindowPos(hEditThreads, nullptr,
        134, 74, 60, 24, SWP_NOZORDER);

    // Filters label
    SetWindowPos(GetDlgItem(hwnd, ID_LBL_FILTERS), nullptr,
        margin, 108, 60, 20, SWP_NOZORDER);

    // Left column filters
    int ly[] = {130,158,186,214,242,270,298,326};
    HWND leftChks[] = {hChkGray,hChkBright,hChkSepia,hChkSharpen,
                       hChkBox,hChkGauss,hChkEdge,hChkEmboss};
    for (int i = 0; i < 8; i++)
        SetWindowPos(leftChks[i], nullptr, margin, ly[i], 110, 22, SWP_NOZORDER);

    // Left combos
    SetWindowPos(hCmbSharpen,  nullptr, margin+114, ly[3], 70, 120, SWP_NOZORDER);
    SetWindowPos(hCmbBoxBlur,  nullptr, margin+114, ly[4], 70, 120, SWP_NOZORDER);
    SetWindowPos(hCmbGaussian, nullptr, margin+134, ly[5], 70, 120, SWP_NOZORDER);
    SetWindowPos(hCmbEmboss,   nullptr, margin+114, ly[7], 70, 120, SWP_NOZORDER);

    // Right column filters
    int ry[] = {130,158,186,214,242,270,298,326};
    HWND rightChks[] = {hChkNeg,hChkThresh,hChkChan,hChkWarm,
                        hChkCool,hChkPix,hChkVig,hChkAll};
    for (int i = 0; i < 8; i++)
        SetWindowPos(rightChks[i], nullptr, col2X, ry[i], 120, 22, SWP_NOZORDER);

    // Right combos
    SetWindowPos(hCmbPixelate, nullptr, col2X+122, ry[5], 90, 120, SWP_NOZORDER);
    SetWindowPos(hCmbVignette, nullptr, col2X+122, ry[6], 90, 120, SWP_NOZORDER);

    // Run button
    SetWindowPos(hBtnRun, nullptr, margin, 364, fullW, 36, SWP_NOZORDER);

    // Results label
    SetWindowPos(GetDlgItem(hwnd, ID_LBL_RESULTS), nullptr,
        margin, 410, 80, 20, SWP_NOZORDER);

    // Results box
    SetWindowPos(hResults, nullptr, margin, 432, fullW, resultsH, SWP_NOZORDER);

    // Open folder button
    SetWindowPos(hBtnOpenFolder, nullptr,
        margin, 432 + resultsH + 8, fullW, 28, SWP_NOZORDER);
    
    InvalidateRect(hwnd, nullptr, TRUE);
    break;
}

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

// ============================================================
//  MAIN
// ============================================================
int main() {
    HINSTANCE hInstance = GetModuleHandle(nullptr);
    int nCmdShow = SW_SHOW;

    WNDCLASS wc      = {};
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = L"ImageProcessorApp";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
    wc.hIcon         = LoadIcon(nullptr, IDI_APPLICATION);
    RegisterClass(&wc);

    hWnd = CreateWindowEx(0,
        L"ImageProcessorApp",
        L"Parallel Image Processor  |  CSC 426 Parallel Programming  |  Group 1",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 628, 800,
        nullptr, nullptr, hInstance, nullptr);

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}