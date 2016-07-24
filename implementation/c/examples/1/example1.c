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

  Example 1 (c language) - loading, printing and saving data.

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

#include <DRL.h> // DRL API.
#include <stdio.h> // printf.

void printDRLObjectTree(struct DRL_OBJECT * lpObject, unsigned int uiLevel) {

  // Print spaces to show item position in tree.
  for(unsigned int i = 0; i < uiLevel; i++)
    printf(" ");

  printf("Object : %s\n", lpObject->m_strName);

  uiLevel++; // Increase amount of spaces by one.

  for(unsigned int i = 0; i < lpObject->m_uiAttributeCount; i++) { // Print object's attributes.

    // Print spaces to show item position in tree.
    for(unsigned int i = 0; i < uiLevel; i++)
      printf(" ");

    printf("Attribute : \"%s\" value \"%s\"\n", lpObject->m_lpAttributes[i]->m_strName, lpObject->m_lpAttributes[i]->m_strValue);

  }

  // Do the same for subobjects.
  for(unsigned int i = 0; i < lpObject->m_uiChildCount;i++)
    printDRLObjectTree(lpObject->m_lpChilds[i], uiLevel);

}

int main() {

  // Try to load data file into memory.
  struct DRL_FILE * lpFile = drl_loadFile("data.drl");

  if(lpFile == NULL) { // On failure show message and exit.

    printf("drl_loadFile failed, error : %s\n",
            drl_translateErrorCode(drl_getLastErrorCode()));

    return -1;

  }

  // Print file data to console.
  printDRLObjectTree(lpFile->m_lpRootObject, 0);

  if(!drl_saveFile(lpFile, "copy.drl")) {

    printf("drl_saveFile failed, error : %s\n",
            drl_translateErrorCode(drl_getLastErrorCode()));

    drl_freeFile(lpFile);

    return -2;

  }

  // Free program memory.
  drl_freeFile(lpFile);

  return 0;

}
