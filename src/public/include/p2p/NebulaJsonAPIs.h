/*! \file NebulaJsonAPIs.h
This file describes all the APIs of the IOTC module in IOTC platform.
IOTC module is a kind of data communication modules to provide basic data
transfer among devices and clients.

\copyright Copyright (c) 2010 by Throughtek Co., Ltd. All Rights Reserved.

Revision Table

Version     | Name             |Date           |Description
------------|------------------|---------------|-------------------
4.0.0.0     |Terry Liu         |2019-08-07     |+ Add Nebula_Json_Obj_Create_From_String, Nebula_Json_Obj_Release
4.0.0.0     |Terry Liu         |2019-08-07     |+ Add Nebula_Json_Obj_Get_length, Nebula_Json_Obj_Get_Sub_Obj
4.0.0.0     |Terry Liu         |2019-08-07     |+ Add Nebula_Json_Obj_Get_Type, Nebula_Json_Obj_Get_Bool, Nebula_Json_Obj_Get_Sub_Obj_Int
4.0.0.0     |Terry Liu         |2019-08-07     |+ Add Nebula_Json_Obj_Get_Double, Nebula_Json_Obj_Get_Int, Nebula_Json_Obj_Get_Sub_Obj_String
4.0.0.0     |Terry Liu         |2019-08-07     |+ Add Nebula_Json_Obj_Get_String, Nebula_Json_Obj_Get_Array_length
4.0.0.0     |Terry Liu         |2019-08-07     |+ Add Nebula_Json_Obj_Get_Array_Element, Nebula_Json_Obj_To_String
4.0.0.0     |Terry Liu         |2019-08-07     |+ Add Nebula_Json_Obj_Get_Sub_Obj_Bool, Nebula_Json_Obj_Get_Sub_Obj_Double
 */
#ifndef _NEBULA_JSONAPIs_H_
#define _NEBULA_JSONAPIs_H_

#include <stdint.h>

#ifdef _WIN32
    /** @cond */
    #ifdef IOTC_STATIC_LIB
    #define NEBULA_JSON_API
    #elif defined AVAPI_EXPORTS
    #define NEBULA_JSON_API __declspec(dllexport)
    #else
    #define NEBULA_JSON_API __declspec(dllimport)
    #endif // #ifdef P2PAPI_EXPORTS
    /** @endcond */
#else // #ifdef _WIN32
	#define NEBULA_JSON_API
#endif //#ifdef _WIN32

#if defined(__GNUC__) || defined(__clang__)
    #define NEBULA_JSON_API_DEPRECATED __attribute__((deprecated))
    #elif defined(_MSC_VER)
    #ifdef IOTC_STATIC_LIB
    #define NEBULA_JSON_API_DEPRECATED __declspec(deprecated)
        #elif defined P2PAPI_EXPORTS
    #define NEBULA_JSON_API_DEPRECATED __declspec(deprecated, dllexport)
    #else
    #define NEBULA_JSON_API_DEPRECATED __declspec(deprecated, dllimport)
    #endif
#else
    #define NEBULA_JSON_API_DEPRECATED
#endif

#ifndef _WIN32
#define __stdcall
#endif // #ifndef _WIN32


/* ============================================================================
 * Platform Dependant Macro Definition
 * ============================================================================
 */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* ============================================================================
 * Enumeration Declaration
 * ============================================================================
 */

typedef enum NebulaJsonType {
  JSON_TYPE_NULL,
  JSON_TYPE_BOOL,
  JSON_TYPE_DOUBLE,
  JSON_TYPE_INT,
  JSON_TYPE_OBJECT,
  JSON_TYPE_ARRAY,
  JSON_TYPE_STRING
} NebulaJsonType;

/* ============================================================================
 * Type Definition
 * ============================================================================
 */

typedef struct NebulaJsonObject NebulaJsonObject;

/**
 * \brief New a NebulaJsonObject from vaild JSON string.
 *
 * \details Parse a string and return a non-NULL json_object if a valid JSON value is found.
 *
 * \param json_str [in] String of vaild JSON.
 * \param json_obj [out] A NebulaJsonObject which type is JSON_TYPE_OBJECT.
 *
 * \return #NEBULA_ER_NoERROR if create successfully
 * \return Error code if return value < 0
 *			- #NEBULA_ER_INVALID_ARG Input null argument
 *			- #NEBULA_ER_JSON_OBJ_TYPE_ERROR json_str is not a JSON format string
 *
 */
NEBULA_JSON_API int32_t Nebula_Json_Obj_Create_From_String(const char *json_str, NebulaJsonObject **json_obj);

/**
 * \brief Free a root NebulaJsonObject
 *
 * \details Free a root NebulaJsonObject.Make sure to free the NebulaJsonObject that ceated by 
 *          Nebula_Json_Obj_New_Empty_Obj or Nebula_Json_Obj_Create_From_String or it may cause
 *          memory leak.
 *
 * \param root_obj [in] root NebulaJsonObject of JSON.
 *
 * \return #NEBULA_ER_NoERROR if freed successfully
 * \return Error code if return value < 0
 *			- #NEBULA_ER_INVALID_ARG Input null argument
 *
 * \see Nebula_Json_Obj_Create_From_String(), Nebula_Json_Obj_New_Empty_Obj()
 * 
 * \attention This function may crash when input is not a root object.
 *
 */
NEBULA_JSON_API int32_t Nebula_Json_Obj_Release(NebulaJsonObject *root_obj);

/**
 * \brief Get object count of a JSON_TYPE_OBJECT NebulaJsonObject
 *
 * \details Get the size of an object in terms of the number of fields it has.
 *
 * \param json_obj [in] A NebulaJsonObject which type is JSON_TYPE_OBJECT.
 *
 * \return >=0 for object count
 * \return Error code if return value < 0
 *			- #NEBULA_ER_INVALID_ARG Input null argument
 *			- #NEBULA_ER_JSON_OBJ_TYPE_ERROR root_obj is not a JSON_TYPE_OBJECT NebulaJsonObject
 *
 */
NEBULA_JSON_API int32_t Nebula_Json_Obj_Get_length(const NebulaJsonObject *json_obj);

/**
 * \brief Get sub NebulaJsonObject by specific key
 *
 * \details Get sub NebulaJsonObject of a JSON_TYPE_OBJECT NebulaJsonObject with specific key.
 *
 * \param parent_obj [in] A NebulaJsonObject which type is JSON_TYPE_OBJECT.
 * \param key [in] Key name of target pair.
 * \param value_obj [out] Sub NebulaJsonObject of target pair.
 *
 * \return #NEBULA_ER_NoERROR when get sub NebulaJsonObject successfully
 *			- #NEBULA_ER_INVALID_ARG Input null argument
 *			- #NEBULA_ER_JSON_OBJ_TYPE_ERROR parent_obj is not a JSON_TYPE_OBJECT NebulaJsonObject
 *			- #NEBULA_ER_JSON_OBJ_NOT_EXIST Key name is not exist
 * 
 * \attention value_obj is constant data , do not modify or free value_obj.
 * 
 */
NEBULA_JSON_API int32_t Nebula_Json_Obj_Get_Sub_Obj(const NebulaJsonObject *parent_obj, const char *key, const NebulaJsonObject **value_obj);

/**
 * \brief Get the type of the NebulaJsonObject.
 *
 * \details Get NebulaJsonType of input NebulaJsonObject.
 *
 * \param value_obj [in] Any type of NebulaJsonObject.
 *
 * \return >=0 for NebulaJsonType
 *			- #NEBULA_ER_INVALID_ARG Input null argument
 * 
 */
NEBULA_JSON_API int32_t Nebula_Json_Obj_Get_Type(const NebulaJsonObject *value_obj);

/**
 * \brief Get boolean value from NebulaJsonObject
 *
 * \details Get boolean value from JSON_TYPE_BOOL type NebulaJsonObject
 *
 * \param value_obj [in] A NebulaJsonObject which type is JSON_TYPE_BOOL.
 * \param bool_value [out] Boolean value of NebulaJsonObject. 0 for FALSE, 1 for TRUE.
 *
 * \return #NEBULA_ER_NoERROR when get value successfully
 *			- #NEBULA_ER_INVALID_ARG Input null argument
 *			- #NEBULA_ER_JSON_OBJ_TYPE_ERROR value_obj is not a JSON_TYPE_BOOL NebulaJsonObject
 * 
 */
NEBULA_JSON_API int32_t Nebula_Json_Obj_Get_Bool(const NebulaJsonObject *value_obj, int32_t *bool_value);

/**
 * \brief Get double value from NebulaJsonObject
 *
 * \details Get double value from JSON_TYPE_DOUBLE type NebulaJsonObject
 *
 * \param value_obj [in] A NebulaJsonObject which type is JSON_TYPE_DOUBLE.
 * \param double_value [out] Double value of NebulaJsonObject.
 *
 * \return #NEBULA_ER_NoERROR when get value successfully
 *			- #NEBULA_ER_INVALID_ARG Input null argument
 *			- #NEBULA_ER_JSON_OBJ_TYPE_ERROR value_obj is not a JSON_TYPE_DOUBLE NebulaJsonObject
 * 
 */
NEBULA_JSON_API int32_t Nebula_Json_Obj_Get_Double(const NebulaJsonObject *value_obj, double *double_value);

/**
 * \brief Get integer value from NebulaJsonObject
 *
 * \details Get integer value from JSON_TYPE_INT type NebulaJsonObject
 *
 * \param value_obj [in] A NebulaJsonObject which type is JSON_TYPE_INT.
 * \param int_value [out] Integer value of NebulaJsonObject.
 *
 * \return #NEBULA_ER_NoERROR when get value successfully
 *			- #NEBULA_ER_INVALID_ARG Input null argument
 *			- #NEBULA_ER_JSON_OBJ_TYPE_ERROR value_obj is not a JSON_TYPE_INT NebulaJsonObject
 * 
 */
NEBULA_JSON_API int32_t Nebula_Json_Obj_Get_Int(const NebulaJsonObject *value_obj, int32_t *int_value);

/**
 * \brief Get string from NebulaJsonObject
 *
 * \details Get string from JSON_TYPE_STRING type NebulaJsonObject
 *
 * \param value_obj [in] A NebulaJsonObject which type is JSON_TYPE_STRING.
 * \param str_value [out] String of NebulaJsonObject.
 *
 * \return #NEBULA_ER_NoERROR when get string successfully
 *			- #NEBULA_ER_INVALID_ARG Input null argument
 * 
 * \attention str_value is constant data , do not modify or free str_value. 
 *            value_obj can be other type, not only JSON_TYPE_STRING
 * 
 */
NEBULA_JSON_API int32_t Nebula_Json_Obj_Get_String(const NebulaJsonObject *value_obj, const char **str_value);

/**
 * \brief Get array length from NebulaJsonObject
 *
 * \details Get array length from JSON_TYPE_ARRAY type NebulaJsonObject 
 *
 * \param array_obj [in] A NebulaJsonObject which type is JSON_TYPE_ARRAY.
 *
 * \return >=0 for array length
 *			- #NEBULA_ER_INVALID_ARG Input null argument
 *			- #NEBULA_ER_JSON_OBJ_TYPE_ERROR value_obj is not a JSON_TYPE_ARRAY NebulaJsonObject
 * 
 */
NEBULA_JSON_API int32_t Nebula_Json_Obj_Get_Array_length(const NebulaJsonObject *array_obj);

/**
 * \brief Get sub NebulaJsonObject from JSON_TYPE_ARRAY type NebulaJsonObject
 *
 * \details Get sub NebulaJsonObject of specificed index from JSON_TYPE_ARRAY type NebulaJsonObject
 *
 * \param array_obj [in] A NebulaJsonObject which type is JSON_TYPE_ARRAY.
 * \param index [in] NebulaJsonObject index of array.
 * \param value_obj [out] Sub NebulaJsonObject of index.
 *
 * \return #NEBULA_ER_NoERROR when get sub NebulaJsonObject successfully
 *			- #NEBULA_ER_INVALID_ARG Input null argument
 *			- #NEBULA_ER_JSON_OBJ_TYPE_ERROR array_obj is not a JSON_TYPE_ARRAY NebulaJsonObject
 *			- #NEBULA_ER_JSON_OBJ_NOT_EXIST Key index is not exist
 * 
 * \attention value_obj is constant data , do not modify or free value_obj. 
 * 
 */
NEBULA_JSON_API int32_t Nebula_Json_Obj_Get_Array_Element(const NebulaJsonObject *array_obj, int32_t index, const NebulaJsonObject **value_obj);

/**
 * \brief Stringify NebulaJsonObject to string
 *
 * \details Get string from any type NebulaJsonObject
 *
 * \param json_obj [in] Any type of NebulaJsonObject.
 *
 * \return String of json_obj
 * 
 * \attention return string is constant data , do not modify or free string 
 * 
 */
NEBULA_JSON_API const char *Nebula_Json_Obj_To_String(const NebulaJsonObject *json_obj);

/**
 * \brief Get specific boolean value from JSON_TYPE_OBJECT type NebulaJsonObject
 *
 * \details Get boolean value by key name of JSON_TYPE_OBJECT type NebulaJsonObject
 *
 * \param obj [in] A NebulaJsonObject which type is JSON_TYPE_OBJECT.
 * \param key [in] Key name of target pair.
 * \param bool_value [out] Boolean value of Key. 0 for FALSE, 1 for TRUE.
 *
 * \return #NEBULA_ER_NoERROR when get value successfully
 *			- #NEBULA_ER_INVALID_ARG Input null argument
 *			- #NEBULA_ER_JSON_OBJ_TYPE_ERROR Value of key type is not boolean
 *
 * \see Nebula_Json_Obj_Get_Sub_Obj(), Nebula_Json_Obj_Get_Bool()
 * 
 */
NEBULA_JSON_API int32_t Nebula_Json_Obj_Get_Sub_Obj_Bool(const NebulaJsonObject *obj, const char *key, int32_t *bool_value);

/**
 * \brief Get specific double value from JSON_TYPE_OBJECT type NebulaJsonObject
 *
 * \details Get double value by key name of JSON_TYPE_OBJECT type NebulaJsonObject
 *
 * \param obj [in] A NebulaJsonObject which type is JSON_TYPE_OBJECT.
 * \param key [in] Key name of target pair.
 * \param double_value [out] Double value of key.
 *
 * \return #NEBULA_ER_NoERROR when get value successfully
 *			- #NEBULA_ER_INVALID_ARG Input null argument
 *			- #NEBULA_ER_JSON_OBJ_TYPE_ERROR Value of key type is not double
 *
 * \see Nebula_Json_Obj_Get_Sub_Obj(), Nebula_Json_Obj_Get_Double()
 * 
 */
NEBULA_JSON_API int32_t Nebula_Json_Obj_Get_Sub_Obj_Double(const NebulaJsonObject *obj, const char *key, double *double_value);

/**
 * \brief Get specific integer value from JSON_TYPE_OBJECT type NebulaJsonObject
 *
 * \details Get integer value by key name of JSON_TYPE_OBJECT type NebulaJsonObject
 *
 * \param obj [in] A NebulaJsonObject which type is JSON_TYPE_OBJECT.
 * \param key [in] Key name of target pair.
 * \param int_value [out] Integer value of key.
 *
 * \return #NEBULA_ER_NoERROR when get value successfully
 *			- #NEBULA_ER_INVALID_ARG Input null argument
 *			- #NEBULA_ER_JSON_OBJ_TYPE_ERROR Value of key type is not integer
 *
 * \see Nebula_Json_Obj_Get_Sub_Obj(), Nebula_Json_Obj_Get_Int()
 * 
 */
NEBULA_JSON_API int32_t Nebula_Json_Obj_Get_Sub_Obj_Int(const NebulaJsonObject *obj, const char *key, int32_t *int_value);

/**
 * \brief Get specific string from JSON_TYPE_OBJECT type NebulaJsonObject
 *
 * \details  Get string by key name of JSON_TYPE_OBJECT type NebulaJsonObject
 *
 * \param obj [in] A NebulaJsonObject which type is JSON_TYPE_OBJECT.
 * \param key [in] Key name of target pair.
 * \param str_value [out] String of key.
 *
 * \return #NEBULA_ER_NoERROR when get string successfully
 *			- #NEBULA_ER_INVALID_ARG Input null argument
 *			- #NEBULA_ER_JSON_OBJ_TYPE_ERROR Value of key type is not string
 * 
 * \see Nebula_Json_Obj_Get_Sub_Obj(), Nebula_Json_Obj_Get_String()
 * 
 * \attention str_value is constant data , do not modify or free str_value 
 * 
 */
NEBULA_JSON_API int32_t Nebula_Json_Obj_Get_Sub_Obj_String(const NebulaJsonObject *obj, const char *key, const char **str_value);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _NEBULA_JSONAPIs_H_ */
