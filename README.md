# ELEC-391
This repository will be used to version control code and notes for our ELEC 391 Project, which will be a keyboard-playing robot.

The goals of the project are as follows:

1.  Given a sheet of music, the robot will be able to read the music and turn it into movement
2.  The robot will quickly and accurately identify where on the keyboard the next note that needs to be played is and will move to that location precisely
3.  The robot will play the note via actuation of some kind

[Sheet Music] => [Desired Location] => [Voltage] =Transmission=> [Stepper Motor] => {Are we in the correct position?} => Actuate
                          /\                                                         ||
                          ||                                                         ||
                           \ = [Error] <= [Voltage] <=Transmission [Motor Encoder] = /

Documentation will be added as the project continues
