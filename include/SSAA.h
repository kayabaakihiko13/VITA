#ifndef SSAA_H
#define SSAA_H

#include <math.h>
#include <raylib.h>

#define MIN_SCALE_FACTOR 1.0f
#define MAX_SCALE_FACTOR 2.5f

// Fungsi untuk mendapatkan warna piksel terdekat menggunakan metode Nearest
// Neighbor
Color getNearestNeighbor(const Color *originalImage, int x, int y,
                         int originalWidth, float scaleFactor) {
  int nearestX = (int)(x / scaleFactor);
  int nearestY = (int)(y / scaleFactor);
  return originalImage[nearestY * originalWidth + nearestX];
}

// Fungsi untuk menerapkan algoritma Nearest Neighbor untuk penskalaan gambar
void applyNearestNeighbor(Image *inputImage, float scaleFactor) {
  if (scaleFactor <= 0) {
    TraceLog(LOG_ERROR, "Invalid scale factor: %f. It must be greater than 0.",
             scaleFactor);
    return; // Do nothing if the scale factor is invalid
  }

  int newWidth = inputImage->width * scaleFactor;
  int newHeight = inputImage->height * scaleFactor;

  Image outputImage = GenImageColor(newWidth, newHeight, BLANK);
  Color *inputColors = (Color *)inputImage->data;

  for (int y = 0; y < newHeight; y++) {
    for (int x = 0; x < newWidth; x++) {
      int nearestX = x / scaleFactor;
      int nearestY = y / scaleFactor;

      // Ensure x and y are within bounds
      nearestX = nearestX < 0
                     ? 0
                     : (nearestX >= inputImage->width ? inputImage->width - 1
                                                      : nearestX);
      nearestY = nearestY < 0
                     ? 0
                     : (nearestY >= inputImage->height ? inputImage->height - 1
                                                       : nearestY);

      Color nearestColor =
          getNearestNeighbor(inputColors, x, y, inputImage->width, scaleFactor);
      ImageDrawPixel(&outputImage, x, y, nearestColor);
    }
  }

  UnloadImage(*inputImage); // Free the original image data
  *inputImage =
      outputImage; // Replace the original image with the processed one
}

// Fungsi untuk mendapatkan warna piksel dari sebuah gambar
Color GetImageColor(Image *image, int x, int y) {
  Color *pixels = (Color *)image->data;
  return pixels[y * image->width + x];
}

// Fungsi untuk mengatur warna piksel pada sebuah gambar
void SetImageColor(Image *image, int x, int y, Color color) {
  Color *pixels = (Color *)image->data;
  pixels[y * image->width + x] = color;
}

// Fungsi untuk menerapkan distribusi warna seragam pada grid
// void applyGridUniformDistribution(Image *image) {
//     int gridSize = 4;

//     for (int y = 0; y < image->height; y += gridSize) {
//         for (int x = 0; x < image->width; x += gridSize) {
//             Color avgColor = {0};
//             int count = 0;

//             for (int dy = 0; dy < gridSize && y + dy < image->height; dy++) {
//                 for (int dx = 0; dx < gridSize && x + dx < image->width;
//                 dx++) {
//                     Color pixelColor = GetImageColor(image, x + dx, y + dy);
//                     avgColor.r += pixelColor.r;
//                     avgColor.g += pixelColor.g;
//                     avgColor.b += pixelColor.b;
//                     count++;
//                 }
//             }

//             avgColor.r /= count;
//             avgColor.g /= count;
//             avgColor.b /= count;

//             for (int dy = 0; dy < gridSize && y + dy < image->height; dy++) {
//                 for (int dx = 0; dx < gridSize && x + dx < image->width;
//                 dx++) {
//                     SetImageColor(image, x + dx, y + dy, avgColor);
//                 }
//             }
//         }
//     }
// }

// Fungsi penerapan Poisson Disc Sampling

// void applyPoissonDisc(Image *image) {
//     int radius = 10;

//     for (int y = 0; y < image->height; y += radius * 2) {
//         for (int x = 0; x < image->width; x += radius * 2) {
//             int sampleX = x + rand() % radius;
//             int sampleY = y + rand() % radius;

//             if (sampleX < image->width && sampleY < image->height) {
//                 Color sampleColor = GetImageColor(image, sampleX, sampleY);

//                 for (int dy = -radius; dy <= radius; dy++) {
//                     for (int dx = -radius; dx <= radius; dx++) {
//                         int px = sampleX + dx;
//                         int py = sampleY + dy;

//                         if (px >= 0 && px < image->width && py >= 0 && py <
//                         image->height) {
//                             float distance = sqrt(dx * dx + dy * dy);
//                             if (distance <= radius) {
//                                 SetImageColor(image, px, py, sampleColor);
//                             }
//                         }
//                     }
//                 }
//             }
//         }
//     }
// }

// Fungsi untuk penerapan Hybrid Resolution Anti-Aliasing (HRAA)

// void applyHRAA(Image *image) {
//     int factor = 2;
//     Image tempImage = ImageCopy(*image);

//     for (int y = 0; y < image->height; y++) {
//         for (int x = 0; x < image->width; x++) {
//             Color sumColor = {0};
//             int count = 0;

//             for (int dy = -factor / 2; dy <= factor / 2; dy++) {
//                 for (int dx = -factor / 2; dx <= factor / 2; dx++) {
//                     int sx = x + dx;
//                     int sy = y + dy;

//                     if (sx >= 0 && sx < image->width && sy >= 0 && sy <
//                     image->height) {
//                         Color sampleColor = GetImageColor(&tempImage, sx,
//                         sy); sumColor.r += sampleColor.r; sumColor.g +=
//                         sampleColor.g; sumColor.b += sampleColor.b; count++;
//                     }
//                 }
//             }

//             Color avgColor = {
//                 sumColor.r / count,
//                 sumColor.g / count,
//                 sumColor.b / count,
//                 255
//             };

//             SetImageColor(image, x, y, avgColor);
//         }
//     }

//     UnloadImage(tempImage);
// }

#endif // SSAA_H
