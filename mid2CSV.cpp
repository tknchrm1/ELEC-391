/*
LED Playing Code
Author: Tsz Kin Charm
Date: March 2nd, 2026
Description: Parses MIDI byte stream and converts into a CSV file containing a list of events. 
This can then be sent to a MCU for action.
*/

#include <cstdint>
#include <stdio.h>

int solenoidStatus;

int main() {

    int n = 0;

    FILE *currentTrack, *outputFile;

    currentTrack = fopen("Mary.mid","rb"); //Opens midi file in binary read mode
    outputFile = fopen("testOUT.csv","w+"); //Opens file in write mode

    //Checks for file opening error
    if (!currentTrack) {
        perror("Error opening file");
        return 1;
    };

    if (!outputFile) {
        perror("Error opening file");
        return 1;
    };

    // Skip header + chunk size bytes (adjust for your file)
    fseek(currentTrack, 64, SEEK_SET); //Replace with something that looks through header

    int status, noteNumber, velocity;
    unsigned int deltaTime, deltaTimeValue;
    int cursor;

    while (1) {

        //Read Delta Time while MSB is 1
        deltaTimeValue = 0;
        do {
            deltaTime = fgetc(currentTrack); //Load Delta Time
            if (deltaTime == EOF) goto endOfFile; //Check for EOF
            deltaTimeValue = (deltaTimeValue << 7) | (deltaTime & 0x7F); //Sum the Delta Time Values
        } while (((deltaTime & 0xF0) != 0x90)&&((deltaTime & 0xF0) != 0x80)&&((deltaTime & 0xF0)  != 0xC0)&&(deltaTime != 0xFF));

        //Read Status
        status = fgetc(currentTrack);

        //Check for Meta Event
        if ((status & 0xF0) == 0x90 || (status & 0xF0) == 0x80) {
            // Note ON or OFF
            noteNumber = fgetc(currentTrack);
            velocity   = fgetc(currentTrack);

            //Output for verification
            printf("DeltaTime: %u, Status: 0x%02X SolenoidStatus: %d, Note: %d, Velocity: %d\n",
                    deltaTimeValue, status, solenoidStatus, noteNumber, velocity);
            
            fprintf(outputFile, "%03u, %d, %d, %03d\n",
                    deltaTimeValue, solenoidStatus, noteNumber, velocity);

        } else if ((status & 0xF0) == 0xC0) {
            // Program Change
            int programNumber = fgetc(currentTrack);
            printf("DeltaTime: %u, Program Change: %d\n", deltaTimeValue, programNumber);
        } else if (status == 0xFF) {
            // Meta Event
            int metaType = fgetc(currentTrack);
            int length   = fgetc(currentTrack); // assume 1 byte for simplicity
            // Skip meta data bytes
            for (int i = 0; i < length; i++) fgetc(currentTrack);

            // End-of-Track
            if (metaType == 0x2F) break;
        } else {
            printf("Unknown status: 0x%02X\n", status);
            break;
        }

    };

    endOfFile:
    fclose(currentTrack); //Closes Track

    return 0;
};

/*
switch(status & 0xF0){   
            case 0x90:
                printf("Turning ON note #%d\n", noteNumber);
                break;
            case 0x80:
                printf("Turning OFF note #%d\n", noteNumber);
                break;
            default:
                printf("Unknown MIDI event: 0x%X\n", status & 0xF0);
                break;
        }

        switch(status & 0xF0){   
            case 0x90:
                printf("Turning ON note #%d\n", noteNumber);
                break;
            case 0x80:
                printf("Turning OFF note #%d at %d out of 32 beats\n", noteNumber, countingTime);
                break;
            default:
                printf("Unknown MIDI event: 0x%X\n", status & 0xF0);
                break;
        }
        
        

        if(countingTime >= 32)
        {
            bar++;
            printf("bar %d\n", bar);
            countingTime = 0;
        }

        n++;
    }
*/