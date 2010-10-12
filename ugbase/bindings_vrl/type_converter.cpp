#include "type_converter.h"
#include <sstream>

namespace ug {
	namespace vrl {

		jstring stringC2J(JNIEnv *env, std::string const& s) {
			return stringC2J(env, s.c_str());
		}

		jstring stringC2J(JNIEnv *env, const char* s) {
			return env->NewStringUTF(s);
		}

		std::string stringJ2C(JNIEnv *env, jstring const& s) {
			const char* tmpStr = env->GetStringUTFChars(s, false);
			std::string result = (std::string)tmpStr;
			env->ReleaseStringUTFChars(s, tmpStr);
			return result;
		}

		jobjectArray stringArrayC2J(
				JNIEnv *env,
				const std::string* strings,
				const unsigned int array_length) {
			jclass stringClass = env->FindClass("java/lang/String");

			jobjectArray result = env->NewObjectArray(array_length, stringClass, 0);

			// convert array elements
			for (unsigned int i = 0; i < array_length; i++) {
				std::string s = strings[ i ];
				jstring javaString = env->NewStringUTF(s.c_str());
				env->SetObjectArrayElement(result, i, javaString);
			}

			return result;
		}

		jobjectArray stringArrayC2J(JNIEnv *env, std::vector<std::string> const& strings) {
			// it is safe to give a pointer to the first vector element as
			// std::vector implementation uses contiguous memory
			if (strings.size() > 0) {
				return stringArrayC2J(env, &strings[0], strings.size());
			} else {
				jclass cls = env->FindClass("java/lang/Object");
				return env->NewObjectArray(0, cls, 0);
			}
		}

		std::vector<std::string> stringArrayJ2C(JNIEnv *env, jobjectArray const& array) {

			std::vector<std::string> result;

			unsigned int length = env->GetArrayLength(array);

			for (unsigned int i = 0; i < length; i++) {
				result.push_back(stringJ2C(env, (jstring) env->GetObjectArrayElement(array, i)));
			}

			return result;
		}

		std::string exportedFunction2Groovy(ug::bridge::ExportedFunction const& func) {
			std::stringstream result;

			result << "@ComponentInfo(name=\"" << func.name() << "\")\n"

					<< "public class UG4_" << func.name() << " {\n"
					<< "private static final serialVersionUID=1L;\n";

			std::stringstream params;
			std::stringstream paramsArray;

			const ug::bridge::ParameterStack& paramStackIn = func.params_in();
			size_t numParams = paramStackIn.size();

			for (unsigned int i = 0; i < numParams; i++) {
				if (i > 0) {
					params << ", ";
					paramsArray << ", ";
				}
				params << paramType2String(paramStackIn.get_type(i)) << " p" << i;
				paramsArray << " p" << i;
			}

			const ug::bridge::ParameterStack& paramStackOut = func.params_out();
			const char* outType;
			if (paramStackOut.size() > 0) {
				outType = paramType2String(paramStackOut.get_type(0));
			} else {
				outType = "void";
			}

			result << "public " << outType << " " << func.name() << " ("
					<< params.str() << ") {\n"
					<< "Object[] params = [" << paramsArray.str() << "] \n";

			if (paramStackOut.size() > 0) {
				result << "return ";
			}

			result << "edu.gcsc.vrl.ug4.UG4.getUG4().invokeFunction("
					<< (jlong) & func << " as long, params)";

			result << "\n}\n}";

			return result.str();
		}

		jobject int2JObject(JNIEnv *env, jint value) {
			jclass cls = env->FindClass("java/lang/Integer");
			jmethodID methodID = env->GetMethodID(cls, "<init>", "(I)V");
			return env->NewObject(cls, methodID, value);
		}

		jint jObject2Int(JNIEnv *env, jobject obj) {
			jclass argClass = env->GetObjectClass(obj);
			jmethodID ajf = env->GetMethodID(argClass, "intValue", "()I");
			return env->CallIntMethod(obj, ajf);
		}

		jobject double2JObject(JNIEnv *env, jdouble value) {
			jclass cls = env->FindClass("java/lang/Double");
			jmethodID methodID = env->GetMethodID(cls, "<init>", "(D)V");
			return env->NewObject(cls, methodID, value);
		}

		jdouble jObject2Double(JNIEnv *env, jobject obj) {
			jclass argClass = env->GetObjectClass(obj);
			jmethodID ajf = env->GetMethodID(argClass, "doubleValue", "()D");
			return env->CallDoubleMethod(obj, ajf);
		}

		jobject boolean2JObject(JNIEnv *env, jboolean value) {
			jclass cls = env->FindClass("java/lang/Boolean");
			jmethodID methodID = env->GetMethodID(cls, "<init>", "(Z)V");
			return env->NewObject(cls, methodID, value);
		}

		jboolean jObject2Boolean(JNIEnv *env, jobject obj) {
			jclass argClass = env->GetObjectClass(obj);
			jmethodID ajf = env->GetMethodID(argClass, "booleanValue", "()Z");
			return env->CallBooleanMethod(obj, ajf);
		}

		void* jObject2Pointer(JNIEnv *env, jobject obj) {
			VRL_DBG("INIT",1);
			jclass argClass = env->GetObjectClass(obj);
			VRL_DBG("AFTER GET_OBJECT",1);
			jmethodID ajf = env->GetMethodID(argClass, "getPtr", "()J");
			VRL_DBG("AFTER METHOD",1);
			return (void*) env->CallLongMethod(obj, ajf);
		}

		jobject pointer2JObject(JNIEnv *env, void* value) {
			VRL_DBG("INIT",1);
			jclass cls = env->FindClass("edu/gcsc/vrl/ug4/Pointer");
			VRL_DBG("AFTER CLASS",1);
			jmethodID methodID = env->GetMethodID(cls, "<init>", "(J)V");
			VRL_DBG("AFTER METHOD",1);
			return env->NewObject(cls, methodID, (jlong) value);
		}

		jobject string2JObject(JNIEnv *env, const char* value) {
			return env->NewStringUTF(value);
		}

		std::string jObject2String(JNIEnv *env, jobject obj) {
			return stringJ2C(env, (jstring) obj);
		}

		const char* paramType2String(int paramType) {

			switch (paramType) {
				case ug::bridge::PT_BOOL: return "boolean";
				case ug::bridge::PT_INTEGER: return "int";
				case ug::bridge::PT_NUMBER: return "double";
				case ug::bridge::PT_STRING: return "String";
				case ug::bridge::PT_POINTER: return "edu.gcsc.vrl.ug4.Pointer";
				case ug::bridge::PT_CONST_POINTER: return "edu.gcsc.vrl.ug4.Pointer";
				default: return "Object";
			}
		}

		int jobjectArray2ParamStack(JNIEnv *env, ug::bridge::ParameterStack& paramsOut,
				const ug::bridge::ParameterStack& paramsTemplate,
				jobjectArray const& array) {
			using namespace ug::bridge;

			std::vector<std::string> stringParams;

			//	iterate through the parameter list and copy the value in the associated
			//	stack entry.
			for (int i = 0; i < paramsTemplate.size(); ++i) {
				int type = paramsTemplate.get_type(i);
//				const std::vector<const char*>* classNames = paramsTemplate.class_names(i);

//				std::stringstream paramMessage;

//				paramMessage << "CLASSNAMES for PARAM " << i << ": ";
//
//				for (unsigned int j = 0; j < classNames->size();j++) {
//					paramMessage << "(" << j << "):" << (*classNames)[j] <<  ",";
//				}
//
//				VRL_DBG(paramMessage.str(), 1);


				jobject value = env->GetObjectArrayElement(array, i);

				switch (type) {
					case PT_BOOL:
					{
						paramsOut.push_bool(jObject2Boolean(env, value));
					}
						break;
					case PT_INTEGER:
					{
						paramsOut.push_integer(jObject2Int(env, value));
					}
						break;
					case PT_NUMBER:
					{
						paramsOut.push_number(jObject2Double(env, value));
					}
						break;
					case PT_STRING:
					{
						paramsOut.push_string(jObject2String(env, value).c_str(), true);
					}
						break;

					case PT_POINTER:
					{
						paramsOut.push_pointer(jObject2Pointer(env, value));
					}
						break;
					case PT_CONST_POINTER:
					{
						paramsOut.push_pointer(jObject2Pointer(env, value));
					}
						break;
				}

			}

			return 0;
		}

		jobject param2JObject(JNIEnv *env, ug::bridge::ParameterStack& params, int index) {
			using namespace ug::bridge;
			//	iterate through the parameter list and copy the value in the associated
			//	stack entry.
			int type = params.get_type(index);

			switch (type) {
				case PT_BOOL:
				{
					return boolean2JObject(env, params.to_bool(index));
				}
					break;
				case PT_INTEGER:
				{
					return int2JObject(env, params.to_integer(index));
				}
					break;
				case PT_NUMBER:
				{
					return double2JObject(env, params.to_number(index));
				}
					break;
				case PT_STRING:
				{
					return string2JObject(env, params.to_string(index));
				}
					break;
				case PT_POINTER:
				{
					return pointer2JObject(env, params.to_pointer(index));
				}
					break;
				case PT_CONST_POINTER:
				{
					return pointer2JObject(env, params.to_pointer(index));
				}
					break;
			}

			return jobject();
		}

		/*
				static int ParamsToLuaStack(const ParameterStack& params, lua_State* L)
				{
				//	push output parameters to the stack
					for(int i = 0; i < params.size(); ++i){
						int type = params.get_type(i);
						switch(type){
							case PT_BOOL:{
								lua_pushboolean(L, (params.to_bool(i)) ? 1 : 0);
							}break;
							case PT_INTEGER:{
								lua_pushnumber(L, params.to_integer(i));
							}break;
							case PT_NUMBER:{
								lua_pushnumber(L, params.to_number(i));
							}break;
							case PT_STRING:{
								lua_pushstring(L, params.to_string(i));
							}break;
							case PT_POINTER:{
								void* obj = params.to_pointer(i);
								CreateNewUserData(L, obj, params.class_name(i), false);
							}break;
							case PT_CONST_POINTER:{
							//	we're removing const with a cast. However, it was made sure that
							//	obj is treated as a const value.
								void* obj = (void*)params.to_const_pointer(i);
								CreateNewUserData(L, obj, params.class_name(i), true);
							}break;
							default:{
								UG_LOG("ERROR in ParamsToLuaStack: Unknown parameter in ParameterList. ");
								UG_LOG("Return-values may be incomplete.\n");
								return (int)i;
							}break;
						}
					}

					return (int)params.size();
				}
		 */
	} // end vrl::
}// end ug::
