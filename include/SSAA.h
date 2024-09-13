#ifndef SSAA_H
#define SSAA_H

#include <cmath>
#include <cstdlib>
#include <immintrin.h> // Required for SIMD intrinsics
#include <raylib.h>
#include <raymath.h>

#define MIN_SCALE_FACTOR 1.0f
#define MAX_SCALE_FACTOR 2.5f

// Function to get the nearest neighbor pixel color using the Nearest Neighbor
// method
Color getNearestNeighbor(const Color *originalImage, int x, int y,
                         int originalWidth, float scaleFactor) {
  int nearestX = static_cast<int>(x / scaleFactor);
  int nearestY = static_cast<int>(y / scaleFactor);
  return originalImage[nearestY * originalWidth + nearestX];
}

// Function to apply the Nearest Neighbor algorithm for image scaling
void applyNearestNeighbor(Image *inputImage, float scaleFactor) {
  if (scaleFactor <= 0) {
    TraceLog(LOG_ERROR, "Invalid scale factor: %f. It must be greater than 0.",
             scaleFactor);
    return; // Do nothing if the scale factor is invalid
  }

  int newWidth = static_cast<int>(inputImage->width * scaleFactor);
  int newHeight = static_cast<int>(inputImage->height * scaleFactor);
  Image outputImage = GenImageColor(newWidth, newHeight, BLANK);
  Color *inputColors = (Color *)inputImage->data;

  for (int y = 0; y < newHeight; y++) {
    for (int x = 0; x < newWidth; x++) {
      int nearestX = static_cast<int>(x / scaleFactor);
      int nearestY = static_cast<int>(y / scaleFactor);

      // Ensure x and y are within bounds
      nearestX = nearestX < 0
                     ? 0
                     : (nearestX >= inputImage->width ? inputImage->width - 1
                                                      : nearestX);
      nearestY = nearestY < 0
                     ? 0
                     : (nearestY >= inputImage->height ? inputImage->height - 1
                                                       : nearestY);

      Color nearestColor = getNearestNeighbor(inputColors, nearestX, nearestY,
                                              inputImage->width, scaleFactor);
      ImageDrawPixel(&outputImage, x, y, nearestColor);
    }
  }

  UnloadImage(*inputImage); // Free the original image data
  *inputImage =
      outputImage; // Replace the original image with the processed one
}

// Function to get the pixel color from an image
Color GetImageColor(const Image *image, int x, int y) {
  Color *pixels = (Color *)image->data;
  return pixels[y * image->width + x];
}

// Function to set the pixel color on an image
void SetImageColor(Image *image, int x, int y, Color color) {
  Color *pixels = (Color *)image->data;
  pixels[y * image->width + x] = color;
}

// Function to perform Grid Uniform Distribution for supersampling using SIMD
static __m128i GridUniformDistributionSIMD(const Image *image, int gridSize,
                                           int x, int y) {
  int sampleCount = gridSize * gridSize;
  float sampleStep = 1.0f / gridSize;
  __m128i sumColor = _mm_setzero_si128();
  Color *pixels = (Color *)image->data;

  for (int i = 0; i < gridSize; i++) {
    for (int j = 0; j < gridSize; j++) {
      float subPixelX = x + (i + 0.5f) * sampleStep;
      float subPixelY = y + (j + 0.5f) * sampleStep;

      int imgX = (int)Clamp(subPixelX, 0, image->width - 1);
      int imgY = (int)Clamp(subPixelY, 0, image->height - 1);

      Color pixelColor = pixels[imgY * image->width + imgX];

      // Combine color components into a single 128-bit register
      __m128i sampleColor = _mm_setr_epi32(pixelColor.r, pixelColor.g,
                                           pixelColor.b, pixelColor.a);
      sumColor = _mm_add_epi32(
          sumColor, _mm_and_si128(sampleColor, _mm_set1_epi32(0x000000FF)));
    }
  }

  __m128 avgColor =
      _mm_div_ps(_mm_cvtepi32_ps(sumColor), _mm_set1_ps((float)sampleCount));
  return _mm_cvtps_epi32(avgColor);
}

// Function to apply supersampling to an image
void ApplyGridUniformDistribution(Image *image, int gridSize) {
  int width = image->width;
  int height = image->height;
  Image supersampledImage = GenImageColor(width, height, BLANK);
  Color *destPixels = (Color *)supersampledImage.data;

  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width - 3; x += 4) {
      __m128i color0 = GridUniformDistributionSIMD(image, gridSize, x, y);
      __m128i color1 = GridUniformDistributionSIMD(image, gridSize, x + 1, y);
      __m128i color2 = GridUniformDistributionSIMD(image, gridSize, x + 2, y);
      __m128i color3 = GridUniformDistributionSIMD(image, gridSize, x + 3, y);

      __m128i packed01 = _mm_packs_epi32(color0, color1);
      __m128i packed23 = _mm_packs_epi32(color2, color3);
      __m128i final = _mm_packus_epi16(packed01, packed23);

      _mm_storeu_si128((__m128i *)&destPixels[y * width + x], final);
    }

    // Handle remaining pixels
    for (int x = (width / 4) * 4; x < width; x++) {
      __m128i color = GridUniformDistributionSIMD(image, gridSize, x, y);
      destPixels[y * width + x].r = _mm_extract_epi8(color, 0);
      destPixels[y * width + x].g = _mm_extract_epi8(color, 4);
      destPixels[y * width + x].b = _mm_extract_epi8(color, 8);
      destPixels[y * width + x].a = 255; // Ensure alpha is set to opaque
    }
  }

  UnloadImage(*image);
  *image = supersampledImage;
}

// Function to perform Poisson Disc Sampling for supersampling
void ApplyPoissonDiscSampling(Image *image, int sampleCount, float radius) {
  Image outputImage = GenImageColor(image->width, image->height, BLANK);
  Vector2 *samples = (Vector2 *)malloc(sampleCount * sizeof(Vector2));
  int validSamples = 0;

  for (int i = 0; i < sampleCount; i++) {
    float x = GetRandomValue(0, image->width - 1);
    float y = GetRandomValue(0, image->height - 1);

    // Check if the sample is within a valid radius
    bool isValid = true;
    for (int j = 0; j < validSamples; j++) {
      if (Vector2Distance(samples[j], (Vector2){x, y}) < radius) {
        isValid = false;
        break;
      }
    }

    if (isValid) {
      samples[validSamples++] = (Vector2){x, y};
      Color color = GetImageColor(image, (int)x, (int)y);
      SetImageColor(&outputImage, (int)x, (int)y, color);
    }
  }

  UnloadImage(*image);  // Free the original image data
  *image = outputImage; // Replace the original image with the processed one
  free(samples);
}

// Function to perform Hierarchical Rasterization Anti-Aliasing (HRAA)
void ApplyHRAA(Image *image, int scale) {
  if (scale <= 1)
    return;

  int newWidth = image->width * scale;
  int newHeight = image->height * scale;
  Image scaledImage = GenImageColor(newWidth, newHeight, BLANK);

  for (int y = 0; y < newHeight; y++) {
    for (int x = 0; x < newWidth; x++) {
      int originalX = x / scale;
      int originalY = y / scale;
      Color originalColor = GetImageColor(image, originalX, originalY);
      SetImageColor(&scaledImage, x, y, originalColor);
    }
  }

  ApplyGridUniformDistribution(&scaledImage, scale); // Apply anti-aliasing
  UnloadImage(*image);  // Free the original image data
  *image = scaledImage; // Replace the original image with the processed one
}

#endif // SSAA_H
