/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class app_zxtune_ZXTune */

#ifndef _Included_app_zxtune_ZXTune
#define _Included_app_zxtune_ZXTune
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     app_zxtune_ZXTune
 * Method:    GlobalOptions_GetProperty
 * Signature: (Ljava/lang/String;J)J
 */
JNIEXPORT jlong JNICALL Java_app_zxtune_ZXTune_GlobalOptions_1GetProperty__Ljava_lang_String_2J
  (JNIEnv *, jclass, jstring, jlong);

/*
 * Class:     app_zxtune_ZXTune
 * Method:    GlobalOptions_GetProperty
 * Signature: (Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_app_zxtune_ZXTune_GlobalOptions_1GetProperty__Ljava_lang_String_2Ljava_lang_String_2
  (JNIEnv *, jclass, jstring, jstring);

/*
 * Class:     app_zxtune_ZXTune
 * Method:    GlobalOptions_SetProperty
 * Signature: (Ljava/lang/String;J)V
 */
JNIEXPORT void JNICALL Java_app_zxtune_ZXTune_GlobalOptions_1SetProperty__Ljava_lang_String_2J
  (JNIEnv *, jclass, jstring, jlong);

/*
 * Class:     app_zxtune_ZXTune
 * Method:    GlobalOptions_SetProperty
 * Signature: (Ljava/lang/String;Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_app_zxtune_ZXTune_GlobalOptions_1SetProperty__Ljava_lang_String_2Ljava_lang_String_2
  (JNIEnv *, jclass, jstring, jstring);

/*
 * Class:     app_zxtune_ZXTune
 * Method:    Handle_Close
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_app_zxtune_ZXTune_Handle_1Close
  (JNIEnv *, jclass, jint);

/*
 * Class:     app_zxtune_ZXTune
 * Method:    Module_Create
 * Signature: (Ljava/nio/ByteBuffer;)I
 */
JNIEXPORT jint JNICALL Java_app_zxtune_ZXTune_Module_1Create
  (JNIEnv *, jclass, jobject);

/*
 * Class:     app_zxtune_ZXTune
 * Method:    Module_GetDuration
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_app_zxtune_ZXTune_Module_1GetDuration
  (JNIEnv *, jclass, jint);

/*
 * Class:     app_zxtune_ZXTune
 * Method:    Module_GetProperty
 * Signature: (ILjava/lang/String;J)J
 */
JNIEXPORT jlong JNICALL Java_app_zxtune_ZXTune_Module_1GetProperty__ILjava_lang_String_2J
  (JNIEnv *, jclass, jint, jstring, jlong);

/*
 * Class:     app_zxtune_ZXTune
 * Method:    Module_GetProperty
 * Signature: (ILjava/lang/String;Ljava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_app_zxtune_ZXTune_Module_1GetProperty__ILjava_lang_String_2Ljava_lang_String_2
  (JNIEnv *, jclass, jint, jstring, jstring);

/*
 * Class:     app_zxtune_ZXTune
 * Method:    Module_CreatePlayer
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_app_zxtune_ZXTune_Module_1CreatePlayer
  (JNIEnv *, jclass, jint);

/*
 * Class:     app_zxtune_ZXTune
 * Method:    Player_Render
 * Signature: (I[S)Z
 */
JNIEXPORT jboolean JNICALL Java_app_zxtune_ZXTune_Player_1Render
  (JNIEnv *, jclass, jint, jshortArray);

/*
 * Class:     app_zxtune_ZXTune
 * Method:    Player_Analyze
 * Signature: (I[I[I)I
 */
JNIEXPORT jint JNICALL Java_app_zxtune_ZXTune_Player_1Analyze
  (JNIEnv *, jclass, jint, jintArray, jintArray);

/*
 * Class:     app_zxtune_ZXTune
 * Method:    Player_GetPosition
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_app_zxtune_ZXTune_Player_1GetPosition
  (JNIEnv *, jclass, jint);

/*
 * Class:     app_zxtune_ZXTune
 * Method:    Player_SetPosition
 * Signature: (II)V
 */
JNIEXPORT void JNICALL Java_app_zxtune_ZXTune_Player_1SetPosition
  (JNIEnv *, jclass, jint, jint);

/*
 * Class:     app_zxtune_ZXTune
 * Method:    Player_GetProperty
 * Signature: (ILjava/lang/String;J)J
 */
JNIEXPORT jlong JNICALL Java_app_zxtune_ZXTune_Player_1GetProperty__ILjava_lang_String_2J
  (JNIEnv *, jclass, jint, jstring, jlong);

/*
 * Class:     app_zxtune_ZXTune
 * Method:    Player_GetProperty
 * Signature: (ILjava/lang/String;Ljava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_app_zxtune_ZXTune_Player_1GetProperty__ILjava_lang_String_2Ljava_lang_String_2
  (JNIEnv *, jclass, jint, jstring, jstring);

/*
 * Class:     app_zxtune_ZXTune
 * Method:    Player_SetProperty
 * Signature: (ILjava/lang/String;J)V
 */
JNIEXPORT void JNICALL Java_app_zxtune_ZXTune_Player_1SetProperty__ILjava_lang_String_2J
  (JNIEnv *, jclass, jint, jstring, jlong);

/*
 * Class:     app_zxtune_ZXTune
 * Method:    Player_SetProperty
 * Signature: (ILjava/lang/String;Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_app_zxtune_ZXTune_Player_1SetProperty__ILjava_lang_String_2Ljava_lang_String_2
  (JNIEnv *, jclass, jint, jstring, jstring);

#ifdef __cplusplus
}
#endif
#endif
/* Header for class app_zxtune_ZXTune_Properties */

#ifndef _Included_app_zxtune_ZXTune_Properties
#define _Included_app_zxtune_ZXTune_Properties
#ifdef __cplusplus
extern "C" {
#endif
#ifdef __cplusplus
}
#endif
#endif
/* Header for class app_zxtune_ZXTune_Properties_Accessor */

#ifndef _Included_app_zxtune_ZXTune_Properties_Accessor
#define _Included_app_zxtune_ZXTune_Properties_Accessor
#ifdef __cplusplus
extern "C" {
#endif
#ifdef __cplusplus
}
#endif
#endif
/* Header for class app_zxtune_ZXTune_Properties_Modifier */

#ifndef _Included_app_zxtune_ZXTune_Properties_Modifier
#define _Included_app_zxtune_ZXTune_Properties_Modifier
#ifdef __cplusplus
extern "C" {
#endif
#ifdef __cplusplus
}
#endif
#endif
/* Header for class app_zxtune_ZXTune_Properties_Sound */

#ifndef _Included_app_zxtune_ZXTune_Properties_Sound
#define _Included_app_zxtune_ZXTune_Properties_Sound
#ifdef __cplusplus
extern "C" {
#endif
#undef app_zxtune_ZXTune_Properties_Sound_FRAMEDURATION_DEFAULT
#define app_zxtune_ZXTune_Properties_Sound_FRAMEDURATION_DEFAULT 20000LL
#ifdef __cplusplus
}
#endif
#endif
/* Header for class app_zxtune_ZXTune_Properties_Core */

#ifndef _Included_app_zxtune_ZXTune_Properties_Core
#define _Included_app_zxtune_ZXTune_Properties_Core
#ifdef __cplusplus
extern "C" {
#endif
#ifdef __cplusplus
}
#endif
#endif
/* Header for class app_zxtune_ZXTune_Properties_Core_Aym */

#ifndef _Included_app_zxtune_ZXTune_Properties_Core_Aym
#define _Included_app_zxtune_ZXTune_Properties_Core_Aym
#ifdef __cplusplus
extern "C" {
#endif
#ifdef __cplusplus
}
#endif
#endif
/* Header for class app_zxtune_ZXTune_Module */

#ifndef _Included_app_zxtune_ZXTune_Module
#define _Included_app_zxtune_ZXTune_Module
#ifdef __cplusplus
extern "C" {
#endif
#ifdef __cplusplus
}
#endif
#endif
/* Header for class app_zxtune_ZXTune_Module_Attributes */

#ifndef _Included_app_zxtune_ZXTune_Module_Attributes
#define _Included_app_zxtune_ZXTune_Module_Attributes
#ifdef __cplusplus
extern "C" {
#endif
#ifdef __cplusplus
}
#endif
#endif
/* Header for class app_zxtune_ZXTune_Player */

#ifndef _Included_app_zxtune_ZXTune_Player
#define _Included_app_zxtune_ZXTune_Player
#ifdef __cplusplus
extern "C" {
#endif
#ifdef __cplusplus
}
#endif
#endif
/* Header for class app_zxtune_ZXTune_GlobalOptions */

#ifndef _Included_app_zxtune_ZXTune_GlobalOptions
#define _Included_app_zxtune_ZXTune_GlobalOptions
#ifdef __cplusplus
extern "C" {
#endif
#ifdef __cplusplus
}
#endif
#endif
/* Header for class app_zxtune_ZXTune_GlobalOptions_Holder */

#ifndef _Included_app_zxtune_ZXTune_GlobalOptions_Holder
#define _Included_app_zxtune_ZXTune_GlobalOptions_Holder
#ifdef __cplusplus
extern "C" {
#endif
#ifdef __cplusplus
}
#endif
#endif
/* Header for class app_zxtune_ZXTune_NativeObject */

#ifndef _Included_app_zxtune_ZXTune_NativeObject
#define _Included_app_zxtune_ZXTune_NativeObject
#ifdef __cplusplus
extern "C" {
#endif
#ifdef __cplusplus
}
#endif
#endif
/* Header for class app_zxtune_ZXTune_NativeModule */

#ifndef _Included_app_zxtune_ZXTune_NativeModule
#define _Included_app_zxtune_ZXTune_NativeModule
#ifdef __cplusplus
extern "C" {
#endif
#ifdef __cplusplus
}
#endif
#endif
/* Header for class app_zxtune_ZXTune_NativePlayer */

#ifndef _Included_app_zxtune_ZXTune_NativePlayer
#define _Included_app_zxtune_ZXTune_NativePlayer
#ifdef __cplusplus
extern "C" {
#endif
#ifdef __cplusplus
}
#endif
#endif
