__kernel void hello(__global char* string)
    {
    string[0] = 'S';
    string[1] = 't';
    string[2] = 'e';
    string[3] = 'r';
    string[4] = 'f';
    string[5] = ',';
    string[6] = ' ';
    string[7] = 'T';
    string[8] = 'i';
    string[9] = 'm';
    string[10] = '!';
    string[11] = '\0';
}