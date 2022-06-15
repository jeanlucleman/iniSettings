#ifndef INISETTINGS_H
#define INISETTINGS_H
  #include <Arduino.h>
  #include <spi.h>
  #include "FS.h"
  #include <SD.h>
  
#define BUFFER_LEN 80
class IniSettingsState;
class IniSettings
  {       
    private:
      // Flag to check if the Settings object has been properly initialized. The full initialization cannot be done in the class constructor.
      // The reason is that the instantiation is made before the main setup is started and this creates a program crash...
      // bool _ini;
      const char *_fileName;
      const char *_fileTempName;
      File _fileSettings;
      File _fileTemp;
      enum error_IniSettings 
        {
          errorNoError = 0,
          errorFileNotFound,
          errorFileNotOpen,
          errorBufferTooSmall,
          errorUnknownError,
        };  
      error_IniSettings _error;

      bool _getValue(const char * section, const char * key, char * value, IniSettingsState &state);
      bool _findKey(const char * section, const char * key, char * value,IniSettingsState &state);
      bool _findSection(const char * section, const char * key, char * value,IniSettingsState &state);
      bool _saveLine(char * startLine);
      bool _openTempFile();
      bool _readNextLine(char * value);
      void _rightTrim(char* str);
      void _leftTrim(char * str);
      bool _checkKey(char *bufferRead, const char *key, const char *value);

    public:          
      IniSettings(const char *fileName); 
      bool begin();
      char * getValue(const char * section, const char * key, char * value);
      int getValueInt(const char * section, const char * key);
      bool saveSettings(const char * section, const char * key, const char * value); 
  };

class IniSettingsState 
  {
    public:
      IniSettingsState();
    private:
      enum state_enm
        {
          starting = 0,
          sectionFound,
          keyFound,
          valueRetrieved
        };
      enum error_IniSettingsState 
        {
          errorNoError = 0,
          errorSeekError,
          errorSectionNotFound,
          errorKeyNotFound,
          errorEndOfFile,
          errorUnknownError,
        };
      uint8_t error_getValue;
      uint32_t readLinePosition;
      uint8_t stateValue;
      friend class IniSettings;
    };
#endif 