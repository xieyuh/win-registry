/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for
 *license information.
 *--------------------------------------------------------------------------------------------*/

#include <string>
#include <windows.h>

#include <node_api.h>

namespace {

HKEY GetHive(std::string hkey) {
  if (hkey == "HKEY_CURRENT_USER") {
    return HKEY_CURRENT_USER;
  }

  if (hkey == "HKEY_LOCAL_MACHINE") {
    return HKEY_LOCAL_MACHINE;
  }

  if (hkey == "HKEY_CLASSES_ROOT") {
    return HKEY_CLASSES_ROOT;
  }

  if (hkey == "HKEY_USERS") {
    return HKEY_USERS;
  }

  if (hkey == "HKEY_CURRENT_CONFIG") {
    return HKEY_CURRENT_CONFIG;
  }

  return NULL;
}

napi_value GetStringRegKey(napi_env env, napi_callback_info info) {
  napi_value argv[3];
  size_t argc = 3;

  napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

  // Check that we have 3 arguments - Hive, Path, Name
  if (argc < 3) {
    napi_throw_error(env, "EINVAL", "Wrong number of arguments");
    return nullptr;
  }

  // Retrieve the 3 arguments
  for (int i = 0; i < 3; i++) {
    napi_valuetype value_type;
    napi_typeof(env, argv[i], &value_type);
    if (value_type != napi_string) {
      napi_throw_error(env, "EINVAL", "Expected string");
      return nullptr;
    }
  }

  size_t str_len = 0;

  napi_get_value_string_utf8(env, argv[0], nullptr, 0, &str_len);
  char *arg1 = reinterpret_cast<char *>(malloc(str_len + 1));
  napi_get_value_string_utf8(env, argv[0], arg1, str_len + 1, nullptr);

  napi_get_value_string_utf8(env, argv[1], nullptr, 0, &str_len);
  char *arg2 = reinterpret_cast<char *>(malloc(str_len + 1));
  napi_get_value_string_utf8(env, argv[1], arg2, str_len + 1, nullptr);

  napi_get_value_string_utf8(env, argv[2], nullptr, 0, &str_len);
  char *arg3 = reinterpret_cast<char *>(malloc(str_len + 1));
  napi_get_value_string_utf8(env, argv[2], arg3, str_len + 1, nullptr);

  HKEY hive = GetHive(std::string(arg1));
  auto path = std::string(arg2);
  auto name = std::string(arg3);
  free(arg1);
  free(arg2);
  free(arg3);
  std::string result;

  HKEY hKey;
  if (ERROR_SUCCESS != RegOpenKeyEx(hive, path.c_str(), 0, KEY_READ, &hKey)) {
    //napi_throw_error(env, nullptr, "Unable to open registry key");
    return nullptr;
  }

  char szBuffer[512];
  DWORD dwBufferSize = sizeof(szBuffer);

  if (ERROR_SUCCESS == RegQueryValueEx(hKey, name.c_str(), 0, NULL,
                                       (LPBYTE)szBuffer, &dwBufferSize)) {
    result = szBuffer;
  }

  RegCloseKey(hKey);


  napi_value napi_result;
  napi_create_string_utf8(env, result.c_str(), result.length(), &napi_result);

  return napi_result;
}

napi_value SetStringRegKey(napi_env env, napi_callback_info info) {
	const int arg_size = 4;
	napi_value argv[arg_size];
	size_t argc = arg_size;

	napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

	// Check that we have 4 arguments - Hive, Path, Name
	if (argc < arg_size) {
		napi_throw_error(env, "EINVAL", "Wrong number of arguments");
		return nullptr;
	}

	// Retrieve the 4 arguments
	for (int i = 0; i < arg_size; i++) {
		napi_valuetype value_type;
		napi_typeof(env, argv[i], &value_type);
		if (value_type != napi_string) {
			napi_throw_error(env, "EINVAL", "Expected string");
			return nullptr;
		}
	}

	size_t str_len = 0;

	napi_get_value_string_utf8(env, argv[0], nullptr, 0, &str_len);
	char *arg1 = reinterpret_cast<char *>(malloc(str_len + 1));
	napi_get_value_string_utf8(env, argv[0], arg1, str_len + 1, nullptr);

	napi_get_value_string_utf8(env, argv[1], nullptr, 0, &str_len);
	char *arg2 = reinterpret_cast<char *>(malloc(str_len + 1));
	napi_get_value_string_utf8(env, argv[1], arg2, str_len + 1, nullptr);

	napi_get_value_string_utf8(env, argv[2], nullptr, 0, &str_len);
	char *arg3 = reinterpret_cast<char *>(malloc(str_len + 1));
	napi_get_value_string_utf8(env, argv[2], arg3, str_len + 1, nullptr);

	napi_get_value_string_utf8(env, argv[3], nullptr, 0, &str_len);
	char *arg4 = reinterpret_cast<char *>(malloc(str_len + 1));
	napi_get_value_string_utf8(env, argv[3], arg4, str_len + 1, nullptr);

  DWORD nLength = 2048;
	HKEY hKey = GetHive(std::string(arg1));
	auto path = std::string(arg2);
	auto name = std::string(arg3);
	auto value = std::string(arg4);
	free(arg1);
	free(arg2);
	free(arg3);
	free(arg4);
	napi_value napi_result;
	std::string result;
	char ret_str[100] = { '\0' };

	HKEY key;
	int ret = RegOpenKeyExA(hKey, path.c_str(), 0, KEY_ALL_ACCESS | KEY_WOW64_64KEY, &key);
	if (ret != ERROR_SUCCESS)
	{
		if (ret == ERROR_FILE_NOT_FOUND)
		{
			DWORD dwOptions = REG_OPTION_NON_VOLATILE;
			DWORD dwDisposition;
			ret = RegCreateKeyExA(hKey, path.c_str(), 0, NULL,
				dwOptions, KEY_ALL_ACCESS | KEY_WOW64_64KEY, NULL, &key, &dwDisposition);
			if (ret != ERROR_SUCCESS)
			{
				sprintf_s(ret_str, "%d", ret);
				result = std::string(ret_str);
				napi_create_string_utf8(env, result.c_str(), result.length(), &napi_result);
				return napi_result;
			}
		}
		else
		{
			sprintf_s(ret_str, "%d", ret);
			result = std::string(ret_str);
			napi_create_string_utf8(env, result.c_str(), result.length(), &napi_result);
			return napi_result;
		}
	}

	CHAR strValue[2048] = {};
	::RegGetValue(key, nullptr, name.c_str(), REG_SZ, 0, strValue, &nLength);
	ret = RegSetValueEx(key, name.c_str(), 0, REG_SZ, (LPBYTE)value.c_str(), (value.size() + 1)*sizeof(WCHAR));
	::RegCloseKey(key);

	//napi_value napi_result;
	//napi_create_string_utf8(env, result.c_str(), result.length(), &napi_result);

	sprintf_s(ret_str, "%d", ret);
	result = std::string(ret_str);
	napi_create_string_utf8(env, result.c_str(), result.length(), &napi_result);
	return napi_result;
}

napi_value Init(napi_env env, napi_value exports) {
  napi_value getStringRegKey;
	napi_value setStringRegKey;

	napi_create_function(env, "SetStringRegKey", NAPI_AUTO_LENGTH,
											 SetStringRegKey, NULL, &setStringRegKey);
  napi_create_function(env, "GetStringRegKey", NAPI_AUTO_LENGTH,
                       GetStringRegKey, NULL, &getStringRegKey);
  napi_set_named_property(env, exports, "GetStringRegKey", getStringRegKey);
  napi_set_named_property(env, exports, "SetStringRegKey", setStringRegKey);

  return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, Init);
} // namespace