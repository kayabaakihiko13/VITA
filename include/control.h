#ifndef CONTROL_H
#define CONTROL_H

#include <raylib.h>
#include <string.h>

#define WIDTH_WINDOW 900
#define HEIGHT_WINDOW 600

// Define the Button structure
typedef struct {
  Rectangle bounds;
  const char *label;
  Color color;
} Button;

// Define the ImageData structure
typedef struct {
  Image original;
  Image processed;
  Texture2D texture;
  float scale;
  const char *currentMethod;
} ImageData;

// Convert an image to grayscale
void ConvertToGrayscale(Image *image) {
  if (image->data == NULL)
    return;

  int width = image->width;
  int height = image->height;
  Color *pixels = (Color *)image->data;

  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      int index = y * width + x;
      Color color = pixels[index];
      unsigned char gray = (unsigned char)(0.299f * color.r + 0.587f * color.g +
                                           0.114f * color.b);
      pixels[index] = (Color){gray, gray, gray, color.a};
    }
  }
}

// Compress the image to a texture
void CompressImage(Image *image) {
  ImageFormat(image, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
}

// Update the image scale and reapply the processing method
void UpdateImageScale(ImageData *imgData, float newScale) {
  imgData->scale = newScale;

  ImageResize(&imgData->processed,
              (int)(imgData->original.width * imgData->scale),
              (int)(imgData->original.height * imgData->scale));

  if (imgData->currentMethod &&
      strcmp(imgData->currentMethod, "Grayscale") == 0) {
    ConvertToGrayscale(&imgData->processed);
  }

  CompressImage(&imgData->processed);

  UnloadTexture(imgData->texture);
  imgData->texture = LoadTextureFromImage(imgData->processed);
}

// Scale an image to fit within specified dimensions
void ScaleImage(Image *image, int maxWidth, int maxHeight) {
  float scaleWidth = (float)maxWidth / image->width;
  float scaleHeight = (float)maxHeight / image->height;
  float scale = (scaleWidth < scaleHeight) ? scaleWidth : scaleHeight;

  int newWidth = (int)(image->width * scale);
  int newHeight = (int)(image->height * scale);

  ImageResize(image, newWidth, newHeight);
}

// Draw a button on the screen
void DrawButton(Button button) {
  DrawRectangleRec(button.bounds, button.color);
  DrawText(button.label, button.bounds.x + 10, button.bounds.y + 10, 20, WHITE);
}

// Check if a button is clicked
bool isButtonClicked(Button button) {
  return CheckCollisionPointRec(GetMousePosition(), button.bounds) &&
         IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
}

// Define Clamp function to restrict values within a range
float Clamp(float value, float min, float max) {
  if (value < min)
    return min;
  if (value > max)
    return max;
  return value;
}

#endif // CONTROL_H
