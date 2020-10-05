/* serialkey.cc - functions to implement a serial key

  -----
  isvalidkey() - checks if a key is valid

  syntax: short isvalidkey(char *username, int userlen, char *serial)
        where input:
                username is a string (max 20 char) representing the user's name
                userlen is the number of characters in username
                serial is the serial key
        where output:
                 1 = valid key
                -1 = corrupt key
                -2 = license expired
                -3 = inconsistancy with date
  -----
  decryptkey() - decrypts a key

  syntax: int* decryptkey(char *username, int userlen, char *serial)
        where input:
                username is a string (max 20 char) representing the user's name
                userlen is the number of characters in username
                serial is the serial key
        where output:
                a pointer to an array of integers containing the decrypted key
  -----
  encryptkey() - creates a key

  syntax: char *encryptkey(char *username, int userlen, char *previouskey, int licMonths, int licYears)
        where input:
                username is a string (max 20 char) representing the user's name
                userlen is the number of characters in username
                previouskey is a string of 12 char that contains an old key from which to create a new key from (use NULL if generating a key from scratch)
                licMonths is the length of the license in month (if generating a new key; otherwise information is extracted from previouskey)
                licYears is the length of the license in years (if generating a new key; otherwise information is extracted from previouskey)
        where output:
                a pointer to an array of characters with the new serial key
  -----

  by Karin Blank (karin.blank@nasa.gov) for HSEG
  modified by James C. Tilton (James.C.Tilton@nasa.gov)
*/

#include "serialkey.h"
#include <params/params.h>
#include <cstdio>
#include <cstring>
#include <cctype>
#include <ctime>
#include <string>
#include <fstream>
#include <iostream>
#include <iomanip>

extern HSEGTilton::Params params;

using namespace std;

// Forward function declarations
int* decryptkey(char *username, int userlen, char *serial);
short isvalidkey(char *username, int userlen, char *serial);
char *encryptkey(char *username, int userlen, char *previouskey, int licMonths, int licYears);

int serialkey()
{
        string user_name;
        char *c_user_name;
        int user_len = 0;
        string serial_key;
        char *c_serial_key;
        int validate;
        string new_key;
        char *newkey;
        bool new_flag = false;

        /*
           Check for a file in which the user_name and serial information is cached so user
           does not have to enter it every time. If such as file is found, the program
           reads the information from file.  Otherwise, the program queries the user for
           the information.
        */

        const char *tmpdir;
        tmpdir = getenv("TMP");
        if (tmpdir == NULL)
          tmpdir = getenv("TEMP");
        if (tmpdir == NULL)
          tmpdir = getenv("TMPDIR");
        if (tmpdir == NULL)
        {
          tmpdir = (char *) malloc(5*sizeof(char));
          string tmp = "/tmp";
          tmpdir = tmp.c_str();
        }
        if (tmpdir == NULL)
        {
          tmpdir = (char *) malloc(2*sizeof(char));
          tmpdir = ".";
        }

        string tempfile = tmpdir;
        tempfile += "/RHSEG_Serial_Key.txt";

        ifstream in_file;
        in_file.open(tempfile.c_str( ));

        if (in_file.fail( ))
        {
cout << "Did not find serialkey file: " << tempfile << endl << endl;
            if (params.gtkmm_flag)
              return false;

            new_flag = true;

            //get user_name
            do
            {
                cout << endl << "Enter user name [max 20 char]: ";
                cin >> user_name;
                user_len = strlen(user_name.c_str( ));
            } while(user_len > 20);

            //get serialkey
            do
            {
                cout << endl << "Enter serial key [12 char]: ";
                cin >> serial_key;
            } while(strlen(serial_key.c_str( )) != 12);
        }
        else
        {
            in_file >> user_name;
            in_file >> serial_key;
            in_file.close( );
            user_len = strlen(user_name.c_str( ));
        }

        c_user_name = (char *) malloc((unsigned)(user_len+1)*sizeof(char));
        sprintf(c_user_name,"%s",user_name.c_str( ));
        c_serial_key = (char *) malloc((unsigned) 13*sizeof(char));
        sprintf(c_serial_key,"%s",serial_key.c_str( ));

        validate = isvalidkey(c_user_name, user_len, c_serial_key);

        switch(validate)
        {
        case 1: //key accepted
                if (new_flag)
                    cout << "Valid key and username entered!" << endl;
                break;
        case -1: //key rejected, information corrupt
                if (params.gtkmm_flag)
                  return false;
                cout << "Unable to decrypt key. Reenter key and username." << endl;
                //get user_name
                do
                {
                    cout << endl << "Enter user name [max 20 char]: ";
                      cin >> user_name;
                      user_len = strlen(user_name.c_str( ));
                } while(user_len > 20);

                //get serialkey
                do
                {
                    cout << endl << "Enter serial key [12 char]: ";
                    cin >> serial_key;
                } while(strlen(serial_key.c_str( )) != 12);

                c_user_name = (char *) malloc((unsigned)(user_len+1)*sizeof(char));
                sprintf(c_user_name,"%s",user_name.c_str( ));
                c_serial_key = (char *) malloc((unsigned) 13*sizeof(char));
                sprintf(c_serial_key,"%s",serial_key.c_str( ));

                validate = isvalidkey(c_user_name, user_len, c_serial_key);

                if (validate == 1)
                {
                    cout << "Valid key and username entered!" << endl;
                }
                else
                {
                    cout << "Unable to decrypt key. Check for valid key and username" << endl;
                    return false;
                }
                break;
        case -2: //key rejected, license expired
                if (params.gtkmm_flag)
                  return false;
                cout << "Sorry, license has expired. Enter new key information, if available." << endl;
                //get user_name
                do
                {
                    cout << endl << "Enter user name [max 20 char]: ";
                      cin >> user_name;
                      user_len = strlen(user_name.c_str( ));
                } while(user_len > 20);

                //get serialkey
                do
                {
                    cout << endl << "Enter serial key [12 char]: ";
                    cin >> serial_key;
                } while(strlen(serial_key.c_str( )) != 12);

                c_user_name = (char *) malloc((unsigned)(user_len+1)*sizeof(char));
                sprintf(c_user_name,"%s",user_name.c_str( ));
                c_serial_key = (char *) malloc((unsigned) 13*sizeof(char));
                sprintf(c_serial_key,"%s",serial_key.c_str( ));

                validate = isvalidkey(c_user_name, user_len, c_serial_key);

                if (validate == 1)
                {
                    cout << "Valid key and username entered!" << endl;
                }
                else
                {
                    cout << "Unable to decrypt key. Check for valid key and username" << endl;
                    return false;
                }
                break;
        case -3: //key rejected, current date is earlier than date program was last run
                if (params.gtkmm_flag)
                  return false;
                cout << "Current date is inconsistant with date program was last run." << endl;
                cout << "Please check the system clock." << endl;
                return false;
                break;
        default: //default action, reject
                if (params.gtkmm_flag)
                  return false;
                cout << "Unknown error with key or username" << endl;
                return false;
        }

        //***only a valid key will execute this code***

        //generate a new key with a new timestamp
        newkey = encryptkey(c_user_name, user_len, c_serial_key, 0, 0);
        new_key = newkey;

        /*
           This new key should overwrite the old key where it was stored. This process makes it harder for the user
           to get around the license by changing the date (since the new key has information on when the program was
           last run). The user can still reset the date by deleting the file with the key, so it is a + if this file
           is a little hard to find.
        */

        ofstream out_file;
        out_file.open(tempfile.c_str( ));
        if (out_file.fail( ))
        {
            cout << "New key: " <<  new_key << endl;
        }
        else
        {
            out_file << user_name << endl;
            out_file << new_key << endl;
            out_file.close( );
        }

        free(newkey);

        return true;
}

short isvalidkey(char *username, int userlen, char *serial)
{
        int *key;
        int checksum=0;
        int temp1, temp2;
        int i;

        int lastTotal;
        int startTotal;
        int licTotal;
        int todayTotal;
        int licEndDate;


        struct tm *newtime;
        time_t longtime;

        key = decryptkey(username, userlen, serial);

        for(i=0; i<10; i++)
                checksum += key[i];

        temp1 = checksum / 26;
        temp2 = checksum % 26;

        if(temp1 != serial[10]-65 || temp2 != serial[11]-65)
                //key corrupt
                return -1;

        licTotal = (key[0] * 100) + ((key[2]) * 100000);
        lastTotal = key[6] + (key[4] * 100) + ((key[8] + 2000) * 100000);
        startTotal = key[3] + (key[1] * 100) + ((key[5] + 2000) * 100000);

        time(&longtime);
        newtime = localtime(&longtime);

        todayTotal = newtime->tm_mday + ((newtime->tm_mon+1) * 100) + ((newtime->tm_year+1900) * 100000);

        //month correction
        if(key[1] + key[0] - 12 > 0)
                licEndDate = key[3] + (key[1]+key[0]-12) * 100 + (key[2]+key[5] + 2001) * 100000;
        else
                licEndDate = licTotal + startTotal;

        if(licEndDate < todayTotal)
                //license expired
                return -2;

        if(lastTotal < startTotal || lastTotal > todayTotal)
                //error with date
                return -3;

        free(key);

        return 1; //valid key

}

int* decryptkey(char *username, int userlen, char *serial)
{
        int user[20];
        int key[12];
        int *dkey;
        int t=0, i;

        dkey = (int*) malloc(sizeof(int) * 10);

        //convert to lower case
        for(i=0; i<userlen; i++)
                username[i] = tolower(username[i]);
//        _strlwr(username);

        //convert to number
        for(i=0; i<userlen; i++)
                user[i] = username[i] % 6;

        //convert to number
        for(i=0; i<12; i++)
                key[i] = serial[i] - 65;

        dkey[0] = 5;
        dkey[1] = 3;
        dkey[2] = 1;
        dkey[3] = 0;
        dkey[4] = 4;
        dkey[5] = 2;
        dkey[6] = 3;
        dkey[7] = 6;
        dkey[8] = 2;
        dkey[9] = 4;


        t = ((userlen) < (10) ? (userlen) : (10));

        //decrypt
        for(i=0; i<t; i++)
                dkey[i] ^= user[i];

        for(i=0; i<10; i++)
                dkey[i] = key[i] ^ (dkey[i] ^ key[11] % 6);

        return dkey;
}

char *encryptkey(char *username, int userlen, char *previouskey, int licMonths, int licYears)
{
        //if previous key is null, generates a new one with start time of today's date & ignores values of licMonth & licYears
        int user[20];
        int key[13];
        int *dkey;
        int t=0, i;
        char *serialkey;
        int checksum=0;

        struct tm *newtime;
        time_t longtime;

        //convert to lower case
        for(i=0; i<userlen; i++)
                username[i] = tolower(username[i]);
//        _strlwr(username);

        //convert to number
        for(i=0; i<userlen; i++)
                user[i] = username[i] % 6;

        //initialize key
        srand( (unsigned)time( NULL ) );
        
        for(i=0; i<12; i++)
                key[i] = (int)rand() % 6;

        if (previouskey != NULL)
        {
                dkey = decryptkey(username, userlen, previouskey);

        //retrieve info
                key[0] = dkey[0];
                key[2] = dkey[2];

                key[1] = dkey[1];
                key[3] = dkey[3];
                key[5] = dkey[5];
        }
        else
        {
                dkey = (int *) malloc(sizeof(int) * 10);

                key[0] = licMonths;
                key[2] = licYears;

                time(&longtime);
                newtime = localtime(&longtime);

                key[1] = newtime->tm_mon+1;
                key[3] = newtime->tm_mday;
                key[5] = newtime->tm_year-100;
        }

        time(&longtime);
        newtime = localtime(&longtime);        

        key[4] = newtime->tm_mon+1;
        key[6] = newtime->tm_mday;
        key[8] = newtime->tm_year-100;

        //calculate checksum;
        for(i=0; i<10; i++)
                checksum += key[i];

        key[10] = (int)checksum / 26;
        key[11] = checksum % 26;

        dkey[0] = 5;
        dkey[1] = 3;
        dkey[2] = 1;
        dkey[3] = 0;
        dkey[4] = 4;
        dkey[5] = 2;
        dkey[6] = 3;
        dkey[7] = 6;
        dkey[8] = 2;
        dkey[9] = 4;

        t = ((userlen) < (10) ? (userlen) : (10));

        //encrypt
        for(i=0; i<t; i++)
                dkey[i] ^= user[i];

        for(i=0; i<10; i++)
                key[i] = key[i] ^ (dkey[i] ^ key[11] % 6);

        serialkey = (char *) malloc(sizeof(char) * 13);

        //convert to char
        for(i=0; i<12; i++)
                serialkey[i] = key[i] + 65;

        serialkey[12] = '\0';

        free(dkey);

        return serialkey;
}


