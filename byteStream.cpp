/*
LED Playing Code
Author: Tsz Kin Charm
Date: March 2nd, 2026
Description: Parses MIDI byte stream and converts into a CSV file containing a list of events. 
This can then be sent to a MCU for action.
*/

#include <cstdint>
#include <stdio.h>

int main() {

    unsigned int byte;

    FILE *currentTrack, *outputFile;

    currentTrack = fopen("Mary.mid","rb"); //Opens midi file in binary read mode

    if(!currentTrack)
    {
        perror("Could not open Current Track");
        return -1;
    }

    byte = fgetc(currentTrack);
    printf("%#02x", byte);

    while (byte != EOF)
    {
        byte = fgetc(currentTrack);
        printf("%#02x\n", byte);
    }

    return 0;
}
