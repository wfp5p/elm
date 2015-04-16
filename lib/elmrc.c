
/*******************************************************************************
 *  The Elm Mail System
 *
 *                      Copyright (c) 2000 USENET Community Trust
 *******************************************************************************
 * Bug reports, patches, comments, suggestions should be sent to:
 *
 *      Bill Pemberton, Elm Coordinator
 *      flash@virginia.edu
 *
 *******************************************************************************
 *
 ******************************************************************************/

/*
 * getelmrcName() returns the name of the elmrc file
 *
 *
 */

#include "elm_defs.h"

char *CmdLineElmrc = NULL;

void getelmrcName(char *filename,int len)
{

   if (CmdLineElmrc)
   {
      strncpy(filename, CmdLineElmrc, len);
   }
   else
   {
      /* we ought to use len here, but we don't know snprintf is available */
      sprintf(filename, "%s/%s", user_home, elmrcfile);
   }
}

void setelmrcName(char *name)
{
   if (name != NULL)
   {
      CmdLineElmrc = safe_strdup(name);
   }
}
