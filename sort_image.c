#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

typedef uint8_t BYTE;

typedef struct
{
    BYTE *bytes;
    size_t size;
    size_t capacity;
} ByteArray;

void appendByte(ByteArray *arr, BYTE *b, size_t count);
void freeByteArray(ByteArray* arr);
int compare(const void* a, const void* b);

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("Please provide an input image and an output image.\n");
        return 1;
    }

    FILE *fIn = NULL;
    FILE *fOut = NULL;

    fopen_s(&fIn, argv[1], "rb");
    fopen_s(&fOut, argv[2], "wb");

    if (fIn == NULL || fOut == NULL)
    {
        printf("Error opening files.\n");
        fclose(fIn);
        fclose(fOut);
        return 1;
    }

    BYTE header[54];
    fread(header, sizeof(BYTE), 54, fIn);
    fwrite(header, sizeof(BYTE), 54, fOut);

    int width = *(int*)&header[18];
    int height = abs(*(int*)&header[22]);
    int bitdepth = *(int*)&header[28];

    if (bitdepth != 24)
    {
        fclose(fIn);
        fclose(fOut);
        printf("Image is not 24bits.\n");
        return 1;
    }

    size_t initialCapacity = width * height * 3; // 3 bytes per pixel
    ByteArray *arr = malloc(sizeof(ByteArray));
    arr->bytes = malloc(initialCapacity);
    arr->size = 0;
    arr->capacity = initialCapacity;

    BYTE *pixel = malloc(width * 3);
    for (int x = 0; x < width; x++)
    {
        for (int y = 0; y < height; y++)
        {
            size_t bytesRead = fread(pixel, sizeof(BYTE), width * 3, fIn);
            appendByte(arr, pixel, bytesRead);
        }
    }
    free(pixel);

    // sort da pixels (treating 3 bytes (Blue Green Red) as a unit )
    qsort(arr->bytes, arr->size / 3, sizeof(BYTE) * 3, compare);

    fwrite(arr->bytes, sizeof(BYTE), arr->size, fOut);

    freeByteArray(arr);
    fclose(fIn);
    fclose(fOut);

    return 0;
}

void appendByte(ByteArray *arr, BYTE *b, size_t count)
{
    if (arr->size + count > arr->capacity)
    {
        while (arr->size + count > arr->capacity)
        {
            arr->capacity *= 2;
        }

        arr->bytes = realloc(arr->bytes, arr->capacity);
    }

    memcpy(arr->bytes + arr->size, b, count);
    arr->size += count;
}

void freeByteArray(ByteArray* arr) {
    free(arr->bytes);
    free(arr);
}

int compare(const void* a, const void* b) {
    const BYTE* pa = (const BYTE*)a;
    const BYTE* pb = (const BYTE*)b;

    for (int i = 0; i < 3; i++) {
        if (pa[i] != pb[i]) {
            return pa[i] - pb[i];
        }
    }
    return 0;
}
