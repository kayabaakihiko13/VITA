#include "include/control.h"
#include <raylib.h>
#include <stdlib.h>
#include <string.h>

#define MAX_BUTTONS 4
#define ZOOM_FACTOR 1.2f
#define MESSAGE_DISPLAY_TIME                                                   \
  3.0f // Time in seconds for the message to be displayed

// Define Clamp function to restrict values within a range
float Clamp(float value, float min, float max) {
  if (value < min)
    return min;
  if (value > max)
    return max;
  return value;
}

// Function to perform Grid Uniform Distribution for supersampling
Color GridUniformDistribution(Image *image, int gridSize, int x, int y) {
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
void ApplySupersampling(Image *image, int gridSize) {
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

int main(void) {
  // Initialize window
  InitWindow(WIDTH_WINDOW, HEIGHT_WINDOW,
             "Image Upload GUI with Supersampling");

  // Load and set window icon
  Image icon = LoadImage("assets/logo.png");
  if (icon.data) {
    SetWindowIcon(icon);
    UnloadImage(icon);
  }

  // Define buttons
  Button buttons[MAX_BUTTONS] = {
      {{0, 10, 200, 50}, "Nearest Neighbor", {109, 104, 117, 255}},
      {{0, 60, 200, 50}, "Grid Uniform Distribution", {134, 129, 140, 255}},
      {{0, 110, 200, 50}, "Poisson disc", {159, 154, 163, 255}},
      {{0, 160, 200, 50}, "HRAA", {184, 179, 186, 255}}};

  ImageData imageData = {0};
  bool imageLoaded = false;
  bool showMessage = false; // Whether to show the message
  float messageTime = 0.0f; // Time when message should disappear
  SetTargetFPS(60);

  while (!WindowShouldClose()) {
    // Handle file drop
    if (IsFileDropped()) {
      FilePathList droppedFiles = LoadDroppedFiles();
      if (droppedFiles.count > 0) {
        if (imageLoaded) {
          UnloadTexture(imageData.texture);
          UnloadImage(imageData.original);
          UnloadImage(imageData.processed);
        }

        imageData.original = LoadImage(droppedFiles.paths[0]);
        if (imageData.original.data != NULL) {
          imageData.processed = ImageCopy(imageData.original);
          imageData.scale = 1.0f;

          // Resize image to fit the drag-and-drop area
          int targetWidth = WIDTH_WINDOW - 200;
          int targetHeight = HEIGHT_WINDOW;
          float scaleX = (float)targetWidth / imageData.original.width;
          float scaleY = (float)targetHeight / imageData.original.height;
          imageData.scale = (scaleX < scaleY) ? scaleX : scaleY;

          ImageResize(&imageData.processed,
                      (int)(imageData.original.width * imageData.scale),
                      (int)(imageData.original.height * imageData.scale));

          // Compress the image and apply supersampling
          CompressImage(&imageData.processed);
          ApplySupersampling(&imageData.processed,
                             3); // Apply 3x3 supersampling
          imageData.texture = LoadTextureFromImage(imageData.processed);
          imageData.currentMethod = NULL;
          imageLoaded = true;
        }
      }
      UnloadDroppedFiles(droppedFiles);
    }

    // Handle button clicks
    Vector2 mousePosition = GetMousePosition();
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
      for (int i = 0; i < MAX_BUTTONS; i++) {
        if (CheckCollisionPointRec(mousePosition, buttons[i].bounds)) {
          if (i == 0) { // If "Nearest Neighbor" button is clicked
            showMessage = true;
            messageTime = GetTime(); // Record the time when the message should
                                     // start displaying
          }
          buttons[i].color = (Color){255, 0, 0, 255}; // Red color when clicked
        } else {
          buttons[i].color =
              (Color){150, 150, 150, 255}; // Light gray when hovered
        }
      }
    } else {
      for (int i = 0; i < MAX_BUTTONS; i++) {
        if (CheckCollisionPointRec(mousePosition, buttons[i].bounds)) {
          buttons[i].color =
              (Color){150, 150, 150, 255}; // Light gray when hovered
        } else {
          buttons[i].color =
              (Color){109, 104, 117, 255}; // Reset color when not hovered
        }
      }
    }

    // Check if the message should still be displayed
    if (showMessage && GetTime() - messageTime > MESSAGE_DISPLAY_TIME) {
      showMessage = false; // Hide message after the specified time
    }

    BeginDrawing();
    ClearBackground((Color){222, 222, 222, 255}); // Light gray background

    // Draw buttons and handle button clicks
    for (int i = 0; i < MAX_BUTTONS; i++) {
      DrawRectangleRec(buttons[i].bounds, buttons[i].color);
      DrawText(buttons[i].label, buttons[i].bounds.x + 10,
               buttons[i].bounds.y + 15, 20, BLACK);
    }

    // Draw image area
    DrawRectangle(200, 0, WIDTH_WINDOW - 200, HEIGHT_WINDOW, BLACK);

    if (imageLoaded) {
      // Scale image to fit within the drag-and-drop area
      int imageWidth = imageData.processed.width;
      int imageHeight = imageData.processed.height;
      int targetWidth = WIDTH_WINDOW - 200;
      int targetHeight = HEIGHT_WINDOW;
      float scaleX = (float)targetWidth / imageWidth;
      float scaleY = (float)targetHeight / imageHeight;
      float scale = (scaleX < scaleY) ? scaleX : scaleY;

      int scaledWidth = (int)(imageWidth * scale);
      int scaledHeight = (int)(imageHeight * scale);
      int imageX = 200 + (targetWidth - scaledWidth) / 2;
      int imageY = (targetHeight - scaledHeight) / 2;
      DrawTextureEx(imageData.texture, (Vector2){imageX, imageY}, 0.0f, scale,
                    WHITE);
    } else {
      DrawText("Drag and drop Image file", 400, HEIGHT_WINDOW / 2 - 5, 20,
               WHITE);
    }

    // Show top-up message if "Nearest Neighbor" was activated
    if (showMessage) {
      const char *message = "Top-up Successful";
      int textWidth = MeasureText(message, 20);
      DrawText(message, WIDTH_WINDOW / 2 - textWidth / 2,
               HEIGHT_WINDOW / 2 - 10, 20, GREEN);
    }

    EndDrawing();
  }

  // Unload resources and close window
  if (imageLoaded) {
    UnloadTexture(imageData.texture);
    UnloadImage(imageData.original);
    UnloadImage(imageData.processed);
  }
  CloseWindow();

  return 0;
}
