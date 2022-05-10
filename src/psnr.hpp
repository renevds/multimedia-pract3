#pragma once
// vim: sw=2 ts=2 expandtab

#include <cmath>
#include <cstdint>
#include <limits>

namespace PSNR {

/**
 * Computes the PSNR between two given uint8_t buffers of given length.
 * @param pixels1 - The first buffer of data of bytes (uint8_t).
 * @param pixels2 - The second buffer of data of bytes (uint8_t).
 * @param length - The number of bytes to compare and use for calculating the
 * PSNR.
 */
    double PSNR(const uint8_t* pixels1, const uint8_t* pixels2, uint32_t length) {
        double MSE = 0;
        for (int i = 0; i < length; ++i) {
            MSE += pow(pixels1[i] - pixels2[i], 2);
        }
        MSE = MSE/(double)length;
        return log(10*(255/MSE));
    }

}  // namespace PSNR
