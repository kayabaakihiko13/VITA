#ifndef SSAA_H
#define SSAA_H

#include "control.h"
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

// Function to perform Grid Uniform Distribution for supersampling
static Color GridUniformDistribution(Image *image, int gridSize, int x, int y) {
  int sampleCount = gridSize * gridSize;
  float sampleStep = 1.0f / gridSize;
  Color finalColor = {0, 0, 0, 255};
  float red = 0.0f, green = 0.0f, blue = 0.0f;

  // Loop over the grid
  for (int i = 0; i < gridSize; i++) {
    for (int j = 0; j < gridSize; j++) {
      float subPixelX = x + (i + 0.5f) * sampleStep;
      float subPixelY = y + (j + 0.5f) * sampleStep;

      int imgX = (int)Clamp(subPixelX, 0, image->width - 1);
      int imgY = (int)Clamp(subPixelY, 0, image->height - 1);
      Color sampleColor = GetImageColor(*image, imgX, imgY);

      red += sampleColor.r;
      green += sampleColor.g;
      blue += sampleColor.b;
    }
  }

  finalColor.r = (unsigned char)(red / sampleCount);
  finalColor.g = (unsigned char)(green / sampleCount);
  finalColor.b = (unsigned char)(blue / sampleCount);
  return finalColor;
}

// Function to apply supersampling to an image
void ApplyGridUniformDistribution(Image *image, int gridSize) {
  int width = image->width;
  int height = image->height;
  Image supersampledImage = GenImageColor(width, height, BLANK);

  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      Color color = GridUniformDistribution(image, gridSize, x, y);

      ImageDrawPixel(&supersampledImage, x, y, color);
    }
  }

  // Replace the original image with the supersampled image
  Image tempImage = ImageCopy(supersampledImage);
  UnloadImage(supersampledImage);
  UnloadImage(*image);
  *image = tempImage;
}

#endif // SSAA_H
