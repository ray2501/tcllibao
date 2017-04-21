/*
 * tcllibao.c
 *
 *      Copyright (C) Danilo Chang - December 2016, 2017
 *
 ********************************************************************/

/*
 * For C++ compilers, use extern "C"
 */
#ifdef __cplusplus
extern "C" {
#endif

#include <tcl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ao/ao.h>

/*
 * Only the _Init function is exported.
 */

extern DLLEXPORT int    Libao_Init(Tcl_Interp * interp);

/*
 * end block for C++
 */

#ifdef __cplusplus
}
#endif

ao_device *device = NULL;
int initialize = 0;
TCL_DECLARE_MUTEX(myMutex);


static int Libao_Cmd(ClientData arg, Tcl_Interp * interp, int objc, Tcl_Obj * CONST objv[])
{
  int cmd;

  enum {
    CMD_INITIALIZE, CMD_DEFAULT, CMD_DRIVER, CMD_OPEN_LIVE, CMD_OPEN_FILE,
    CMD_PLAY, CMD_CLOSE, CMD_SHUTDOWN
  };

  static CONST char *sCmd[] = {
    "initialize", "default_id", "driver_id", "open_live", "open_file",
    "play", "close", "shutdown",
    0
  };

  if (objc < 2) {
    Tcl_WrongNumArgs(interp, 1, objv, "cmd args");
    return TCL_ERROR;
  }

  if (Tcl_GetIndexFromObj(interp, objv[1], sCmd, "command", TCL_EXACT, (int *) &cmd) != TCL_OK) {
    return TCL_ERROR;
  }

  switch (cmd) {
    case CMD_INITIALIZE: {
      if( objc != 2){
        Tcl_WrongNumArgs(interp, 2, objv, 0);
        return TCL_ERROR;
      }

      Tcl_MutexLock(&myMutex);
      initialize = 1;
      Tcl_MutexUnlock(&myMutex);

      ao_initialize();
      break;
    }

    case CMD_DEFAULT: {
      Tcl_Obj *return_obj;
      int result;

      if( objc != 2){
        Tcl_WrongNumArgs(interp, 2, objv, 0);
        return TCL_ERROR;
      }

      if(initialize==0) {
        Tcl_AppendResult(interp, "Please execute initialize method first!", (char*)0);
        return TCL_ERROR;
      }
 
      result = ao_default_driver_id();
      if(result == -1) {
          Tcl_AppendResult(interp, "No default device is available", (char*)0);
          return TCL_ERROR;
      }

      return_obj = Tcl_NewIntObj(result);
      Tcl_SetObjResult(interp, return_obj);
      break;
    }

    case CMD_DRIVER: {
     /*
      * Check https://www.xiph.org/ao/doc/drivers.html
      */
      Tcl_Obj *return_obj;
      char *short_name = NULL;
      int len;
      int result;

      if( objc != 3){
        Tcl_WrongNumArgs(interp, 2, objv, "short_name");
        return TCL_ERROR;
      }

      if(initialize==0) {
        Tcl_AppendResult(interp, "Please execute initialize method first!", (char*)0);
        return TCL_ERROR;
      }

      short_name = Tcl_GetStringFromObj(objv[2], &len);
      if( !short_name || len < 1 ) {
          return TCL_ERROR;
      }

      result = ao_driver_id(short_name);
      if(result == -1) {
          Tcl_AppendResult(interp, "No device is available", (char*)0);
          return TCL_ERROR;
      }

      return_obj = Tcl_NewIntObj(result);
      Tcl_SetObjResult(interp, return_obj);
      break;
    }

    case CMD_OPEN_LIVE: {
      ao_sample_format format;
      const char *zArg;
      int driver_id = 0;
      int i;
      int bits = 16;
      int rate = 44100;
      int channels = 2;
      int byteformat = 4;

      if( objc < 3 || (objc&1)!=1){
        Tcl_WrongNumArgs(interp, 2, objv, "id ?-bits bits? ?-rate samplerate? ?-channels channels? ?-byteformat format? ");
        return TCL_ERROR;
      }

      if(initialize==0) {
        Tcl_AppendResult(interp, "Please execute initialize method first!", (char*)0);
        return TCL_ERROR;
      }

      if(Tcl_GetIntFromObj(interp, objv[2], &driver_id) != TCL_OK) {
            return TCL_ERROR;
      }

      if(driver_id < 0) {
        Tcl_AppendResult(interp, "Error: id needs >= 0", (char*)0);
        return TCL_ERROR;
      }

      for(i=3; i+1<objc; i+=2){
         zArg = Tcl_GetStringFromObj(objv[i], 0);

         if( strcmp(zArg, "-bits")==0 ){
            if(Tcl_GetIntFromObj(interp, objv[i+1], &bits) != TCL_OK) {
                return TCL_ERROR;
            }

            if(bits <= 0) {
               Tcl_AppendResult(interp, "Error: bits needs > 0", (char*)0);
               return TCL_ERROR;
            }
         } else if( strcmp(zArg, "-rate")==0 ){
            if(Tcl_GetIntFromObj(interp, objv[i+1], &rate) != TCL_OK) {
                return TCL_ERROR;
            }

            if(rate <= 0) {
               Tcl_AppendResult(interp, "Error: rate needs > 0", (char*)0);
               return TCL_ERROR;
            }
         } else if( strcmp(zArg, "-channels")==0 ){
            if(Tcl_GetIntFromObj(interp, objv[i+1], &channels) != TCL_OK) {
                return TCL_ERROR;
            }

            if(channels <= 0) {
               Tcl_AppendResult(interp, "Error: channels needs > 0", (char*)0);
               return TCL_ERROR;
            }
         } else if( strcmp(zArg, "-byteformat")==0 ){
            if(Tcl_GetIntFromObj(interp, objv[i+1], &byteformat) != TCL_OK) {
                return TCL_ERROR;
            }

            //#define AO_FMT_LITTLE 1
            //#define AO_FMT_BIG    2
            //#define AO_FMT_NATIVE 4
            if(byteformat <= 0 || byteformat > 4) {
               Tcl_AppendResult(interp, "Error: byteformat out of range", (char*)0);
               return TCL_ERROR;
            }
         }else{
            Tcl_AppendResult(interp, "unknown option: ", zArg, (char*)0);
            return TCL_ERROR;
         }
      }

      format.bits = bits;
      format.rate = rate;
      format.channels = channels;
      format.byte_format = byteformat;
      format.matrix = 0;

      device = ao_open_live(driver_id, &format, NULL);
      if(!device) {
          Tcl_AppendResult(interp, "open_live fail!", (char*)0);
          return TCL_ERROR;
      }

      break;
    }

    case CMD_OPEN_FILE: {
      ao_sample_format format;
      const char *zArg;
      int driver_id = 0;
      int i;
      int bits = 16;
      int rate = 44100;
      int channels = 2;
      int byteformat = 4;
      char *filename;
      Tcl_DString translatedFilename;
      int len;
      int overwrite = 0;

      if( objc < 4 || (objc&1)!=0){
        Tcl_WrongNumArgs(interp, 2, objv, "id filename ?-overwrite overwrite? ?-bits bits? ?-rate samplerate? ?-channels channels? ?-byteformat format? ");
        return TCL_ERROR;
      }

      if(initialize==0) {
        Tcl_AppendResult(interp, "Please execute initialize method first!", (char*)0);
        return TCL_ERROR;
      }

      if(Tcl_GetIntFromObj(interp, objv[2], &driver_id) != TCL_OK) {
            return TCL_ERROR;
      }

      if(driver_id < 0) {
        Tcl_AppendResult(interp, "Error: id needs >= 0", (char*)0);
        return TCL_ERROR;
      }
      
      filename = Tcl_GetStringFromObj(objv[3], &len);
      if( !filename || len < 1 ){
          return TCL_ERROR;
      }

      for(i=4; i+1<objc; i+=2){
         zArg = Tcl_GetStringFromObj(objv[i], 0);

         if( strcmp(zArg, "-overwrite")==0 ){
            if(Tcl_GetBooleanFromObj(interp, objv[i+1], &overwrite) != TCL_OK) {
                return TCL_ERROR;
            }
         } else if( strcmp(zArg, "-bits")==0 ){
            if(Tcl_GetIntFromObj(interp, objv[i+1], &bits) != TCL_OK) {
                return TCL_ERROR;
            }

            if(bits <= 0) {
               Tcl_AppendResult(interp, "Error: bits needs > 0", (char*)0);
               return TCL_ERROR;
            }
         } else if( strcmp(zArg, "-rate")==0 ){
            if(Tcl_GetIntFromObj(interp, objv[i+1], &rate) != TCL_OK) {
                return TCL_ERROR;
            }

            if(rate <= 0) {
               Tcl_AppendResult(interp, "Error: rate needs > 0", (char*)0);
               return TCL_ERROR;
            }
         } else if( strcmp(zArg, "-channels")==0 ){
            if(Tcl_GetIntFromObj(interp, objv[i+1], &channels) != TCL_OK) {
                return TCL_ERROR;
            }

            if(channels <= 0) {
               Tcl_AppendResult(interp, "Error: channels needs > 0", (char*)0);
               return TCL_ERROR;
            }
         } else if( strcmp(zArg, "-byteformat")==0 ){
            if(Tcl_GetIntFromObj(interp, objv[i+1], &byteformat) != TCL_OK) {
                return TCL_ERROR;
            }

            //#define AO_FMT_LITTLE 1
            //#define AO_FMT_BIG    2
            //#define AO_FMT_NATIVE 4
            if(byteformat <= 0 || byteformat > 4) {
               Tcl_AppendResult(interp, "Error: byteformat out of range", (char*)0);
               return TCL_ERROR;
            }
         }else{
            Tcl_AppendResult(interp, "unknown option: ", zArg, (char*)0);
            return TCL_ERROR;
         }
      }

      format.bits = bits;
      format.rate = rate;
      format.channels = channels;
      format.byte_format = byteformat;
      format.matrix = 0;

      filename = Tcl_TranslateFileName(interp, filename, &translatedFilename);
      device = ao_open_file(driver_id, filename, overwrite, &format, NULL);
      Tcl_DStringFree(&translatedFilename);
      if(!device) {
          Tcl_AppendResult(interp, "open_file fail!", (char*)0);
          return TCL_ERROR;
      }

      break;
    }    

    case CMD_PLAY: {
      Tcl_Obj *return_obj;
      char *zData = NULL;
      int len;
      int result;

      if( objc != 3){
        Tcl_WrongNumArgs(interp, 2, objv, "byte_array");
        return TCL_ERROR;
      }

      if(initialize==0) {
        Tcl_AppendResult(interp, "Please execute initialize method first!", (char*)0);
        return TCL_ERROR;
      }

      zData = Tcl_GetStringFromObj(objv[2], &len);
      if( !zData || len < 1 ){
          return TCL_ERROR;
      }

      result = ao_play(device, zData, (uint_32) len);

      return_obj = Tcl_NewBooleanObj(result);
      Tcl_SetObjResult(interp, return_obj);
      break;
    }

    case CMD_CLOSE: {
      Tcl_Obj *return_obj;
      int result = 0;

      if( objc != 2){
        Tcl_WrongNumArgs(interp, 2, objv, 0);
        return TCL_ERROR;
      }

      if(initialize==0) {
        Tcl_AppendResult(interp, "Please execute initialize method first!", (char*)0);
        return TCL_ERROR;
      }
 
      if(device) {
        result = ao_close(device);
        device = NULL;
      }

      return_obj = Tcl_NewBooleanObj(result);
      Tcl_SetObjResult(interp, return_obj);
      break;
    }

    case CMD_SHUTDOWN: {
      if( objc != 2){
        Tcl_WrongNumArgs(interp, 2, objv, 0);
        return TCL_ERROR;
      }

      if(initialize==0) {
        Tcl_AppendResult(interp, "Please execute initialize method first!", (char*)0);
        return TCL_ERROR;
      }

      ao_shutdown();

      Tcl_MutexLock(&myMutex);
      initialize = 0;
      Tcl_MutexUnlock(&myMutex);
      break;
    }

  }

  return TCL_OK;
}


/*
 *----------------------------------------------------------------------
 *
 * Libao_Init --
 *
 *	Initialize the new package.
 *
 * Results:
 *	A standard Tcl result
 *
 * Side effects:
 *	The Libao package is created.
 *
 *----------------------------------------------------------------------
 */

int
Libao_Init(Tcl_Interp *interp)
{
    if (Tcl_InitStubs(interp, "8.4", 0) == NULL) {
	return TCL_ERROR;
    }
    if (Tcl_PkgProvide(interp, PACKAGE_NAME, PACKAGE_VERSION) != TCL_OK) {
	return TCL_ERROR;
    }

    Tcl_CreateObjCommand(interp, "libao::ao", Libao_Cmd, NULL, NULL);

    return TCL_OK;
}
