#ifndef CONTROL_H
#define CONTROL_H

#define WIDTH_WINDOW 900
#define HEIGHT_WINDOW 600
#include <raylib.h>
#include <string.h>

// Define the Button structure
typedef struct Button {
    Rectangle bounds;  // Rectangle representing the button's position and size
    const char *label; // Label of the button
    Color color;       // Button color
} Button;

// Function to scale an image to fit within the specified maximum dimensions
void ScaleImage(Image *image, int maxWidth, int maxHeight) {
    float scaleWidth = (float)maxWidth / image->width;
    float scaleHeight = (float)maxHeight / image->height;
    float scale = scaleWidth < scaleHeight ? scaleWidth : scaleHeight;
    
    int newWidth = (int)(image->width * scale);
    int newHeight = (int)(image->height * scale);
    
    // Resize the image
    ImageResize(image, newWidth, newHeight);
}

// Function to draw a button on the screen
void DrawButton(Button button) {
    DrawRectangleRec(button.bounds, button.color); // Draw button background
    DrawText(button.label, button.bounds.x + 10, button.bounds.y + 10, 20, WHITE); // Draw button label
}

// Function to detect if a button is clicked
bool isButtonClicked(Button button) {
    if (CheckCollisionPointRec(GetMousePosition(), button.bounds) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        return true;
    }
    return false;
}

#endif // CONTROL_H
