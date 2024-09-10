#ifndef CONTROL_H
#define CONTROL_H

#define WIDTH_WINDOW 900
#define HEIGHT_WINDOW 600
#include <raylib.h>
#include <string.h>

// Fungsi untuk mengubah ukuran gambar sesuai dengan skala maksimum
void ScaleImage(Image *image, int maxWidth, int maxHeight) {
    float scaleWidth = (float)maxWidth / image->width;
    float scaleHeight = (float)maxHeight / image->height;
    float scale = scaleWidth < scaleHeight ? scaleWidth : scaleHeight;
    
    int newWidth = (int)(image->width * scale);
    int newHeight = (int)(image->height * scale);
    
    // Resize image
    ImageResize(image, newWidth, newHeight);
}

#endif //CONTROL_H
