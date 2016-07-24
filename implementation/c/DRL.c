/*

                        8888888b.  8888888b.  888
                        888  "Y88b 888   Y88b 888
                        888    888 888    888 888
                        888    888 888   d88P 888
                        888    888 8888888P"  888
                        888    888 888 T88b   888
                        888  .d88P 888  T88b  888
                        8888888P"  888   T88b 88888888

                        /----------------------------\
                        Data  Representation  Language
                        \----------------------------/


                   Designed and developed by Isdite Studio.

--------------------------------------------------------------------------------

  Simple, fast, hierarchical and object-oriented language for
  processing and storing data.

  C implementation (splitted, definition)

  Version One

  Written by Casper 'Falate' Fałat.

--------------------------------------------------------------------------------

  Copyright © Isdite Studio 2016.

  This software is provided 'as-is', without any express or implied
  warranty. In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
      claim that you wrote the original software. If you use this software
    in a product, an acknowledgement in the product documentation would be
    appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
      misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "DRL.h"

#ifdef __cplusplus

extern "C" {

#endif

unsigned int DRL_LAST_ERROR_CODE = 0;

static inline int _drl_ceil(float fVal) {

  int iNumber = (int) fVal;

  if (iNumber == (float)iNumber) {
      return iNumber;
  }

  if(iNumber >= 0)
    return iNumber + 1;
  else
    return iNumber - 1;

}

const unsigned int drl_getLastErrorCode() {

  return DRL_LAST_ERROR_CODE;

}

const char * drl_translateErrorCode(const unsigned int uiCode) {

  static const char * EC_LIBRARY[] = {

    "DRL_NO_ERROR",
    "DRL_FILE_OPEN_ERROR",
    "DRL_FILE_DATA_ERROR",
    "DRL_FILE_CONTEXT_ERROR"
    "DRL_FILE_WRITE_ERROR"
    "DRL_INTERNAL_MEMORY_ERROR"

  };

  if(uiCode >= DRL_ERROR_MAX)
    return "DRL_INVALID_ERR_CODE";
  else
    return EC_LIBRARY[uiCode];

}

struct DRL_FILE * drl_createFile() {

  struct DRL_FILE * lpFile = malloc(sizeof(struct DRL_FILE));

  if(!lpFile)
    goto ERROR;

  lpFile->m_lpRootObject = drl_addNewChildObject(NULL, "ROOT");

  if(!lpFile->m_lpRootObject)
    goto ERROR;

  return lpFile;

  ERROR:

  DRL_LAST_ERROR_CODE = DRL_INTERNAL_MEMORY_ERROR;

  return NULL;

}

struct DRL_FILE * drl_loadFile(const char * strPath) {

  FILE * fHandle = fopen(strPath, "r");

  if(fHandle == NULL) {

    DRL_LAST_ERROR_CODE = DRL_FILE_OPEN_ERROR;

    return NULL;

  }

  byte btaStackInputBuffer[DRL_DEFAULT_BUFFER_SIZE];

  size_t stBytesIn = 0;

  struct DRL_FILE * lpFile = drl_createFile();

  struct DRL_OBJECT * lpCurrentContext = lpFile->m_lpRootObject;

  unsigned int uiRegisterA = 0;
  unsigned int uiRegisterB = 0;
  unsigned int uiRegisterC = 0;
  unsigned int uiRegisterD = 0;

  bool bQuotes = false;
  bool bEscape = false;

  bool bIsCommentLine = false;

  byte btaTempStackBuffer[DRL_DEFAULT_BUFFER_SIZE];
  byte * lpTemporaryBuffer = btaTempStackBuffer;

  size_t stTemporaryBufferSize = DRL_DEFAULT_BUFFER_SIZE;

  while((stBytesIn = fread(btaStackInputBuffer, sizeof(byte), DRL_DEFAULT_BUFFER_SIZE, fHandle)) > 0) {

    for(size_t i = 0; i < stBytesIn; i++) {

      if(bIsCommentLine) {

        if(btaStackInputBuffer[i] == '\n')
          bIsCommentLine = false;

        continue;

      }

      if(btaStackInputBuffer[i] == ';' && !bEscape && !bQuotes)
        bIsCommentLine = true;

      if(btaStackInputBuffer[i] == '\\' && !bEscape) {

        bEscape = true;

        continue;

      }

      if(uiRegisterA == 0) {

        if(btaStackInputBuffer[i] == '@')
          uiRegisterA = 1;
        else if(btaStackInputBuffer[i] == '#')
          uiRegisterA = 2;
        else if(btaStackInputBuffer[i] == '!') {

          lpCurrentContext = lpCurrentContext->m_lpParent;

          if(lpCurrentContext == NULL) {

            DRL_LAST_ERROR_CODE = DRL_FILE_CONTEXT_ERROR;

            drl_freeFile(lpFile);

            fclose(fHandle);

            if(stTemporaryBufferSize != DRL_DEFAULT_BUFFER_SIZE)
              free(lpTemporaryBuffer);

            return NULL;

          }

        }
        else if((btaStackInputBuffer[i] != ' ' && btaStackInputBuffer[i] != '\r' && btaStackInputBuffer[i] != '\n' && btaStackInputBuffer[i] != '\t') && !bIsCommentLine) { // Junk error.

          DRL_LAST_ERROR_CODE = DRL_FILE_DATA_ERROR;

          drl_freeFile(lpFile);

          fclose(fHandle);

          if(stTemporaryBufferSize != DRL_DEFAULT_BUFFER_SIZE)
            free(lpTemporaryBuffer);

          return NULL;

        }

      }
      else {

        if(!bQuotes && btaStackInputBuffer[i] == '"' && !bEscape) {

          bQuotes = true;

          continue;

        }

        if((btaStackInputBuffer[i] == '"' && !bEscape && bQuotes) || ((btaStackInputBuffer[i] == ' ' || btaStackInputBuffer[i] == '\t' || btaStackInputBuffer[i] == '\r' || btaStackInputBuffer[i] == '\n') && !bQuotes) || (!bQuotes && bIsCommentLine)) {

          if((uiRegisterA == 1 && (uiRegisterC == 0 || uiRegisterC == uiRegisterD)) || (uiRegisterC == 0 && uiRegisterA == 2))
            continue;

          if(uiRegisterC == stTemporaryBufferSize - 1) {

            void * lpNewBuffer = malloc(stTemporaryBufferSize + DRL_DEFAULT_BUFFER_SIZE);

            memcpy(lpNewBuffer, lpTemporaryBuffer, stTemporaryBufferSize);

            if(stTemporaryBufferSize != DRL_DEFAULT_BUFFER_SIZE)
              free(lpTemporaryBuffer);

            stTemporaryBufferSize += DRL_DEFAULT_BUFFER_SIZE;

            lpTemporaryBuffer = lpNewBuffer;

          }

          lpTemporaryBuffer[uiRegisterC] = 0;
          uiRegisterC++;

          if(bQuotes)
            bQuotes = false;

          if(uiRegisterA == 1) {

            uiRegisterB++;

            if(uiRegisterB == 1) {

              uiRegisterD = uiRegisterC;

              continue;

            }

            drl_addNewAttribute(lpCurrentContext, (const char*)lpTemporaryBuffer, (const char*)lpTemporaryBuffer+uiRegisterD);

            uiRegisterB = 0;
            uiRegisterD = 0;

          }
          else
            lpCurrentContext = drl_addNewChildObject(lpCurrentContext, (const char*)lpTemporaryBuffer);

          uiRegisterA = 0;
          uiRegisterC = 0;

        }
        else {

          if(uiRegisterC == stTemporaryBufferSize - 1) {

            void * lpNewBuffer = malloc(stTemporaryBufferSize + DRL_DEFAULT_BUFFER_SIZE);

            memcpy(lpNewBuffer, lpTemporaryBuffer, stTemporaryBufferSize);

            if(stTemporaryBufferSize != DRL_DEFAULT_BUFFER_SIZE)
              free(lpTemporaryBuffer);

            stTemporaryBufferSize += DRL_DEFAULT_BUFFER_SIZE;

            lpTemporaryBuffer = lpNewBuffer;

          }

          lpTemporaryBuffer[uiRegisterC] = btaStackInputBuffer[i];
          uiRegisterC++;

          if(bEscape)
            bEscape = false;

        }
      }
    }
  }

  fclose(fHandle);

  if(stTemporaryBufferSize != DRL_DEFAULT_BUFFER_SIZE)
    free(lpTemporaryBuffer);

  return lpFile;

}

static void _drl_freeAttribute(struct DRL_ATTRIBUTE * lpAttrib) {

  free(lpAttrib->m_strName);
  free(lpAttrib->m_strValue);

  free(lpAttrib);

}

static void _drl_freeObject(struct DRL_OBJECT * lpObject) {

  free(lpObject->m_strName);

  for(int i = 0; i < lpObject->m_uiAttributeCount;i++) {

    _drl_freeAttribute(lpObject->m_lpAttributes[i]);

  }

  free(lpObject->m_lpAttributes);

  for(int i = 0; i < lpObject->m_uiChildCount; i++) {

    _drl_freeObject(lpObject->m_lpChilds[i]);

  }

  free(lpObject->m_lpChilds);

}

void drl_freeFile(struct DRL_FILE * lpFile) {

  _drl_freeObject(lpFile->m_lpRootObject);

  free(lpFile);

}

static bool _drl_writeToFileObject(FILE * lpHandle, struct DRL_OBJECT * lpObject, unsigned int uiLevel) {

  for(int i = 0; i < lpObject->m_uiAttributeCount;i++) {

    for(int x = 0; x < uiLevel; x++) {

      #ifdef DRL_PEDANTIC_ERROR_CHECK

      if(fputc('\t', lpHandle) == EOF)
        return false;

      #else

      fputc('\t', lpHandle);

      #endif

    }

    #ifdef DRL_PEDANTIC_ERROR_CHECK

    if(fputs("@ \"", lpHandle) == EOF)
      return false;

    #else

    fputs("@ \"", lpHandle);

    #endif

    for(int z = 0; z < strlen(lpObject->m_lpAttributes[i]->m_strName); z++) {

      if(lpObject->m_lpAttributes[i]->m_strName[z] == '\\' || lpObject->m_lpAttributes[i]->m_strName[z] == '\"') {

        #ifdef DRL_PEDANTIC_ERROR_CHECK

        if(fputc('\\', lpHandle) == EOF)
          return false;

        #else

        fputc('\\', lpHandle);

        #endif

      }

      #ifdef DRL_PEDANTIC_ERROR_CHECK

      if(fputc(lpObject->m_lpAttributes[i]->m_strName[z], lpHandle) == EOF)
        return false;

      #else

      fputc(lpObject->m_lpAttributes[i]->m_strName[z], lpHandle);

      #endif

    }

    #ifdef DRL_PEDANTIC_ERROR_CHECK

    if(fputs("\" \"", lpHandle) == EOF)
      return false;

    #else

    fputs("\" \"", lpHandle);

    #endif

    for(int z = 0; z < strlen(lpObject->m_lpAttributes[i]->m_strValue); z++) {

      if(lpObject->m_lpAttributes[i]->m_strValue[z] == '\\' || lpObject->m_lpAttributes[i]->m_strValue[z] == '\"') {

        #ifdef DRL_PEDANTIC_ERROR_CHECK

        if(fputc('\\', lpHandle) == EOF)
          return false;

        #else

        fputc('\\', lpHandle);

        #endif

      }

      #ifdef DRL_PEDANTIC_ERROR_CHECK

      if(fputc(lpObject->m_lpAttributes[i]->m_strValue[z], lpHandle) == EOF)
        return false;

      #else

      fputc(lpObject->m_lpAttributes[i]->m_strValue[z], lpHandle);

      #endif

    }

    #ifdef DRL_PEDANTIC_ERROR_CHECK

    if(fputs("\"\n", lpHandle) == EOF)
      return false;

    #else

    fputs("\"\n", lpHandle);

    #endif

  }

  for(int i = 0; i < lpObject->m_uiChildCount; i++) {

    for(int x = 0; x < uiLevel; x++) {

      #ifdef DRL_PEDANTIC_ERROR_CHECK

      if(fputc('\t', lpHandle) == EOF)
        return false;

      #else

      fputc('\t', lpHandle);

      #endif

    }

    #ifdef DRL_PEDANTIC_ERROR_CHECK

    if(fputs("# \"", lpHandle) == EOF)
      return false;

    #else

    fputs("# \"", lpHandle);

    #endif

    for(int z = 0; z < strlen(lpObject->m_lpChilds[i]->m_strName); z++) {

      if(lpObject->m_lpChilds[i]->m_strName[z] == '\\' || lpObject->m_lpChilds[i]->m_strName[z] == '\"') {

        #ifdef DRL_PEDANTIC_ERROR_CHECK

        if(fputc('\\', lpHandle) == EOF)
          return false;

        #else

        fputc('\\', lpHandle);

        #endif

      }

      #ifdef DRL_PEDANTIC_ERROR_CHECK

      if(fputc(lpObject->m_lpChilds[i]->m_strName[z], lpHandle) == EOF)
        return false;

      #else

      fputc(lpObject->m_lpChilds[i]->m_strName[z], lpHandle);

      #endif

    }

    #ifdef DRL_PEDANTIC_ERROR_CHECK

    if(fputs("\"\n", lpHandle) == EOF)
      return false;

    #else

    fputs("\"\n", lpHandle);

    #endif

    if(!_drl_writeToFileObject(lpHandle, lpObject->m_lpChilds[i], uiLevel+1))
      return false;

    for(int x = 0; x < uiLevel; x++) {

      #ifdef DRL_PEDANTIC_ERROR_CHECK

      if(fputc('\t', lpHandle) == EOF)
        return false;

      #else

      fputc('\t', lpHandle);

      #endif

    }

    #ifdef DRL_PEDANTIC_ERROR_CHECK

    if(fputc('!', lpHandle) == EOF)
      return false;

    #else

    fputc('!', lpHandle);

    #endif

  }

  if(lpObject->m_uiChildCount != 0) {

    #ifdef DRL_PEDANTIC_ERROR_CHECK

    if(fputc('\n', lpHandle) == EOF)
      return false;

    #else

    fputc('\n', lpHandle);

    #endif

  }

  return true;

}

bool drl_saveFile(struct DRL_FILE * lpFile, const char * strPath) {

  FILE * lpHandle = fopen(strPath, "w");

  if(lpHandle == NULL) {

    DRL_LAST_ERROR_CODE = DRL_FILE_OPEN_ERROR;

    return false;
  }

  struct DRL_OBJECT * lpRoot = lpFile->m_lpRootObject;

  if(!_drl_writeToFileObject(lpHandle, lpRoot, 0)) {

    fclose(lpHandle);

    DRL_LAST_ERROR_CODE = DRL_FILE_WRITE_ERROR;

    return false;

  }

  fclose(lpHandle);

  return true;

}

 void drl_addAttribute(struct DRL_OBJECT * lpDest, struct DRL_ATTRIBUTE * lpAttr)
 {

   if(lpDest->m_uiAttributeCount != 0 && lpDest->m_uiAttributeCount % DRL_POINTER_POOL_BASE == 0)
     lpDest->m_lpAttributes = realloc(lpDest->m_lpAttributes, (_drl_ceil((float)lpDest->m_uiAttributeCount / (float)DRL_POINTER_POOL_BASE) + 1) * DRL_POINTER_POOL_BASE);

   lpDest->m_lpAttributes[lpDest->m_uiAttributeCount] = lpAttr;
   lpDest->m_uiAttributeCount++;

 }

struct DRL_ATTRIBUTE * drl_addNewAttribute(struct DRL_OBJECT * lpDest, const char * strName, const char * strValue) {

  struct DRL_ATTRIBUTE * lpAttrib = malloc(sizeof(struct DRL_ATTRIBUTE));

  size_t stNameLength = strlen(strName) + 1;
  size_t stValueLength = strlen(strValue) + 1;

  lpAttrib->m_strName = malloc(stNameLength);
  memcpy(lpAttrib->m_strName, strName, stNameLength);

  lpAttrib->m_strValue = malloc(stValueLength);
  memcpy(lpAttrib->m_strValue, strValue, stValueLength);

  if(lpDest != NULL)
    drl_addAttribute(lpDest, lpAttrib);

  return lpAttrib;

}

void drl_addChildObject(struct DRL_OBJECT * lpDest, struct DRL_OBJECT * lpChild) {

  lpChild->m_lpParent = lpDest;

  if(lpDest->m_uiChildCount % DRL_POINTER_POOL_BASE == 0 && lpDest->m_uiChildCount != 0)
    lpDest->m_lpChilds = realloc(lpDest->m_lpChilds, (_drl_ceil((float)lpDest->m_uiChildCount / (float)DRL_POINTER_POOL_BASE) + 1) * DRL_POINTER_POOL_BASE);

  lpDest->m_lpChilds[lpDest->m_uiChildCount] = lpChild;
  lpDest->m_uiChildCount++;

}

struct DRL_OBJECT * drl_addNewChildObject(struct DRL_OBJECT * lpDest, const char * strName) {

  struct DRL_OBJECT * lpObject = malloc(sizeof(struct DRL_OBJECT));

  size_t stNameLength = strlen(strName) + 1;

  lpObject->m_strName = malloc(stNameLength);
  memcpy(lpObject->m_strName, strName, stNameLength);

  lpObject->m_uiAttributeCount = 0;
  lpObject->m_lpAttributes = malloc(sizeof(struct DRL_ATTRIBUTE*) * DRL_POINTER_POOL_BASE);

  lpObject->m_uiChildCount = 0;
  lpObject->m_lpChilds = malloc(sizeof(struct DRL_OBJECT*) * DRL_POINTER_POOL_BASE);

  if(lpDest != NULL)
    drl_addChildObject(lpDest, lpObject);
  else
    lpObject->m_lpParent = NULL;

  return lpObject;

}

void drl_removeAttribute(struct DRL_OBJECT * lpObject, size_t stIndex) {

  #ifdef DRL_PEDANTIC_ERROR_CHECK

  if(stIndex >= lpObject->m_uiAttributeCount)
    return;

  #endif

  size_t stDiff = lpObject->m_uiAttributeCount - stIndex - 1;

  _drl_freeAttribute(lpObject->m_lpAttributes[stIndex]);

  for(int i = 0; i < stDiff;i++)
    lpObject->m_lpAttributes[stIndex + i] = lpObject->m_lpAttributes[stIndex + i + 1];

  lpObject->m_uiAttributeCount--;

  if(lpObject->m_uiAttributeCount != 0 && lpObject->m_uiAttributeCount % DRL_POINTER_POOL_BASE == 0)
    lpObject->m_lpAttributes = realloc(lpObject->m_lpAttributes, lpObject->m_uiAttributeCount / DRL_POINTER_POOL_BASE);

}

void drl_removeObject(struct DRL_OBJECT * lpObject, size_t stIndex) {

  #ifdef DRL_PEDANTIC_ERROR_CHECK

  if(stIndex >= lpObject->m_uiChildCount)
    return;

  #endif

  size_t stDiff = lpObject->m_uiChildCount - stIndex - 1;

  _drl_freeObject(lpObject->m_lpChilds[stIndex]);

  for(int i = 0; i < stDiff;i++)
    lpObject->m_lpChilds[stIndex + i] = lpObject->m_lpChilds[stIndex + i + 1];

  lpObject->m_uiChildCount--;

  if(lpObject->m_uiChildCount != 0 && lpObject->m_uiChildCount % DRL_POINTER_POOL_BASE == 0)
    lpObject->m_lpChilds = realloc(lpObject->m_lpChilds, lpObject->m_uiAttributeCount / DRL_POINTER_POOL_BASE);

}

static inline int _drl_rapidATOI(const char * strValue)
{

    int iValue = 0;

    bool bMod = false;

    if(*strValue == '-') {

      strValue++;

      bMod = true;

    }

    while(*strValue)
      iValue = iValue * 10 + (*strValue++ - '0');

    if(bMod)
      iValue = -iValue;

    return iValue;

}

int drl_attributeToInt(struct DRL_ATTRIBUTE * lpAttribute) {

  return _drl_rapidATOI(lpAttribute->m_strValue);

}

static int _drl_rapidIPOW(int iBase, int iExp)
{
    int iResult = 1;

    while (iExp)
    {

        if (iExp & 1)
            iResult *= iBase;

        iExp >>= 1;

        iBase *= iBase;
    }

    return iResult;
}

static double _drl_rapidATOD(const char * strValue) {

    double dblValue = 0.0f;

    bool bMod = false;

    if (*strValue == '-') {

      bMod = true;
      strValue++;

    }

    while (*strValue >= '0' && *strValue <= '9')
        dblValue = (dblValue * 10.0f) + (*strValue++ - '0');

    if (*strValue == '.') {

        double dblTemp = 0.0f;
        unsigned int iCounter = 0;

        ++strValue;

        while (*strValue >= '0' && *strValue <= '9') {

            dblTemp = (dblTemp * 10.0f) + (*strValue++ - '0');
            ++iCounter;

        }

        dblValue += dblTemp / (double)_drl_rapidIPOW(10, iCounter);

    }

    if(bMod)
      dblValue = -dblValue;

    return dblValue;

}

double drl_attributeToDouble(struct DRL_ATTRIBUTE * lpAttribute) {

  return _drl_rapidATOD(lpAttribute->m_strValue);

}

#ifdef __cplusplus

}

#endif
