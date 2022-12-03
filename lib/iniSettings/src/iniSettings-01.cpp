/*
The following is derived of iniFile from steve Marple (IniFile@^1.3.0), adding the function to write / modify section/key/value.

*/
#define DEBUG false
#define DEBUGSAVE false

#include "iniSettings.h"
IniSettings::IniSettings(const char* fileName) // Constructeur IniSettings
  { 
    //_ini=false; // this private boolean will be set to true when ini() function is done
    _fileName= fileName; // settings ini filename
    _fileTempName = "/temp.ini"; // temporary file to write modified settings
    _error=error_IniSettings::errorNoError;

  }
bool IniSettings::begin() // Initialize the settings file in read mode
  {
      String listFiles;
      if (!LittleFS.begin())
      {
        Serial.println("An Error has occurred while mounting file system");
        return "";
      }
      String sep="$$";
      Dir root = LittleFS.openDir("/");
      while (root.next()) 
        {
          listFiles += String(root.fileName()) + sep;
          if(root.fileSize()) 
            {
              File f = root.openFile("r");
              Serial.print("Taille: ");
              Serial.println(f.size());
            }
        }
      Serial.println(listFiles);
    Serial.println(String(_fileName));
    if (_fileSettings)
      {
        _fileSettings.close();
      }
    #ifdef SDCARD
      _fileSettings= SD.open(_fileName, FILE_READ); 
    #else
       _fileSettings= LittleFS.open(String(_fileName), "r"); 
    #endif
   if(_fileSettings) {return true;}else{return false;}
  }
bool IniSettings::_openTempFile() // Initialize the temporary file in write mode
  {
    if (_fileTemp)
      {
        _fileTemp.close();
      }
// File::dateTimeCallback(dateTime);
    #ifdef SDCARD
      _fileTemp = SD.open(_fileTempName, FILE_WRITE); 
    #else
      _fileTemp = LittleFS.open(String(_fileTempName), "w"); 
    #endif
    return _fileTemp;
  }
int IniSettings::getValueInt(const char * section, const char * key)
  {
    char value[BUFFER_LEN];
    return atoi(getValue(section, key, value));
  }
float IniSettings::getValueFloat(const char * section, const char * key)
  {
    char value[BUFFER_LEN];
    return atof(getValue(section, key, value));
  }
String IniSettings::getValueString(const char * section, const char * key)
  {
    char value[BUFFER_LEN];
    return String(getValue(section, key, value));
    
  }
char * IniSettings::getValue(const char * section, const char * key, char * value)
  {
    IniSettingsState state;
    _getValue(section, key, value, state);
    return value;
  }



bool IniSettings::_getValue(const char * section, const char * key, char * value, IniSettingsState &state)
  {
    bool done=false;
    switch (state.stateValue)
    {
      case IniSettingsState::starting:
        _fileSettings.seek(0);
        if(_findSection(section, key, value, state))
          {
            state.stateValue=IniSettingsState::sectionFound;
            done=true;
          }
        else
          {
            Serial.println("_findSection=false");
            *value='\0';
          }
        break;
      case IniSettingsState::sectionFound:
        if(_findKey(section, key, value, state))
          {
            state.stateValue=IniSettingsState::keyFound;
            done=true;
          }
        else
          {
            Serial.println("_findKey=false");
            *value='\0';
          }
        break;
      case IniSettingsState::keyFound:
        state.stateValue=IniSettingsState::valueRetrieved;
        break;
    }
    if (done && state.stateValue!=IniSettingsState::valueRetrieved)
      {
         _getValue(section, key, value, state);
      }
    return done;
  }
bool IniSettings::_findKey(const char * section, const char * key, char * cp,IniSettingsState &state)
  {
    bool found = false;
    while (!found)
      {
        if (!_readNextLine(cp)) // Section not found reaching end of file
          {
            Serial.println("Section not found reaching end of file");
            break;
          } 
        _leftTrim(cp);
        if(*cp=='[') // New section, key not found in the previous section   
          {
            state.error_getValue =IniSettingsState::errorKeyNotFound;
            Serial.println("New section, key not found in the previous section");
            break;
          }   
        if(*cp!=';' && *cp!='#') // We are not on a commented line
          {
            char *ptrEqual = strchr(cp, '='); // Searching if there is the = in this line
            if(ptrEqual!=NULL)
              {
                *ptrEqual='\0'; // cp will point on the key string
                _rightTrim(cp); // removing space if any on the right side of key
                if(strcmp(cp,key)==0) // Comparing if the key found equals the key searched
                  {
                    found=true;
                    *ptrEqual++; // Now this pointer points on the start of value
                    char *ptrComment = strchr(ptrEqual,';');
                    if(ptrComment!=NULL)
                      {
                        *ptrComment='\0'; // The comment are left out when ready a value
                      }                    
                    _leftTrim(ptrEqual);// removing left space if any
                    _rightTrim(ptrEqual);// removing right space if any
                    while(*ptrEqual!='\0')
                      {
                        *cp=*ptrEqual; // The key value is shifted left to start on the cp pointer
                        *cp++;
                        *ptrEqual++;
                      }
                    *cp='\0'; // ending the value string 
                    // _rightTrim(cp); 
                    // _leftTrim(cp); 

                  }
              }
          }
      }
    return found;
  }
bool IniSettings::_findSection(const char * section, const char * key, char * cp,IniSettingsState &state)
  {
    bool found=false;
    while (!found)
      {
        if (!_readNextLine(cp)){break;} // Section not found reaching end of file
        #if DEBUG
          Serial.printf("116: %S\n", cp);
        #endif
        _saveLine(cp);
        _leftTrim(cp);
        if(*cp=='[') // One section found
          {
            ++cp;
            _leftTrim(cp);
            char *ep = strchr(cp, ']');
            if (ep != NULL) 
              {
                *ep = '\0'; // make ] be end of string
                _rightTrim(cp);// removeTrailingWhiteSpace(cp);
              }
             if (strcmp(cp, section) == 0) 
              {
                found=true;
                break;
              }
          }
      }
      if(!found)
        {
          state.error_getValue =IniSettingsState::errorSectionNotFound;
        }
    return found;
  }
bool IniSettings::_saveLine(char* bufferWrite)
  {
    _fileTemp.printf("%s\n",bufferWrite);
    #if DEBUG
        Serial.printf("-> %s\n",bufferWrite);
    #endif
    return true;
  }
bool IniSettings::saveSettings(const char * section, const char * key, const char * value)
  {
    char bufferRead[BUFFER_LEN];
    char bufferWrite[BUFFER_LEN];
    char * ptrWrite = bufferWrite;
    bool done =false; // becomes true when all the settings file has been copied with the key/value parameters modified of added
    bool keySaved = false; // becomes true when the key/value parameter has been written
    bool sectionFound=false; // become true when the section parameter has been found in the settings file
    _openTempFile(); // Open a temp file for writing mode, making a copy of the settings file with the modification from parameters
    _fileSettings.seek(0); // set the file pointer at the beginning
    #if DEBUGSAVE
      Serial.println("\n===========================================");
      Serial.printf("\nEntering in saveSettings saving section = %s, key = %s, value = %s",section, key,value);
    #endif  
    while (!done)  
      {
        if(_readNextLine(bufferRead)) // will be false at end of file
          {
            #if DEBUGSAVE
              Serial.printf("\nWe read a new line %s\n", bufferRead);
            #endif            
            if(keySaved) // As the key/value parameter have been written, now we just copy the rest in the temp file
              {
                #if DEBUGSAVE
                  Serial.println("As the key/value parameter have been written, now we just copy the rest in the temp file");
                #endif
                _saveLine(bufferRead);
              }
            else
              {
                char * firstChar=bufferRead;
                while(*firstChar ==' ') // to set the firstChar pointer after the left space
                  {
                    firstChar++;
                  }
                switch (*firstChar)
                  {
                    case '#': // This is a comment line
                    case ';':
                      #if DEBUGSAVE
                        Serial.println("This is a comment line");
                      #endif
                      _saveLine(bufferRead);
                      break;
                    case '[': // This is a section line
                      { // Bracket is needed because there is a declaration inside this case (char * ep)
                        if (sectionFound) // We are reading a line starting a new section while the correct section has just been read
                          {  // so we first save the key/value at the end of the previous section
                            while (*key!='\0')
                              {
                                *ptrWrite++=*key++; // First we copy the key in the bufferWrite using the ptrWrite pointer
                              }
                            *ptrWrite++='='; // Then we add the = character
                            while (*value!='\0')
                              {
                                *ptrWrite++=*value++; // and finally we copy the value
                              }
                            *ptrWrite='\0'; // and we close the new string
                            #if DEBUGSAVE
                              Serial.println("The section was found before, we are on a new section line so we copy the key=value line");
                            #endif
                            _saveLine(bufferWrite);
                            keySaved=true; // As the key/value is saved, now we just have to copy the rest of the file
                            // sectionFound=true; // useless?
                          }
                        #if DEBUGSAVE
                          Serial.println("We write the section line just read");
                        #endif
                        _saveLine(bufferRead); // We save the section line, not modified
                        if (!keySaved) // as the key has not yet been saved we must check if this is the section in parameter
                          {
                            ++firstChar;
                            _leftTrim(firstChar);
                            char *ep = strchr(firstChar, ']');
                            if (ep != NULL) 
                              {
                                *ep = '\0'; // make ] be end of string
                                _rightTrim(firstChar);// removeTrailingWhiteSpace(cp);
                              }
                            if (strcmp(firstChar, section) == 0)  // and we check if we are on the section in parameter 
                              {
                                sectionFound=true;
                              }
                            
                          }
                        break;
                      }
                    default: // The line read is not a comment line and not a section line
                      #if DEBUGSAVE
                        Serial.printf("La ligne lue n'est ni commentaire si section line: %s\n",bufferRead);
                      #endif
                      if (sectionFound && _checkKey(bufferRead, key,value)) //On est dans la bonne section et la ligne lue contient la bonne clé
                        {
                      #if DEBUGSAVE
                        Serial.printf("On est dans la bonne section et la ligne lue contient la bonne clé: %s\n",bufferRead);
                      #endif
                          ptrWrite=bufferWrite;
                          while (*key!='\0')
                            {
                              *ptrWrite++=*key++; // the key is copied in the bufferWrite
                            }
                          *ptrWrite++='='; // Then  we add the '='
                          while (*value!='\0')
                            {
                              *ptrWrite++=*value++; // and we write the value
                            }

                          firstChar=bufferRead;
                          firstChar = strchr(firstChar, '=');
                          firstChar = strchr(firstChar, ';');
                          
                          if (firstChar != NULL) 
                            {
                              *firstChar++;
                              *ptrWrite++=';';
                              while (*firstChar!='\0')
                                {
                                  *ptrWrite++=*firstChar++; // the comment is copied in the bufferWrite
                                }
                            }


                          *ptrWrite='\0';
                          #if DEBUGSAVE
                            Serial.printf("Ecriture de la ligne avec la nouvelle valeur: %S\n", bufferWrite);
                          #endif
                          _saveLine(bufferWrite);
                          sectionFound=false; 
                          keySaved=true;
                        }
                      else
                        {
                          #if DEBUGSAVE
                            Serial.printf("The line read is copied as it is because we are not in the right section or it is not the right key: %S\n", bufferRead);
                          #endif
                          _saveLine(bufferRead); // The line read is copied as it is because we are not in the right section or it is not the right key
                        }
                  }
              }
          }
        else // we are at end of file and this is not done!
          {
            if (!keySaved) // We are at end of file and the section in parameter has not been found yet
              {
                *ptrWrite++='[';// Creating the new section to be added at the end of file
                while (*section!='\0')
                  {
                    *ptrWrite++=*section++;
                  }
                *ptrWrite++=']';
                *ptrWrite='\0';   
                #if DEBUGSAVE
                  Serial.printf("Being at end of file with no section found, a new section line is added: %S\n", bufferWrite);
                #endif
                _saveLine(bufferWrite); // saving the new section line ex. [Wifi]
                // Writing the new key=value 
                ptrWrite=bufferWrite;
                while (*key!='\0') // Creating the line with the new key=value
                  {
                    *ptrWrite++=*key++;
                  }
                *ptrWrite++='=';
                while (*value!='\0')
                  {
                    *ptrWrite++=*value++;
                  }
                *ptrWrite='\0';
                #if DEBUGSAVE
                  Serial.printf("And the key value is added: %S\n", bufferWrite);
                #endif
                _saveLine(bufferWrite); // saving the new key=value line

                keySaved=true;
              }
            done=true;
          }
      }
    _fileTemp.flush();
    _fileTemp.close();
    _fileSettings.close();
    // begin();

    const char * path1 ="/temp.ini";
    const char * path2 ="/settings.ini";
    // char path3[] = "/*.ini"; 
    // path3[1]=count+64;//  (char *) count;
    // Serial.println("Effacement du fichier");
    #ifdef SDCARD
      SD.remove(path2);
      SDFS.rename(path1,path2);
    #else
      LittleFS.remove(String(path2));
      LittleFS.rename(String(path1),String(path2));
    #endif


    // SD.rename(path2,path3);
    // SD.rename(path1, path2);
    begin();
    return done;
  }
bool IniSettings::saveSettings(const char * section, const char * key, int value)
  {
    char sValue[16] ;
    sprintf(sValue, "%d", value);
    return saveSettings(section, key, sValue);
  }
bool IniSettings::saveSettings(const char * section, const char * key, unsigned long value)
  {
    char sValue[16] ;
    sprintf(sValue, "%lu", value);
    return saveSettings(section, key, sValue);
  }
bool IniSettings::_checkKey(char *bufferRead, const char *key, const char *value)
  {
    char temp[BUFFER_LEN];
    char * cp = temp;  
    strcpy(cp,bufferRead);
   // créer une copie de bufferRead qui ne doit pas être modifié
    bool found = false;
    while(*cp ==' ') // to set the cp pointer after the left space
      {
        cp++;
      }
    char *ptrEqual = strchr(cp, '='); // Searching if there is the = in this line
    if(ptrEqual!=NULL)
      {
        *ptrEqual='\0'; // cp will point on the key string
        _rightTrim(cp); // removing space if any on the right side of key
        #if DEBUG
          Serial.printf("Comparaison de cp (%s) et key (%s)\n",cp,key);
        #endif
        if(strcmp(cp,key)==0) // Comparing if the key found equals the key searched
          {
            found=true;
          }
      }
    return found;
  }
bool IniSettings::_readNextLine(char * value)
  {
      if(!_fileSettings.available()){return false;} //  bytesRead==0){return false;}
      char terminator='\n';
      int len=BUFFER_LEN;
      size_t bytesRead;
      bytesRead=_fileSettings.readBytesUntil(terminator, value,len);

      value[bytesRead]='\0';
      #if DEBUG
        Serial.printf("bytesRead=%d\n",bytesRead);
        Serial.printf("392: %s\n",value);
        Serial.printf("Position = %d\n",_fileSettings.position());
      #endif
      _fileSettings.seek(_fileSettings.position());
    return true;
  }
void IniSettings::_leftTrim(char * str)
  {
    char * cp=str;
    while(*cp ==' ') // to set the cp pointer after the left space
      {
        cp++;
      }
    while (*cp != '\0') // Until reaching end of string
      {
        *str=*cp; // shifting string on left
        str++;
        cp++;
      }
    *str='\0';
  }
void IniSettings::_rightTrim(char* str)
{
	if (str == nullptr)
		return;
	char *cp = str + strlen(str) - 1;
	while (cp >= str && isspace(*cp))
		*cp-- = '\0';
}
IniSettingsState::IniSettingsState()
  {
    readLinePosition = 0;
    stateValue= starting;
    error_getValue=error_IniSettingsState::errorNoError;
    
  }
