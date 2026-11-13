#include <stdio.h>

int main() {
    const char c = 'A';
    char hex_str[3]; // 2 hex chars + '\0'
    snprintf(hex_str, sizeof(hex_str), "%02x", (unsigned char)c);
    for(size_t i = 0; i < sizeof(hex_str); i++)
        printf("%c", hex_str[i]);

    printf("\n");
}