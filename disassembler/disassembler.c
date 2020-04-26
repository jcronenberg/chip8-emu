#include <stdio.h>
#include <stdlib.h>

int disassemble8080Op(unsigned char *, int);

int main (int argc, char *argv[])
{

    FILE *readfile;
    char *buffer = NULL;
    long bufsize;

    if (argc > 2) {
        printf("Too many arguments\n");
        exit(EXIT_FAILURE);
    } else if (argc == 1) {
        printf("Please specify a file to disassemble\n");
        exit(EXIT_FAILURE);
    }

    printf("Dissasembling file: %s\n", argv[1]);

    readfile = fopen(argv[1], "r");
    if (readfile == NULL) {
        printf("Error reading file\n");
        exit(EXIT_FAILURE);
    }

    if (fseek(readfile, 0L, SEEK_END) == 0) {
        bufsize = ftell(readfile);
        if (bufsize == -1) { printf("Error bufsize"); }

        buffer = malloc(sizeof(char) * (bufsize + 1));

        if (fseek(readfile, 0L, SEEK_SET) != 0) { printf("Error with fseek"); }

        size_t newLen = fread(buffer, sizeof(char), bufsize, readfile);

        if (ferror(readfile) != 0) {
            printf("Error reading file");
        } else {
            buffer[newLen++] = '\0'; /* Just to be safe. */
        }
    }

    fclose(readfile);

    //Disassemble
    for (int i = 0; i < bufsize;)
        i += disassemble8080Op(buffer, i);

    free(buffer);
}

/*
 * codebuffer is a valid pointer to 8080 assembly code
 * pc is the current offset into the code
 *
 * returns the number of bytes of the op
*/
int disassemble8080Op(unsigned char *codebuffer, int pc)
{
    unsigned char *code = &codebuffer[pc];
    int opbytes = 1;
    printf ("%04x ", pc);
    switch (*code)
    {
        case 0x00: printf("NOP"); break;
        case 0x01: printf("LXI    B,#$%02x%02x", code[2], code[1]); opbytes = 3; break;
        case 0x02: printf("STAX   B"); break;
        case 0x03: printf("INX    B"); break;
        case 0x04: printf("INR    B"); break;
        case 0x05: printf("DCR    B"); break;
        case 0x06: printf("MVI    B,#$%02x", code[1]); opbytes = 2; break;
        case 0x07: printf("RLC"); break;
        case 0x08: printf("NOP"); break;
        case 0x09: printf("DAD    B"); break;
        case 0x0a: printf("LDAX   B"); break;
        case 0x0b: printf("DCX    B"); break;
        case 0x0c: printf("INR    C"); break;
        case 0x0d: printf("DCR    C"); break;
        case 0x0e: printf("MVI    C,#$%02x", code[1]); opbytes = 2; break;
        case 0x0f: printf("RRC"); break;
        case 0x10: printf("NOP"); break;
        case 0x11: printf("LXI    D,#$%02x%02x", code[1], code[2]); opbytes = 3; break;
        /* ........ */
        case 0x3e: printf("MVI    A,#0x%02x", code[1]); opbytes = 2; break;
        /* ........ */
        case 0xc3: printf("JMP    $%02x%02x", code[2], code[1]); opbytes = 3; break;
        /* ........ */
	default: printf("Unknown operation"); break;
    }

    printf("\n");

    return opbytes;
}
