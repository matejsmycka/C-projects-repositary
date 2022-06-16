#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
/* Writes number to terminal, this funtion is not neccesarry, but I think it helpes sometimes to be more clear*/
void writeToTerminal(uint64_t NumberInAkumulator) {
    printf("# %" PRIu64 "\n", NumberInAkumulator);
}

uint64_t enumerate(int character, uint64_t NumberFirst, uint64_t NumberSecond) {
    if (character == '+') {
        return NumberFirst + NumberSecond;
    }
    if (character == '-') {
        return NumberFirst - NumberSecond;
    }
    if (character == '*') {
        return NumberFirst * NumberSecond;
    }
    if (character == '/') {
        if(NumberSecond == 0){
            fprintf(stderr, "%s\n", "Division by zero");
            exit(EXIT_FAILURE);
        }
        return NumberFirst / NumberSecond;
    }
    if (character == '%') {
        if(NumberSecond == 0){
            fprintf(stderr, "%s\n", "Division by zero");
            exit(EXIT_FAILURE);
        }
        return NumberFirst % NumberSecond;
    }
    if (character == '<') {
        return NumberFirst << NumberSecond;
    }
    if (character == '>') {
        return NumberFirst >> NumberSecond;
    }
    return 0;
}

void evaluate(int ArgChar, uint64_t NumberInAkumulator, uint64_t * Number) {
    if (ArgChar != 0) {
        *Number = enumerate(ArgChar, NumberInAkumulator, * Number);
    }
}

uint64_t GetDecimalFromBinary(uint64_t Number) {
    uint64_t decimal = 0, base = 1, remainder;
    uint64_t temp = Number;
    while (temp > 0) {
        remainder = temp % 10;
        decimal = decimal + remainder * base;
        temp = temp / 10;
        base = base * 2;
    }
    return decimal;
}

uint64_t GetDecimalFromOctal(uint64_t Number) {
    uint64_t num = Number;
    uint64_t decimal = 0;
    uint64_t base = 1;
    uint64_t temp = num;
    while (temp) {
        uint64_t last_digit = temp % 10;
        temp = temp / 10;
        decimal += last_digit * base;
        base = base * 8;
    }
    return decimal;
}

uint64_t GetBinaryFromDecimal(uint64_t Number) {
    uint64_t binary = 0;
    uint64_t rem, temp = 1;
    while (Number != 0) {
        rem = Number % 2;
        Number = Number / 2;
        binary = binary + rem * temp;
        temp = temp * 10;
    }
    return binary;
}

uint64_t systemGetDecimal(int * system, uint64_t Number, uint64_t NumberInAkumulator) {
    if ( * system == 0) {
        return Number;
    }
    if ( * system == 3) {
        /*HexaDecimal*/
        if (Number == 0) {
            * system = 0;
            return NumberInAkumulator;

        }
        * system = 0;
        return Number;
    }
    if ( * system == 2) {
        /*Octal*/
        if (Number == 0) {
            * system = 0;
            return NumberInAkumulator;
        }

        * system = 0;
        return GetDecimalFromOctal(Number);
    }
    if ( * system == 1) {
        /*Binary*/
        if (Number == 0) {
            writeToTerminal(GetBinaryFromDecimal(NumberInAkumulator));
            * system = 0;
            return NumberInAkumulator;
        }
        * system = 0;

        return GetDecimalFromBinary(Number);
    }
    fprintf(stderr, "%s\n", "Syntax error");
    return EXIT_FAILURE;
}

void GetDigit(int character, uint64_t * Number, bool FirstDigit, int system) {
    if (system != 3) {
        character -= 48;
        if (FirstDigit) {
            * Number = character;
        } else {
            * Number *= 10;
            * Number += character;
        }
    } else {
        /*HEXADECIMAL*/
        /*Převod na ciselne hodnoty*/
        if (character == 65) {
            character = 10;
            /*A*/
        } else if (character == 66) {
            character = 11;
        } else if (character == 67) {
            character = 12;
        } else if (character == 68) {
            character = 13;
        } else if (character == 69) {
            character = 14;
            /*F*/
        } else if (character == 70) {
            character = 15;
        } else {
            character -= 48;
        }
        if ( * Number == 0) {
            * Number = character;
        } else {
            * Number *= 16;
            * Number += character;
        }
    }
}

/* MAIN, chtel jsem ji rozlozit, ale spise jsem si to mel vice rozmyslet na zacatku, ted uz by to bylo optizne. Priste to udelam.*/
int main() {
    int character;
    bool FirstDigit = true;
    bool akumulator = false;
    bool Comment = false;
    bool evaluated = false;
    int NumeralSystem = 0;
    int ArgChar = 0;
    uint64_t Number = 0;
    uint64_t NumberInAkumulator = 0;
    uint64_t NumberInMemory = 0;


        while ((character = getchar()) != EOF) {

            if (character == '\n') {
                if ((ArgChar != 0 && Number != 0)&& (!evaluated && ArgChar != 0 )&& (character != 'T' && character != 'X' && character != 'O' && character != 'm')) {
                    evaluate(ArgChar, NumberInAkumulator, & Number);
                    NumberInAkumulator = Number;
                    evaluated = false;
                    Number = 0;
                    if (!akumulator) {
                        ArgChar = 0;
                    }
                }

            Comment = false;
        }
        if (!isspace(character)) {
            if (!Comment) {
                if (character > 96 && character < 103) {
                    character = toupper(character);
                }
                /*zoufalá doba plodí zoufalá řešení*/
                if (!isdigit(character) &&
                    character != '\n' && character != ';' &&
                    character != 'M' && character != 'P' &&
                    character != 'O' && character != 'X' &&
                    character != 'T' && character != '+' &&
                    character != '-' && character != '*' &&
                    character != '/' && character != '%' &&
                    character != '<' && character != '>' &&
                    character != '=' && character != '^' &&
                    character != 'R' && character != 'N' &&
                    character != 'm' &&
                    (character <= 64 || character >= 71)) {
                    fprintf(stderr, "%s\n", "Syntax error");
                    return EXIT_FAILURE;
                }

                if (isdigit(character) || (character > 64 && character < 71)) {

                    /*ERROR CHECK*/
                    if (((character > 64 && character < 71) && NumeralSystem != 3)
                    || ((ArgChar == 0 && NumeralSystem == 0)&&(!akumulator))
                    ||(NumeralSystem == 1 && character > 49)
                    ||(NumeralSystem == 2 && character > 55)){
                        fprintf(stderr, "%s\n", "Syntax error");
                        return EXIT_FAILURE;
                    }
                        GetDigit(character, & Number, FirstDigit, NumeralSystem);
                        FirstDigit = false;
                } else {
                    Number = systemGetDecimal( & NumeralSystem, Number, NumberInAkumulator);
                    if ((character == 'T' || character == 'X' || character == 'O') && Number !=0){
                        NumberInAkumulator =Number;
                    }
                    /*VYHODNOCUJE VYRAZ PRVNE KDYZ JE AKUMULATOR*/
                    if (akumulator && (character != 'T' && character != 'X' && character != 'O'&& character != 'm')) {
                        if (ArgChar == 0) {
                            NumberInAkumulator = Number;
                            writeToTerminal(NumberInAkumulator);

                        }
                    }
                    /*VYHODNOCUJE VYRAZ*/
                    if (ArgChar != 0 && (character != 'T' && character != 'X' && character != 'O' && character != 'm')) {
                        if (character != 61 || Number !=0) {
                            evaluate(ArgChar, NumberInAkumulator, &Number);
                            NumberInAkumulator = Number;
                        }
                        if (character != 'M'){
                        writeToTerminal(NumberInAkumulator);}
                        evaluated = true;
                    }

                    Number = 0;
                    /*PRIKAZY*/
                    if (character == 'P') {
                        akumulator = true;
                    } else if (character == 'M') {
                        NumberInMemory += NumberInAkumulator;
                    } else if (character == 'N') {
                        NumberInAkumulator = 0;
                        writeToTerminal(NumberInAkumulator);

                    } else if (character == 'm') {
                        Number = NumberInMemory;

                    } else if (character == 'R') {
                        NumberInMemory = 0;
                    } else if (character == 'T') {
                            writeToTerminal(GetBinaryFromDecimal(NumberInAkumulator));
                        NumeralSystem = 1;
                    } else if (character == 'O') {
                            printf("# %"
                                   PRIo64 "\n", NumberInAkumulator);
                        NumeralSystem = 2;
                    } else if (character == 'X') {
                            printf("# %"
                                   PRIX64 "\n", NumberInAkumulator);
                        NumeralSystem = 3;
                    } else if (character == '+' || character == '-' || character == '*' || character == '/' ||
                               character == '%' || character == '<' || character == '>') {
                        ArgChar = character;
                        NumeralSystem = 0;

                    } else if (character == '=') {
                        writeToTerminal(NumberInAkumulator);
                        ArgChar = 0;
                        akumulator = false;
                    } else if (character == ';') {
                        NumeralSystem = 0;
                        ArgChar = 0;
                        Comment = true;
                        akumulator = false;

                    }
                }
            }

        }
    }
    /*EOF*/
    if (akumulator) {
        evaluate(ArgChar, NumberInAkumulator, & Number);
        NumberInAkumulator = Number;
        writeToTerminal(NumberInAkumulator);
    }
    return EXIT_SUCCESS;
}
