#include "raylib.h"
#include <string.h>
#include "include/control.h"

typedef struct {
    Rectangle rect;
    const char *text;
    Color color;
} Button;

int main(void)
{
    InitWindow(WIDTH_WINDOW, HEIGHT_WINDOW, "Image Upload GUI");
    // show image icon
    Image icon = LoadImage("assets/logo.png");
    SetWindowIcon(icon);
    // membersihkan memori pada gambar icon
    UnloadImage(icon);
    Button buttons[4] = {
        {{0, 0, 200, 50}, "Nearest Neighbor", {109, 104, 117, 255}},
        {{0, 50, 200, 50}, "Nearest Neighbor", {134, 129, 140, 255}},
        {{0, 100, 200, 50}, "", {159, 154, 163, 255}},
        {{0, 150, 200, 50}, "", {184, 179, 186, 255}}
    };

    Texture2D texture = {0};
    bool imageLoaded = false;

    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        // Update
        if (IsFileDropped())
        {
            FilePathList droppedFiles = LoadDroppedFiles();
            if (droppedFiles.count > 0)
            {
                if (imageLoaded) UnloadTexture(texture);

                // Load the image first
                Image image = LoadImage(droppedFiles.paths[0]);

                // Scale the image
                ScaleImage(&image, WIDTH_WINDOW - 200, HEIGHT_WINDOW);

                // Convert Image to Texture
                texture = LoadTextureFromImage(image);

                // Unload image from CPU memory after converting to texture
                UnloadImage(image);
                
                imageLoaded = true;
            }
            UnloadDroppedFiles(droppedFiles);
        }

        // Draw
        BeginDrawing();
        ClearBackground((Color){222, 222, 222, 255}); // Light gray background

        // Draw buttons
        for (int i = 0; i < 4; i++)
        {
            DrawRectangleRec(buttons[i].rect, buttons[i].color);
            if (strlen(buttons[i].text) > 0)
            {
                DrawText(buttons[i].text, buttons[i].rect.x + 10, buttons[i].rect.y + 15, 20, WHITE);
            }
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
            DrawText("Drag and drop Image file", 300, HEIGHT_WINDOW / 2 - 10, 20, WHITE);
        }

        EndDrawing();
    }

    if (imageLoaded) UnloadTexture(texture);
    CloseWindow();

    return 0;
}
