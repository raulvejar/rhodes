/*------------------------------------------------------------------------
* (The MIT License)
* 
* Copyright (c) 2008-2011 Rhomobile, Inc.
* 
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
* 
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
* 
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
* 
* http://rhomobile.com
*------------------------------------------------------------------------*/

#include "rhodes/JNIRhodes.h"
#include "rhodes/JNIRhoRuby.h"

#include <android/log.h>
#include <logging/RhoLogConf.h>

#undef DEFAULT_LOGCATEGORY
#define DEFAULT_LOGCATEGORY "PhonebookJNI"

#define logging_enable true

static const char* field_names[] = {
"id", // 0
"display_name", // 1
"first_name",  // 2
"last_name",  // 3
"mobile_number", // 4
"home_number",  // 5
"business_number",  // 6
"email_address",  // 7
"company_name"  // 8
};

#define PB_FIELDS_COUNT sizeof(field_names)/sizeof(field_names[0])

RHO_GLOBAL void* openPhonebook()
{
    JNIEnv *env = jnienv();
    jclass cls = getJNIClass(RHODES_JAVA_CLASS_PHONEBOOK);
    if (!cls) return NULL;
    jmethodID cid = getJNIClassMethod(env, cls, "<init>", "()V");
    if (!cid) return NULL;

    jobject local = env->NewObject(cls, cid);
    jobject obj = env->NewGlobalRef(local);
    env->DeleteLocalRef(local);
    return obj;
}

RHO_GLOBAL void closePhonebook(void* pb)
{
    JNIEnv *env = jnienv();
    jobject obj = (jobject)pb;
    jclass cls = getJNIClass(RHODES_JAVA_CLASS_PHONEBOOK);
    if (!cls) return;
    jmethodID mid = getJNIClassMethod(env, cls, "close", "()V");
    if (!mid) return;

    env->CallVoidMethod(obj, mid);
    env->DeleteGlobalRef(obj);
}

static VALUE createHashFromContact(jobject contactObj)
{
    if (logging_enable) RAWLOG_INFO("createHashFromContact() START");
    JNIEnv *env = jnienv();
    jclass contactCls = getJNIClass(RHODES_JAVA_CLASS_CONTACT);
    if (!contactCls) return Qnil;
    jclass fieldCls = getJNIClass(RHODES_JAVA_CLASS_CONTACT_FIELD);
    if (!fieldCls) return Qnil;

    jmethodID contactGetFieldMID = getJNIClassMethod(env, contactCls, "getField", "(Ljava/lang/String;)Ljava/lang/String;");
    if (!contactGetFieldMID) return Qnil;

    CHoldRubyValue contactHash(rho_ruby_createHash());
    // contact.moveToBegin();

    for (unsigned i = 0; i < PB_FIELDS_COUNT; ++i) {
        jhstring jhFieldName = rho_cast<jhstring>(env, field_names[i]);
        jhstring jhField(static_cast<jstring>(env->CallObjectMethod(contactObj, contactGetFieldMID, jhFieldName.get())));
        if (jhField.get() != 0) {
            addStrToHash(contactHash, field_names[i], rho_cast<std::string>(env, jhField).c_str());
        }
    }
    if (logging_enable) RAWLOG_INFO("createHashFromContact() FINISH");
    return contactHash;
}

RHO_GLOBAL VALUE getPhonebookRecords(void* pb, int offset, int max_results, rho_param* select_param)
{
    if (logging_enable) RAWLOG_INFO2("getPhonebookRecords(%d, %d) START", offset, max_results);
    jobject phonebookObj = (jobject)pb;

    JNIEnv *env = jnienv();

    jclass phonebookCls = getJNIClass(RHODES_JAVA_CLASS_PHONEBOOK);
    if (!phonebookCls) return Qnil;
    jclass contactCls = getJNIClass(RHODES_JAVA_CLASS_CONTACT);
    if (!contactCls) return Qnil;

    jmethodID queryMID = getJNIClassMethod(env, phonebookCls, "queryContacts", "(IILjava/util/List;)V");
    if (!queryMID) return Qnil;

    jmethodID phonebookMoveToBeginMID = getJNIClassMethod(env, phonebookCls, "moveToBegin", "()V");
    if (!phonebookMoveToBeginMID) return Qnil;
    jmethodID hasNextMID = getJNIClassMethod(env, phonebookCls, "hasNext", "()Z");
    if (!hasNextMID) return Qnil;
    jmethodID nextMID = getJNIClassMethod(env, phonebookCls, "next", "()Ljava/lang/Object;");
    if (!nextMID) return Qnil;
    jmethodID contactIdMID = getJNIClassMethod(env, contactCls, "id", "()Ljava/lang/String;");
    if (!contactIdMID) return Qnil;

    jmethodID contactGetFieldMID = getJNIClassMethod(env, contactCls, "getField", "(Ljava/lang/String;)Ljava/lang/String;");
    if (!contactGetFieldMID) return Qnil;

    jobject selectObj = NULL;
    if (select_param) {
        RAWLOG_INFO("Converting 'select_param'.");
        selectObj = RhoValueConverter(env).createObject(select_param);
    }
    env->CallVoidMethod(phonebookObj, queryMID, offset, max_results, selectObj);
    env->DeleteLocalRef(selectObj);
    env->CallVoidMethod(phonebookObj, phonebookMoveToBeginMID);

    VALUE valGc = rho_ruby_disable_gc();
    CHoldRubyValue hash(rho_ruby_createHash());
    // while(pb.hasNext())
    while(env->CallBooleanMethod(phonebookObj, hasNextMID))
    {
        // Contact contact = (Contact)pb.next();
        jobject contactObj = env->CallObjectMethod(phonebookObj, nextMID);
        if (!contactObj) return Qnil;
        // String id = contact.id();
        jstring idObj = (jstring)env->CallObjectMethod(contactObj, contactIdMID);
        if (!idObj) return Qnil;

        //addHashToHash(hash, rho_cast<std::string>(idObj).c_str(), createHashFromContact(contactObj));
        CHoldRubyValue contactHash(rho_ruby_createHash());
        // contact.moveToBegin();

        for (unsigned i = 0; i < PB_FIELDS_COUNT; ++i) {
            jhstring jhFieldName = rho_cast<jhstring>(env, field_names[i]);
            jhstring jhField(static_cast<jstring>(env->CallObjectMethod(contactObj, contactGetFieldMID, jhFieldName.get())));
            if (jhField.get() != 0) {
                addStrToHash(contactHash, field_names[i], rho_cast<std::string>(env, jhField).c_str());
            }
        }

        addHashToHash(hash, rho_cast<std::string>(idObj).c_str(), contactHash);

        env->DeleteLocalRef(idObj);
        env->DeleteLocalRef(contactObj);
    }

    rho_ruby_enable_gc(valGc);
    if (logging_enable) RAWLOG_INFO("getPhonebookRecords() FINISH");
    return hash;
}

RHO_GLOBAL int getPhonebookRecordCount(void* pb, int offset, int max_results)
{
    if (logging_enable) RAWLOG_INFO2("getPhonebookRecordCount(%d, %d) START", offset, max_results);
    jobject phonebookObj = static_cast<jobject>(pb);

    JNIEnv *env = jnienv();

    jclass phonebookCls = getJNIClass(RHODES_JAVA_CLASS_PHONEBOOK);
    if (!phonebookCls) return 0;

    jmethodID queryMID = getJNIClassMethod(env, phonebookCls, "queryContactCount", "(II)I");
    if (!queryMID) return 0;

    int contactCount =  env->CallIntMethod(phonebookObj, queryMID, offset, max_results);

    if (logging_enable) RAWLOG_INFO("getPhonebookRecordCount() FINISH");
    return contactCount;
}

RHO_GLOBAL VALUE getallPhonebookRecords(void* pb)
{
    if (logging_enable) RAWLOG_INFO("getallPhonebookRecords() START");
    VALUE res = getPhonebookRecords(pb, 0, -1, 0);
    if (logging_enable) RAWLOG_INFO("getallPhonebookRecords() FINISH");
    return res;
}

RHO_GLOBAL void* openPhonebookRecord(void* pb, char* id)
{
    if (logging_enable) RAWLOG_INFO("openPhonebookRecord() START");
    JNIEnv *env = jnienv();
    jobject obj = (jobject)pb;
    jclass cls = getJNIClass(RHODES_JAVA_CLASS_PHONEBOOK);
    if (!cls) return NULL;
    jmethodID mid = getJNIClassMethod(env, cls, "getRecord", "(Ljava/lang/String;)Lcom/rhomobile/rhodes/phonebook/Contact;");
    if (!mid) return NULL;

    std::string rawId(id+1, strlen(id)-2);
    jhstring jhId = rho_cast<jhstring>(env, rawId.c_str());
//    jhstring jhId = rho_cast<jhstring>(env, id);
    jhobject recordObj = jhobject(env->CallObjectMethod(obj, mid, jhId.get()));
    if (!recordObj) return NULL;
    jhobject retval = jhobject(env->NewGlobalRef(recordObj.get()));

    if (logging_enable) RAWLOG_INFO("openPhonebookRecord() FINISH");

    if (!retval) return NULL;
    return retval.release();
}

RHO_GLOBAL VALUE getPhonebookRecord(void* pb, char* id)
{
    if (logging_enable) RAWLOG_INFO("getPhonebookRecord() START");
    jobject recordObj = (jobject)openPhonebookRecord(pb, id);
    if (!recordObj) {
	if (logging_enable) RAWLOG_INFO("getPhonebookRecord() FINISH return NIL");
        return Qnil;
    }
    VALUE retval = createHashFromContact(recordObj);
    jnienv()->DeleteGlobalRef(recordObj);
    if (logging_enable) RAWLOG_INFO("getPhonebookRecord() FINISH");
    return retval;
}

static VALUE getRecord(void *pb, const char *name)
{
    if (logging_enable) RAWLOG_INFO("getRecord() START");
    jobject obj = (jobject)pb;
    JNIEnv *env = jnienv();
    if (!env) {
        if (logging_enable) RAWLOG_INFO("getRecord() FINISH return NIL0");
        return Qnil;
    }
    jclass cls = getJNIClass(RHODES_JAVA_CLASS_PHONEBOOK);
    if (!cls) {
        if (logging_enable) RAWLOG_INFO("getRecord() FINISH return NIL1");
	return Qnil;
    }
    jmethodID mid = getJNIClassMethod(env, cls, name, "()Lcom/rhomobile/rhodes/phonebook/Contact;");
    if (!mid) {
        if (logging_enable) RAWLOG_INFO("getRecord() FINISH return NIL2");
	return Qnil; 
    }	
    jobject recordObj = env->CallObjectMethod(obj, mid);
    if (!recordObj) {
        if (logging_enable) RAWLOG_INFO("getRecord() FINISH return NIL");
        return Qnil;
    }
    if (logging_enable) RAWLOG_INFO("getRecord() FINISH");
    return createHashFromContact(recordObj);
}

RHO_GLOBAL VALUE getfirstPhonebookRecord(void* pb)
{
    return getRecord(pb, "getFirstRecord");
}

RHO_GLOBAL VALUE getnextPhonebookRecord(void* pb)
{
    return getRecord(pb, "getNextRecord");
}

RHO_GLOBAL void* createRecord(void* pb)
{
    JNIEnv *env = jnienv();
    jclass cls = getJNIClass(RHODES_JAVA_CLASS_CONTACT);
    if (!cls) return NULL;
    jmethodID cid = getJNIClassMethod(env, cls, "<init>", "()V");
    if (!cid) return NULL;

    jobject local = env->NewObject(cls, cid);
    jobject obj = env->NewGlobalRef(local);
    env->DeleteLocalRef(local);
    return obj;
}

RHO_GLOBAL int setRecordValue(void* record, char* property, char* value)
{
    jobject contactObj = (jobject)record;
    JNIEnv *env = jnienv();
    jclass contactCls = getJNIClass(RHODES_JAVA_CLASS_CONTACT);
    if (!contactCls) return 0;
    jmethodID mid = getJNIClassMethod(env, contactCls, "setField", "(Ljava/lang/String;Ljava/lang/String;)V");
    if (!mid) return 0;

    jhstring jhName = rho_cast<jhstring>(env, property);
    jhstring jhValue = rho_cast<jhstring>(env, value);

    env->CallVoidMethod(contactObj, mid, jhName.get(), jhValue.get());

    return 1;
}

static void doContactOp(jobject pbObj, jobject contactObj, const char *name)
{
    JNIEnv *env = jnienv();
    jclass pbCls = getJNIClass(RHODES_JAVA_CLASS_PHONEBOOK);
    if (!pbCls) return;
    jmethodID mid = getJNIClassMethod(env, pbCls, name, "(Lcom/rhomobile/rhodes/phonebook/Contact;)V");
    if (!mid) return;

    env->CallVoidMethod(pbObj, mid, contactObj);
}

RHO_GLOBAL int addRecord(void* pb, void* record)
{
    doContactOp((jobject)pb, (jobject)record, "saveContact");
    jnienv()->DeleteGlobalRef((jobject)record);
    return 1;
}

RHO_GLOBAL int saveRecord(void* pb, void* record)
{
    doContactOp((jobject)pb, (jobject)record, "saveContact");
    return 1;
}

RHO_GLOBAL int deleteRecord(void* pb, void* record)
{
    doContactOp((jobject)pb, (jobject)record, "removeContact");
    jnienv()->DeleteGlobalRef((jobject)record);
    return 1;
}

