#include "include/SSAA.h"
#include "include/control.h"
#include <raylib.h>
#include <stdlib.h>
#include <string.h>

#define MAX_BUTTONS 4
#define ZOOM_FACTOR 1.2f
// Time in seconds for the message to be displayed
#define MESSAGE_DISPLAY_TIME 3.0

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
  // Whether to show the message
  bool showMessage = false;
  // Time when message should disappear
  float messageTime = 0.0f;
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
          ImageFormat(&imageData.original, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
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

          // Compress the image and apply Grid Uniform Distribution
          // supersampling
          CompressImage(&imageData.processed);
          ApplyGridUniformDistribution(&imageData.processed,
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
          switch (i) {
          case 0: // Nearest Neighbor
            applyNearestNeighbor(&imageData.processed, 1.0f);
            break;
          case 1: // Grid Uniform Distribution
            ApplyGridUniformDistribution(&imageData.processed, 3);
            break;
          case 2: // Poisson disc
            ApplyPoissonDiscSampling(&imageData.processed, 3, 0.5);
            break;
          case 3: // HRAA
            ApplyHRAA(&imageData.processed, 1);
            break;
          }
          buttons[i].color = (Color){255, 0, 0, 255}; // Red color when clicked
          showMessage = true;
          messageTime = GetTime();
        } else {
          buttons[i].color =
              (Color){150, 150, 150, 255}; // Light gray when hovered
        }
      }
    }

    // Check if the message should still be displayed
    if (showMessage && GetTime() - messageTime > MESSAGE_DISPLAY_TIME) {
      showMessage = false; // Hide message after the specified time
    }

    BeginDrawing();
    ClearBackground((Color){222, 222, 222, 255}); // Light gray background

    // Draw buttons
    for (int i = 0; i < MAX_BUTTONS; i++) {
      DrawRectangleRec(buttons[i].bounds, buttons[i].color);
      DrawText(buttons[i].label, buttons[i].bounds.x + 10,
               buttons[i].bounds.y + 15, 20, BLACK);
    }

    // Draw image area background
    DrawRectangle(200, 0, WIDTH_WINDOW - 200, HEIGHT_WINDOW, BLACK);

    // Draw the image or display the drag-and-drop message
    if (imageLoaded) {
      int imageX = 200 + ((WIDTH_WINDOW - 200) - imageData.texture.width) / 2;
      int imageY = (HEIGHT_WINDOW - imageData.texture.height) / 2;
      DrawTexture(imageData.texture, imageX, imageY, WHITE);
    } else {
      int textWidth = MeasureText("Drag and drop Image file", 20);
      DrawText("Drag and drop Image file",
               200 + (WIDTH_WINDOW - 200 - textWidth) / 2,
               HEIGHT_WINDOW / 2 - 10, 20, WHITE);
    }

    // Show top-up message if "Nearest Neighbor" was activated
    if (showMessage) {
      const char *msg_success = "Top-up Successful";
      const char *msg_warning = "Please drag and drop your Picture";
      int textWidth_success = MeasureText(msg_success, 20);
      if (imageData.original.data == NULL) {
        DrawText(msg_warning, WIDTH_WINDOW / 2 - textWidth_success / 2,
                 HEIGHT_WINDOW / 2 - 10, 20, RED);
      } else {
        DrawText(msg_success, WIDTH_WINDOW / 2 - textWidth_success / 2,
                 HEIGHT_WINDOW / 2 - 10, 20, GREEN);
      }
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
