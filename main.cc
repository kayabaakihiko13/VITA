#include "raylib.h"
#include <string.h>
#include <stdio.h>
#include "include/control.h"

int main(void)
{
    InitWindow(WIDTH_WINDOW, HEIGHT_WINDOW, "Image Upload GUI");

    // Show image icon
    Image icon = LoadImage("assets/logo.png");
    SetWindowIcon(icon);
    UnloadImage(icon); // Clean up icon memory

    Button buttons[4] = {
        {{0, 10, 200, 50}, "Nearest Neighbor", {109, 104, 117, 255}},
        {{0, 60, 200, 50}, "Grid Uniform Distribution", {134, 129, 140, 255}},
        {{0, 110, 200, 50}, "Poisson disc", {159, 154, 163, 255}},
        {{0, 160, 200, 50}, "HRAA", {184, 179, 186, 255}}
    };

    Texture2D texture = {0};
    bool imageLoaded = false;

    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        // Handle image loading from file drop
        if (IsFileDropped())
        {
            FilePathList droppedFiles = LoadDroppedFiles();
            if (droppedFiles.count > 0)
            {
                if (imageLoaded) UnloadTexture(texture);

                Image image = LoadImage(droppedFiles.paths[0]);
                ScaleImage(&image, WIDTH_WINDOW - 200, HEIGHT_WINDOW); // Scale the image
                texture = LoadTextureFromImage(image);
                UnloadImage(image);

                imageLoaded = true;
            }
            UnloadDroppedFiles(droppedFiles);
        }

        // Draw everything
        BeginDrawing();
        ClearBackground((Color){222, 222, 222, 255}); // Light gray background

        // Draw buttons and handle button clicks
        for (int i = 0; i < 4; i++)
        {
            if (CheckCollisionPointRec(GetMousePosition(), buttons[i].bounds)) {
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    buttons[i].color = (Color){255, 0, 0, 255}; // Red color when clicked
                    printf("Button '%s' clicked!\n", buttons[i].label);
                } else {
                    buttons[i].color = (Color){150, 150, 150, 255}; // Light gray when hovered
                }
            } else {
                buttons[i].color = (Color){109, 104, 117, 255}; // Reset color when not hovered
            }

            DrawButton(buttons[i]);
        }

        // Draw image area
        DrawRectangle(200, 0, WIDTH_WINDOW - 200, HEIGHT_WINDOW, BLACK);

        if (imageLoaded)
        {
            int imageX = 200 + ((WIDTH_WINDOW - 200) - texture.width) / 2;
            int imageY = (HEIGHT_WINDOW - texture.height) / 2;
            DrawTexture(texture, imageX, imageY, WHITE);
        }
        else
        {
            DrawText("Drag and drop Image file", 400, HEIGHT_WINDOW / 2 - 5, 20, WHITE);
        }

        EndDrawing();
    }

    if (imageLoaded) UnloadTexture(texture);
    CloseWindow();

    return 0;
}
