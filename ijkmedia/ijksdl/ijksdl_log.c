#include "ijksdl_log.h"

#ifdef __ANDROID__
#include <jni.h>
#include <android/log.h>
#endif

#define LOG_BUF_SIZE	2048

static JavaVM* gs_jvm = NULL;

jclass gs_clazz;
jmethodID gs_mid_log;
static int gs_inited = 0;

int java_log(int level, const char* tag, const char* msg);

int my_log_init(JavaVM *jvm)
{
	gs_jvm = jvm;

	if (gs_inited)
		return 0;

	if (NULL == gs_jvm)
		return -1;

	JNIEnv* env = NULL;

	if ((*gs_jvm)->GetEnv(gs_jvm, (void**)&env, JNI_VERSION_1_4) != JNI_OK)
		return -1;

	jclass clazz = (*env)->FindClass(env, "tv/danmaku/ijk/media/player/LogUtils");
	if (NULL == clazz) {
		//AND_LOGE("failed to find class tv/danmaku/ijk/media/player/LogUtils");
		return -1;
	}

	gs_mid_log = (*env)->GetStaticMethodID(env, clazz, "nativeLog", "(ILjava/lang/String;Ljava/lang/String;)V");
	if (NULL == gs_mid_log) {
		//AND_LOGE("failed to find nativeLog methodID");
		return -1;
	}

	gs_clazz = (jclass)((*env)->NewGlobalRef(env, clazz));

	gs_inited = 1;
	//PPLOGI("pplog inited");
	return 0;
}

void my_log_uninit()
{
	JNIEnv* env = NULL;

	if ((*gs_jvm)->GetEnv(gs_jvm, (void**)&env, JNI_VERSION_1_4) == JNI_OK) {
		(*env)->DeleteGlobalRef(env, gs_clazz);
	}
}

void my_log_print(int level, const char *tag,  const char *fmt, ...)
{
    //int log_res = 0;
    va_list ap;
    va_start(ap, fmt);
    //if (pplog != NULL){
    //    log_res = pplog(prio, tag, fmt, ap);
    //}
    //if (log_res != 0){
        __android_log_vprint(level, tag, fmt, ap);
    //}

    char buf[LOG_BUF_SIZE] = {0};
    vsnprintf(buf, LOG_BUF_SIZE, fmt, ap);
    java_log(level, tag, buf);

    va_end(ap);
}

void my_log_vprint(int level, const char *tag, const char *fmt, va_list ap)
{
    __android_log_vprint(level, tag, fmt, ap);

    char buf[LOG_BUF_SIZE];
    vsnprintf(buf, LOG_BUF_SIZE, fmt, ap);
    java_log(level, tag, buf);
}

int java_log(int level, const char* tag, const char* msg)
{
	if (!gs_inited)
		return -1;

	JNIEnv* env = NULL;

	if (NULL != gs_jvm)
		(*gs_jvm)->GetEnv(gs_jvm, (void**)&env, JNI_VERSION_1_4);

	if (!env)
		return -1;

	//if (!IsUTF8(msg, strlen(msg))) {
	//	AND_LOGE("string is not utf-8(java_log): %s", msg);
	//	return -1;
	//}

	jstring jtag = (*env)->NewStringUTF(env, tag);
	jstring jmsg = (*env)->NewStringUTF(env, msg);

	(*env)->CallStaticVoidMethod(env, gs_clazz, gs_mid_log, level, jtag, jmsg);
	(*env)->DeleteLocalRef(env, jtag);
	(*env)->DeleteLocalRef(env, jmsg);
	return 0;
}
