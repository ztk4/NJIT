#include <iostream>
#include <string>
#include <bitset>

#include <cstring>

using namespace std;

enum suit { CLUBS, DIAMONDS, HEARTS, SPADES }; //these will be assigned 0-3 consecutively

struct card {
    int val;
    suit s;
};

/*
 * Fills card struct with values from string repr in str
 * return value - true if str repr a valid card
 */
bool fill_card(const char *str, card &out) {
    //fast way to get last character and also useful for getting length of string (with only one iteration over string)
    const char *end = strchr(str, '\0'); 

    //Assign Card Value 
    int len = end - str;
    if(len == 3) {
        if(str[0] == '1' && str[1] == '0')
            out.val = 10;
        else
            return false; //no card is 3 characters unless it is a 10
    } else if(len == 2) {
        switch(*str) { //switch on first character
            case 'A':
                out.val = 1;
                break;
                
            case 'J':
                out.val = 11;
                break;

            case 'Q':
                out.val = 12;
                break;

            case 'K':
                out.val = 13;
                break;

            default: //digits 2-9
                out.val = *str - '0'; //if *str is a digit, this will get the propper value

                if(out.val < 2 || out.val > 9)
                    return false; //not a digit 2-9
        }
    } else
        return false; //if len is not 2 or 3, then str is not a valid card string

    //Assign Card Suit
    switch(*(end - 1)) { //switch on last character of string
        case 'C':
            out.s = CLUBS;
            break;
        
        case 'D':
            out.s = DIAMONDS;
            break;
        
        case 'H':
            out.s = HEARTS;
            break;
     
        case 'S':
            out.s = SPADES;
            break;

        default:
            return false; //invalid suit
    }

    return true; //card is valid
}

/*
 * Processes a stream as one hand
 * returns scores or, if less than 0:
 *  -1 - EOF (before reaching a period)
 *  -2 - Formatting Error
 *  -3 - Wrong Number of Cards
 *  -4 - Duplicate Card
 */
int proc_hand(istream &in) {
    string line;
    card c;
    int err, score, count = 0,
        suit_count[4] = {0},    //{0} -> 0 initialized
        face_count[4] = {0};    //{0} -> 0 initialized; FACES -> {0: JACK, 1: QUEEN, 2: KING, 3: ACE }
    bitset<52> card_count;      //52 false values, one for keeping track of each card
    
    while(true) { //loop for reading in cards

        //read in cards
        do {
            getline(in, line); //gets 1 line of input

            if(in.eof())
                return -1; //EOF (end program)
        } while(!in); //same as while(in.fail()); makes sure at least one line is read

        if(line.size() == 1 && line[0] == '.')
            break; //done fetching cards

        char *tok, tokens[line.size() + 1];
        line.copy(tokens, line.size()); //copy line c++ string to tokens
        tokens[line.size()] = '\0'; //terminate the c string
        tok = strtok(tokens, ", ");

        while(tok) {
            //check for done
            if(++count == 14) {
                err = -3; //wrong num, but needs cleaning
                goto clean;
            }

            //validity checks
            if(!fill_card(tok, c)) { //puts card into c
                err = -2; //formatting error with card
                goto clean;
            }

            int cnum = (c.val + c.s * 13) - 1; //subtraction makes it 0-indexed, c.s will be a value from 0-3
            if(card_count[cnum]) {
                err = -4; //duplicate cards
                goto clean;
            }
            card_count[cnum] = true; //mark card
        
            //increments
            ++suit_count[c.s];
            if(c.val == 1) {
                ++face_count[3];
            } else {
                int tmp = c.val - 11; //0 if jack, 1 if queen, 2 if king, < 0 if anything else
                if(tmp >= 0)
                    ++face_count[tmp];
            }
            
            tok = strtok(NULL, ", ");
        }

    }

    if(count != 13)
        return -3; //wrong number of cards (no cleanup needed)

    //score hand
    
    score = 0;

    for(int i = 0; i < 4; ++i) {
        score += face_count[i] * (i + 1); //1 pt for jack, 2 pts for queen, etc.
        if(suit_count[i] < 3)
            score += 3 - suit_count[i];
        else if(suit_count[i] > 4)
            score += suit_count[i] - 4;
    }

    for(int i = 0; i < 3; ++i) {
        if(face_count[i] == 1) //subtract one for each sole face card (save aces)
            score -= 1;
    }

    if(face_count[3] == 0) //zero aces
        score -= 1;
    else if(face_count[3] == 4) //four aces
        score += 1;

    return score;

clean: //cleans out input until '.' is found
   
    do {
        getline(in, line);

        if(in.eof())
            return -1; //EOF while trying to finish hand
    } while(line.size() != 1 || line[0] != '.');

    return err;
}

/*
 * Process all hands
 */
int main(int argc, char **argv) {
    int hand_num = 1, ret;

    while( (ret = proc_hand(cin)) != -1) { //while EOF not reached
        switch(ret) {
            case -2:
                cout << "BAD FORMAT";
                break;
            case -3:
                cout << "WRONG NUMBER OF CARDS";
                break;
            case -4:
                cout << "DUPLICATE CARDS";
                break;

            default:
                cout << "HAND " << hand_num++ << " TOTAL POINTS " << ret;
                break;
        }

        cout << endl;
    }

    return 0;
}
